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

#pragma once

// https://v8.github.io/api/head/

#include <functional>
#include <iostream>
#include <string>
#include <type_traits>

#include <libplatform/libplatform.h>

#define V8_COMPRESS_POINTERS 1
#define V8_ENABLE_SANDBOX 1
#define V8_31BIT_SMIS_ON_64BIT_ARCH 1

#include <v8.h>

// V8 change log
// https://docs.google.com/document/d/1g8JFi8T_oAE_7uAri7Njtig7fKaPDfotU6huOa1alds/

namespace ezv8
{
    static std::string v8_version()
    {
        return std::to_string(V8_MAJOR_VERSION) + "." + std::to_string(V8_MINOR_VERSION) + "." + std::to_string(V8_BUILD_NUMBER) + "." + std::to_string(V8_PATCH_LEVEL);
    }

    static v8::Isolate::CreateParams make_params()
    {
        v8::Isolate::CreateParams create_params;
        create_params.array_buffer_allocator = v8::ArrayBuffer::Allocator::NewDefaultAllocator();
        return create_params;
    }

    class V8Platform
    {
    private:
        inline static std::unique_ptr<v8::Platform> platform;

    public:
        V8Platform()
        {
            if (!!platform)
            {
                return;
            }
            platform = v8::platform::NewDefaultPlatform();
            v8::V8::InitializePlatform(platform.get());
            v8::V8::Initialize();
        }

        ~V8Platform()
        {
            if (!platform)
            {
                return;
            }
            // fprintf(stderr, "v8 shutdown platform\n");
            v8::V8::Dispose();
            // v8::V8::ShutdownPlatform();
            platform.release(); // Just let it leak
        }
    };

    ///////////////////////////////////////////////////////////////////////////////
    // Can improve this in c++20
    // https://www.fluentcpp.com/2021/03/05/stdindex_sequence-and-its-improvement-in-c20/
    template <std::size_t... Is>
    auto indexer(std::index_sequence<Is...>)
    {
        return [](auto &&f)
        {
            return f(std::integral_constant<std::size_t, Is>{}...);
        };
    }
    template <std::size_t N>
    auto indexer_upto(std::integral_constant<std::size_t, N> = {})
    {
        return indexer(std::make_index_sequence<N>{});
    }
    template <class T>
    struct tag_t
    {
        using type = T;
    };
    template <class T>
    constexpr tag_t<T> tag{};
    inline unsigned long to_type(v8::Local<v8::Context> &ctx, tag_t<unsigned long>, v8::Handle<v8::Value> const &val) { return val->Uint32Value(ctx).FromJust(); }
    inline unsigned char to_type(v8::Local<v8::Context> &ctx, tag_t<unsigned char>, v8::Handle<v8::Value> const &val) { return val->Int32Value(ctx).FromJust(); }
    inline char to_type(v8::Local<v8::Context> &ctx, tag_t<char>, v8::Handle<v8::Value> const &val) { return val->Int32Value(ctx).FromJust(); }
    inline int to_type(v8::Local<v8::Context> &ctx, tag_t<int>, v8::Handle<v8::Value> const &val) { return val->Int32Value(ctx).FromJust(); }
    inline bool to_type(v8::Local<v8::Context> &ctx, tag_t<bool>, v8::Handle<v8::Value> const &val) { return val->BooleanValue(ctx->GetIsolate()); }
    inline double to_type(v8::Local<v8::Context> &ctx, tag_t<double>, v8::Handle<v8::Value> const &val) { return val->NumberValue(ctx).FromJust(); }
    inline std::string to_type(v8::Local<v8::Context> &ctx, tag_t<std::string>, v8::Handle<v8::Value> const &val) { return *v8::String::Utf8Value(ctx->GetIsolate(), val); }

    inline unsigned long from_type(v8::Isolate *isolate, unsigned long val) { return val; }
    inline char from_type(v8::Isolate *isolate, char val) { return val; }
    inline int from_type(v8::Isolate *isolate, int val) { return val; }
    inline bool from_type(v8::Isolate *isolate, bool val) { return val; }
    inline double from_type(v8::Isolate *isolate, double val) { return val; }
    inline v8::Local<v8::String> from_type(v8::Isolate *isolate, std::string str) { return v8::String::NewFromUtf8(isolate, str.c_str()).ToLocalChecked(); }

    // inline v8::Local<v8::Uint8ClampedArray> from_type(v8::Isolate *isolate, std::vector<uint8_t> *val)
    // {
    //     auto buffer = v8::ArrayBuffer::New(isolate, val->data(), val->size(), v8::ArrayBufferCreationMode::kExternalized);
    //     return v8::Uint8ClampedArray::New(buffer, 0, buffer->GetContents().ByteLength());
    // }

