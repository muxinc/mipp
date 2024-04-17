// Copyright 2024 Mux, Inc.

// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at

//     http://www.apache.org/licenses/LICENSE-2.0

// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "mipp.h"
#include "cairo.hpp"

#include <iostream>

static std::string load_script(std::string filename)
{
    std::ifstream t(filename);
    std::string str((std::istreambuf_iterator<char>(t)),
                    std::istreambuf_iterator<char>());
    return str;
}

ezv8::V8Platform platform = ezv8::V8Platform();

class Mipp
{
private:
    std::unique_ptr<v8::Isolate, void (*)(v8::Isolate *)> isolate;
    v8::HandleScope handle_scope;
    v8::Local<v8::ObjectTemplate> global_templ;

    v8::Persistent<v8::Context> persistent_context;

    v8::Local<v8::Function> receive_video_frame_func;
    std::function<void(int width, int height, double pts, uint8_t *data)> receive_video_frame_callback;
    std::function<void(int level, std::string msg)> log_callback;

    v8::Local<v8::Function> VideoFrameCtor;

    int videoInPads = 1;

public:
    int inputPads() const { return videoInPads; }
    int send_video_frame(int width, int height, int stride, double pts, uint8_t *data, int in_pad_index)
    {
        auto scope = v8::HandleScope(isolate.get());
        auto context = v8::Local<v8::Context>::New(isolate.get(), persistent_context);
        auto context_scope = v8::Context::Scope(context);

        if (receive_video_frame_func.IsEmpty())
        {
            return -1;
        }

        // Create AVFrame in JS and copy pixel data to it
        v8::Handle<v8::Value> frameArgs[] = {
            v8::Number::New(isolate.get(), width),
            v8::Number::New(isolate.get(), height),
            v8::Number::New(isolate.get(), pts)};
        auto f = VideoFrameCtor->NewInstance(context, 3, frameArgs).ToLocalChecked();
        auto obj = v8::Local<v8::Object>::Cast(f);
        auto buffer = obj->Get(context, v8::String::NewFromUtf8(isolate.get(), "data").ToLocalChecked()).ToLocalChecked().As<v8::Uint32Array>();

        for (int y = 0; y < height; y++)
        {
            auto src = reinterpret_cast<uint8_t *>(data) + y * stride;
            auto dest = reinterpret_cast<uint8_t *>(buffer->Buffer()->Data()) + y * width * 4;
            std::memcpy(dest, src, width * 4);
        }

        v8::Handle<v8::Value> args[] = {f, v8::Number::New(isolate.get(), in_pad_index)};
        auto result = receive_video_frame_func->Call(context, context->Global(), 2, args);
        return 0; // TODO return value
    };