    ///////////////////////////////////////////////////////////////////////////////////////////////////////
    template <typename R, typename... Args>
    v8::Local<v8::FunctionTemplate> NewFunction(v8::Isolate *isolate, R (*func)(Args...))
    {
        return v8::FunctionTemplate::New(
            isolate,
            [](const v8::FunctionCallbackInfo<v8::Value> &args)
            {
            if (sizeof...(Args) > args.Length()) {
                return; // throw std::invalid_argument("arg count mismatch");
            }

            auto iso = args.GetIsolate();
            auto cxt = iso->GetCurrentContext();

            return indexer_upto<sizeof...(Args)>()([&](auto... Is) {
                auto func = reinterpret_cast<R (*)(Args...)>(v8::External::Cast(*args.Data())->Value());
                if constexpr (std::is_same_v<R, void>) {
                    func(to_type(cxt, tag<Args>, args[Is])...);
                } else {
                    auto ret = func(to_type(cxt, tag<Args>, args[Is])...);
                    args.GetReturnValue().Set(from_type(iso, ret));
                }
            }); },

            v8::External::New(isolate, reinterpret_cast<void *>(func)));
    }
    ///////////////////////////////////////////////////////////////////////////////////////////////////////
    template <typename T, typename R, typename... Args>
    void NewObjectMethod(v8::Isolate *isolate, v8::Local<v8::FunctionTemplate> &tmpl, const std::string &name, R (T::*func)(Args...))
    {
        using CbFunc = std::function<void(T * _this, const v8::FunctionCallbackInfo<v8::Value> &args)>;
        // TODO cb gets leaked!
        auto cb = new CbFunc([func](T *_this, const v8::FunctionCallbackInfo<v8::Value> &args)
                             {
        if (sizeof...(Args) > args.Length()) {
            return; // throw std::invalid_argument("arg count mismatch"); // or whatever
        }
        auto iso = args.GetIsolate();
        auto cxt = iso->GetCurrentContext();
        return indexer_upto<sizeof...(Args)>()([&](auto... Is) {
            if constexpr (std::is_same_v<R, void>) {
                (_this->*func)(to_type(cxt, tag<Args>, args[Is])...);
            } else {
                auto ret = (_this->*func)(to_type(cxt, tag<Args>, args[Is])...);
                args.GetReturnValue().Set(from_type(iso, ret));
            }
        }); });

        tmpl->InstanceTemplate()->Set(
            v8::String::NewFromUtf8(isolate, name.c_str()).ToLocalChecked(),
            v8::FunctionTemplate::New(
                isolate, [](const v8::FunctionCallbackInfo<v8::Value> &args)
                {
                auto func = reinterpret_cast<CbFunc*>(v8::External::Cast(*args.Data())->Value());
                auto _this = reinterpret_cast<T*>(v8::Local<v8::External>::Cast(args.Holder()->GetInternalField(0))->Value());
                (*func)(_this, args); },
                v8::External::New(isolate, reinterpret_cast<void *>(cb))));
    }

    template <typename T, typename Arg>
    void NewAccessor(v8::Isolate *isolate, v8::Local<v8::FunctionTemplate> &tmpl, const std::string &name, Arg (T::*get)(), void (T::*set)(Arg))
    {
        using CbPair = std::pair<std::function<void(T *, const v8::PropertyCallbackInfo<v8::Value> &)>,
                                 std::function<void(T *, v8::Local<v8::Value>, const v8::PropertyCallbackInfo<void> &)>>;
        // TODO this is a mempry leak
        auto cbpair = new CbPair(
            [get](T *_this, const v8::PropertyCallbackInfo<v8::Value> &info)
            {
                auto iso = info.GetIsolate();
                auto ret = (_this->*get)();
                info.GetReturnValue().Set(from_type(iso, ret));
            },
            [set](T *_this, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<void> &info)
            {
                auto cxt = info.GetIsolate()->GetCurrentContext();
                (_this->*set)(to_type(cxt, tag_t<Arg>(), value));
            });

        tmpl->InstanceTemplate()->SetAccessor(
            v8::String::NewFromUtf8(isolate, name.c_str()).ToLocalChecked(),

            [](v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value> &info)
            {
                auto _this = reinterpret_cast<T *>(v8::Local<v8::External>::Cast(info.Holder()->GetInternalField(0))->Value());
                auto axor = reinterpret_cast<CbPair *>(v8::External::Cast(*info.Data())->Value());
                axor->first(_this, info);
            },

            [](v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<void> &info)
            {
                auto _this = reinterpret_cast<T *>(v8::Local<v8::External>::Cast(info.Holder()->GetInternalField(0))->Value());
                auto axor = reinterpret_cast<CbPair *>(v8::External::Cast(*info.Data())->Value());
                axor->second(_this, value, info);
            },

            v8::External::New(isolate, reinterpret_cast<void *>(cbpair)));
    }

    template <typename T, typename Arg>
    void NewAccessor(v8::Isolate *isolate, v8::Local<v8::FunctionTemplate> &tmpl, const std::string &name, Arg (T::*get)())
    {
        using CbTmpl = std::function<void(T *, const v8::PropertyCallbackInfo<v8::Value> &)>;

        auto cb = new CbTmpl([get](T *_this, const v8::PropertyCallbackInfo<v8::Value> &info)
                             {
        auto iso = info.GetIsolate();
        auto ret = (_this->*get)();
        info.GetReturnValue().Set(from_type(iso, ret)); });

        tmpl->InstanceTemplate()->SetAccessor(
            v8::String::NewFromUtf8(isolate, name.c_str()).ToLocalChecked(),
            [](v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value> &info)
            {
                auto self = info.Holder();
                auto ifield0 = self->GetInternalField(0);
                auto ifield1 = ifield0.As<v8::External>();
                auto ifield2 = ifield1->Value();
                auto _this = reinterpret_cast<T *>(ifield2);
                // auto _this = reinterpret_cast<T*>(v8::Local<v8::External>::Cast(ifield)->Value());
                auto axor = reinterpret_cast<CbTmpl *>(v8::External::Cast(*info.Data())->Value());
                (*axor)(_this, info);
            },
            0, v8::External::New(isolate, reinterpret_cast<void *>(cb)));
    }
} // namespace