    Mipp(const std::string &script_path,
         std::function<void(int width, int height, double pts, uint8_t *data)> receive_video_frame_callback,
         std::function<void(int level, std::string msg)> log_callback)
        : isolate(std::unique_ptr<v8::Isolate, void (*)(v8::Isolate *)>(
              v8::Isolate::New(ezv8::make_params()),
              [](v8::Isolate *i)
              { i->Dispose(); })),
          handle_scope(isolate.get()), global_templ(v8::ObjectTemplate::New(isolate.get())) // I dont think we need to save this in the object?
          ,
          receive_video_frame_callback(receive_video_frame_callback), log_callback(log_callback)
    {
        isolate->Enter();                       // manually enter and exit the isolate
        global_templ->SetInternalFieldCount(1); // Used to track `this` for callbacks

        auto receive_video_frame = v8::FunctionTemplate::New(isolate.get(), [](const v8::FunctionCallbackInfo<v8::Value> &args)
                                                             {
                                                                 auto iso = args.GetIsolate();
                                                                 auto ctx = iso->GetCurrentContext();
                                                                 auto scope = v8::HandleScope(iso);
                                                                 auto obj = v8::Local<v8::Object>::Cast(args[0]);

                                                                 auto width = obj->Get(ctx, v8::String::NewFromUtf8(args.GetIsolate(), "_width").ToLocalChecked()).ToLocalChecked()->NumberValue(ctx).FromJust();
                                                                 auto height = obj->Get(ctx, v8::String::NewFromUtf8(args.GetIsolate(), "_height").ToLocalChecked()).ToLocalChecked()->NumberValue(ctx).FromJust();
                                                                 auto pts = obj->Get(ctx, v8::String::NewFromUtf8(args.GetIsolate(), "pts").ToLocalChecked()).ToLocalChecked()->NumberValue(ctx).FromJust();
                                                                 auto data = obj->Get(ctx, v8::String::NewFromUtf8(args.GetIsolate(), "data").ToLocalChecked()).ToLocalChecked().As<v8::Uint32Array>();
                                                                 auto buffer = data->Buffer();

                                                                 // TODO validate values
                                                                 auto expectedLength = static_cast<int>(width * height * 4);
                                                                 if (expectedLength == 0 || buffer->ByteLength() != expectedLength)
                                                                 {
                                                                     std::cerr << "send_frame: invalid frame size " << expectedLength << " != " << buffer->ByteLength() << std::endl;
                                                                     exit(-1);
                                                                 }

                                                                 auto mipp = reinterpret_cast<Mipp *>(v8::Local<v8::External>::Cast(args.Holder()->GetInternalField(0))->Value());
                                                                 mipp->receive_video_frame_callback(width, height, pts, reinterpret_cast<uint8_t *>(buffer->Data()));
                                                                 // TODO return value
                                                             });

        auto VideoFrameTmpl = v8::FunctionTemplate::New(isolate.get(), [](const v8::FunctionCallbackInfo<v8::Value> &args)
                                                        {
                                                            double pts = 0;
                                                            int width = 0, height = 0;
                                                            auto iso = args.GetIsolate();
                                                            auto ctx = iso->GetCurrentContext();
                                                            if (args.Length() >= 3)
                                                            {
                                                                width = args[0]->NumberValue(ctx).FromJust();
                                                                height = args[1]->NumberValue(ctx).FromJust();
                                                                pts = args[2]->NumberValue(ctx).FromJust();
                                                            }

                                                            auto arrayBuffer = v8::Uint32Array::New(v8::ArrayBuffer::New(iso, width * height * 4), 0, width * height);
                                                            args.This()->Set(ctx, v8::String::NewFromUtf8(iso, "_width").ToLocalChecked(), v8::Number::New(args.GetIsolate(), width)).FromJust();
                                                            args.This()->Set(ctx, v8::String::NewFromUtf8(iso, "_height").ToLocalChecked(), v8::Number::New(args.GetIsolate(), height)).FromJust();
                                                            args.This()->Set(ctx, v8::String::NewFromUtf8(iso, "pts").ToLocalChecked(), v8::Number::New(args.GetIsolate(), pts)).FromJust();
                                                            args.This()->Set(ctx, v8::String::NewFromUtf8(iso, "data").ToLocalChecked(), arrayBuffer).FromJust();
                                                            auto data = reinterpret_cast<uint8_t *>(arrayBuffer->Buffer()->Data());
                                                            auto c = new cairo(width, height, data);
                                                            args.This()->SetInternalField(0, v8::External::New(iso, c));

                                                            if (args.Length() >= 4)
                                                            {
                                                                // TODO more checks
                                                                auto srcFrame = v8::Local<v8::Object>::Cast(args[3]);
                                                                if (srcFrame->IsObject())
                                                                {
                                                                    auto srcWidth = srcFrame->Get(ctx, v8::String::NewFromUtf8(iso, "_width").ToLocalChecked()).ToLocalChecked()->NumberValue(ctx).FromJust();
                                                                    auto srcHeight = srcFrame->Get(ctx, v8::String::NewFromUtf8(iso, "_height").ToLocalChecked()).ToLocalChecked()->NumberValue(ctx).FromJust();
                                                                    auto srcPts = srcFrame->Get(ctx, v8::String::NewFromUtf8(iso, "pts").ToLocalChecked()).ToLocalChecked()->NumberValue(ctx).FromJust();
                                                                    auto srcData = srcFrame->Get(ctx, v8::String::NewFromUtf8(iso, "data").ToLocalChecked()).ToLocalChecked().As<v8::Uint32Array>();

                                                                    // Make a temp canvas sharing the same buffer as the source frame
                                                                    auto tmp = cairo(srcWidth, srcHeight, reinterpret_cast<uint8_t *>(srcData->Buffer()->Data()));
                                                                    c->drawImage(&tmp, 0, 0, srcWidth, srcHeight);
                                                                }
                                                            }
                                                            args.GetReturnValue().Set(args.This());
                                                            // TODO get garbage collection working
                                                        });
        VideoFrameTmpl->InstanceTemplate()->SetInternalFieldCount(1);

        // TODO should methods be applied to the prototype?
        ezv8::NewAccessor(isolate.get(), VideoFrameTmpl, "width", &cairo::width);
        ezv8::NewAccessor(isolate.get(), VideoFrameTmpl, "height", &cairo::height);

        ezv8::NewAccessor(isolate.get(), VideoFrameTmpl, "globalAlpha", &cairo::get_globalAlpha, &cairo::set_globalAlpha);
        ezv8::NewAccessor(isolate.get(), VideoFrameTmpl, "lineWidth", &cairo::get_lineWidth, &cairo::set_lineWidth);
        ezv8::NewAccessor(isolate.get(), VideoFrameTmpl, "fillStyle", &cairo::get_fillStyle, &cairo::set_fillStyle);
        ezv8::NewAccessor(isolate.get(), VideoFrameTmpl, "strokeStyle", &cairo::get_strokeStyle, &cairo::set_strokeStyle);
        ezv8::NewAccessor(isolate.get(), VideoFrameTmpl, "font", &cairo::get_font, &cairo::set_font);
        ezv8::NewObjectMethod(isolate.get(), VideoFrameTmpl, "rotate", &cairo::rotate);
        ezv8::NewObjectMethod(isolate.get(), VideoFrameTmpl, "translate", &cairo::translate);
        ezv8::NewObjectMethod(isolate.get(), VideoFrameTmpl, "save", &cairo::save);
        ezv8::NewObjectMethod(isolate.get(), VideoFrameTmpl, "restore", &cairo::restore);
        ezv8::NewObjectMethod(isolate.get(), VideoFrameTmpl, "arc", &cairo::arc);
        ezv8::NewObjectMethod(isolate.get(), VideoFrameTmpl, "beginPath", &cairo::beginPath);
        ezv8::NewObjectMethod(isolate.get(), VideoFrameTmpl, "moveTo", &cairo::moveTo);
        ezv8::NewObjectMethod(isolate.get(), VideoFrameTmpl, "lineTo", &cairo::lineTo);
        ezv8::NewObjectMethod(isolate.get(), VideoFrameTmpl, "closePath", &cairo::closePath);
        ezv8::NewObjectMethod(isolate.get(), VideoFrameTmpl, "fill", &cairo::fill);
        ezv8::NewObjectMethod(isolate.get(), VideoFrameTmpl, "stroke", &cairo::stroke);
        ezv8::NewObjectMethod(isolate.get(), VideoFrameTmpl, "fillRect", &cairo::fillRect);
        ezv8::NewObjectMethod(isolate.get(), VideoFrameTmpl, "rect", &cairo::rect);
        ezv8::NewObjectMethod(isolate.get(), VideoFrameTmpl, "strokeRect", &cairo::strokeRect);
        ezv8::NewObjectMethod(isolate.get(), VideoFrameTmpl, "bezierCurveTo", &cairo::bezierCurveTo);
        ezv8::NewObjectMethod(isolate.get(), VideoFrameTmpl, "fillText", &cairo::fillText);
        ezv8::NewObjectMethod(isolate.get(), VideoFrameTmpl, "strokeText", &cairo::strokeText);
        ezv8::NewObjectMethod(isolate.get(), VideoFrameTmpl, "scale", &cairo::scale);

        // hack
        // ezv8::NewObjectMethod(isolate.get(), VideoFrameTmpl, "setStrokeStyle", &cairo::strokeStyle);

        VideoFrameTmpl->InstanceTemplate()->Set(
            v8::String::NewFromUtf8(isolate.get(), "draw").ToLocalChecked(),
            v8::FunctionTemplate::New(
                isolate.get(), [](const v8::FunctionCallbackInfo<v8::Value> &args)
                {
                    if (args.Length() != 5) {
                        return;
                    }

                    int x = 0, y = 0, w = 0, h = 0;
                    x = args[1]->NumberValue(args.GetIsolate()->GetCurrentContext()).FromJust();
                    y = args[2]->NumberValue(args.GetIsolate()->GetCurrentContext()).FromJust();
                    w = args[3]->NumberValue(args.GetIsolate()->GetCurrentContext()).FromJust();
                    h = args[4]->NumberValue(args.GetIsolate()->GetCurrentContext()).FromJust();
                    // TODO security issues? I think yes
                    auto canvas = reinterpret_cast<cairo*>(v8::Local<v8::External>::Cast(args.Holder()->GetInternalField(0))->Value());
                    auto source = reinterpret_cast<cairo*>(v8::Local<v8::External>::Cast(args[0]->ToObject(args.GetIsolate()->GetCurrentContext()).ToLocalChecked()->GetInternalField(0))->Value());
                    canvas->drawImage(source, x, y, w, h); }));

        global_templ->Set(v8::String::NewFromUtf8(isolate.get(), "VideoFrame").ToLocalChecked(), VideoFrameTmpl);

        global_templ->Set(v8::String::NewFromUtf8(isolate.get(), "make_pads").ToLocalChecked(),
                          v8::FunctionTemplate::New(isolate.get(), [](const v8::FunctionCallbackInfo<v8::Value> &args)
                                                    {
                auto iso = args.GetIsolate();
                auto ctx = iso->GetCurrentContext();
                auto mipp = reinterpret_cast<Mipp*>(v8::Local<v8::External>::Cast(args.Holder()->GetInternalField(0))->Value());
                if (args.Length() >= 1) {
                    mipp->videoInPads = args[0]->NumberValue(ctx).FromJust();
                } }));

        global_templ->Set(v8::String::NewFromUtf8(isolate.get(), "log").ToLocalChecked(),
                          v8::FunctionTemplate::New(isolate.get(), [](const v8::FunctionCallbackInfo<v8::Value> &args)
                                                    {
                if (args.Length() == 0) {
                    return;
                }

                std::string txt;
                int level = 48;
                for (int i = 0; i < args.Length(); i++) {
                    if (i == 0 && args[0]->IsNumber()) {
                        level = args[0]->NumberValue(args.GetIsolate()->GetCurrentContext()).FromJust();
                        continue;
                    }
                    txt += *v8::String::Utf8Value(args.GetIsolate(), args[i]);
                }
                auto mipp = reinterpret_cast<Mipp*>(v8::Local<v8::External>::Cast(args.Holder()->GetInternalField(0))->Value());
                mipp->log_callback(level, txt); }));

        global_templ->Set(v8::String::NewFromUtf8(isolate.get(), "send_video_frame").ToLocalChecked(), receive_video_frame);

        auto context = v8::Context::New(isolate.get(), nullptr, global_templ);
        context->Global()->SetInternalField(0, v8::External::New(isolate.get(), this));

        persistent_context.Reset(isolate.get(), context);
        auto context_scope = v8::Context::Scope(context);

        std::string js = load_script(script_path);
        v8::Local<v8::String> source = v8::String::NewFromUtf8(isolate.get(), js.c_str(), v8::NewStringType::kNormal).ToLocalChecked();

        auto script = v8::Script::Compile(context, source).ToLocalChecked(); // Compile the source code.
        v8::Local<v8::Value> result = script->Run(context).ToLocalChecked(); // Run the script to get the result.

        auto func = context->Global()->Get(context, v8::String::NewFromUtf8(isolate.get(), "VideoFrame").ToLocalChecked()).ToLocalChecked();
        if (func->IsFunction())
        {
            VideoFrameCtor = v8::Handle<v8::Function>::Cast(func);
        }

        func = context->Global()->Get(context, v8::String::NewFromUtf8(isolate.get(), "receive_video_frame").ToLocalChecked()).ToLocalChecked();
        if (func->IsFunction())
        {
            receive_video_frame_func = v8::Handle<v8::Function>::Cast(func);
        }
    }

    ~Mipp()
    {
        isolate->Exit();
        persistent_context.Reset();
    }
};

extern "C"
{
    int mipp_init(mipp_t *mipp, char *script_path, void *opaque,
                  int (*receive_video_frame)(void *, int width, int height, double pts, uint8_t *data),
                  void (*log)(int level, const char *msg))
    {
        auto priv = new Mipp(
            script_path,
            [opaque, receive_video_frame](int width, int height, double pts, uint8_t *data)
            {
                receive_video_frame(opaque, width, height, pts, data);
            },
            [log](int level, std::string msg)
            {
                if (log)
                {
                    log(level, msg.c_str());
                    return;
                }
                std::cerr << msg << std::endl;
            });

        mipp->video_in_count = std::max(1, priv->inputPads());
        mipp->priv = reinterpret_cast<void *>(priv);
        return 0;
    }

    void mipp_free(mipp_t *mipp)
    {
        if (0 != mipp->priv)
        {
            delete reinterpret_cast<Mipp *>(mipp->priv);
        }
    }

    int mipp_send_video_frame(mipp_t *mipp, int width, int height, int stride, double pts, uint8_t *data, int in_pad_index)
    {
        return reinterpret_cast<Mipp *>(mipp->priv)->send_video_frame(width, height, stride, pts, data, in_pad_index);
    }
};
