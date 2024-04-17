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

#include "colors.hpp"
#include "ezv8.hpp"

#include <cairo/cairo.h>

// https://cairographics.org/manual/
// https://developer.mozilla.org/en-US/docs/Web/API/CanvasRenderingContext2D
class cairo
{
private:
    std::unique_ptr<cairo_surface_t, void (*)(cairo_surface_t *)> m_surface;
    std::unique_ptr<cairo_t, void (*)(cairo_t *)> m_cairo;

    std::unique_ptr<cairo_pattern_t, void (*)(cairo_pattern_t *)> m_fillPattern;
    std::unique_ptr<cairo_pattern_t, void (*)(cairo_pattern_t *)> m_strokePattern;

    inline double r(uint32_t c) { return (c >> 16 & 0xff) / 255.0; }
    inline double g(uint32_t c) { return (c >> 8 & 0xff) / 255.0; }
    inline double b(uint32_t c) { return (c >> 0 & 0xff) / 255.0; }
    inline double a(uint32_t c) { return (c >> 24 & 0xff) / 255.0; }

public:
    cairo(int width, int height, uint8_t *data)
        : m_surface(cairo_image_surface_create_for_data(data, CAIRO_FORMAT_ARGB32, width, height, width * 4), cairo_surface_destroy), m_cairo(cairo_create(m_surface.get()), cairo_destroy), m_fillPattern(cairo_pattern_create_rgb(1, 1, 1), cairo_pattern_destroy), m_strokePattern(cairo_pattern_create_rgb(1, 1, 1), cairo_pattern_destroy)
    {
        // set_strokeStyle("black");
        // set_fillStyle("black");
        set_lineWidth(10.0);
    }

    ~cairo() = default;
    cairo &operator=(cairo &&) = default;
    void save_png(const std::string &name)
    {
        cairo_surface_write_to_png(m_surface.get(), name.c_str());
    }

    uint8_t *data() { return cairo_image_surface_get_data(m_surface.get()); }
    int stride() { return cairo_image_surface_get_stride(m_surface.get()); }
    const uint8_t *data() const { return cairo_image_surface_get_data(m_surface.get()); }
    int stride() const { return cairo_image_surface_get_stride(m_surface.get()); }

    void flush() { cairo_surface_flush(m_surface.get()); }
    void mark_dirty() { cairo_surface_mark_dirty(m_surface.get()); }
    int width() { return cairo_image_surface_get_width(m_surface.get()); }
    int height() { return cairo_image_surface_get_height(m_surface.get()); }
    int width() const { return cairo_image_surface_get_width(m_surface.get()); }
    int height() const { return cairo_image_surface_get_height(m_surface.get()); }
    void rotate(double angle) { cairo_rotate(m_cairo.get(), angle); }
    void translate(double tx, double ty) { cairo_translate(m_cairo.get(), tx, ty); }
    void fill()
    {
        cairo_set_source(m_cairo.get(), m_fillPattern.get());
        cairo_fill(m_cairo.get());
    }

    void set_globalAlpha(double alpha) { cairo_set_source_rgba(m_cairo.get(), 1, 1, 1, alpha); }
    double get_globalAlpha() { return 1; } // TODO

    std::string get_font() { return ""; }
    void set_font(std::string font)
    {
        int size;
        char family[128];
        if (2 == std::sscanf(font.c_str(), "%dpx %127s", &size, family))
        {
            cairo_select_font_face(m_cairo.get(), &family[0], CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
            cairo_set_font_size(m_cairo.get(), size);
        }
    }

    void fillText(std::string text, int x, int y)
    {
        save();
        cairo_move_to(m_cairo.get(), x, y);
        cairo_set_source(m_cairo.get(), m_fillPattern.get());
        cairo_show_text(m_cairo.get(), text.c_str());
        cairo_fill(m_cairo.get());
        restore();
    }

    void strokeText(std::string text)
    {
        save();
        cairo_set_source(m_cairo.get(), m_strokePattern.get());
        cairo_show_text(m_cairo.get(), text.c_str());
        cairo_stroke(m_cairo.get());
        restore();
    }

    void strokeStyle(int r, int g, int b, int a)
    {
        m_strokePattern = std::unique_ptr<cairo_pattern_t, void (*)(cairo_pattern_t *)>(
            cairo_pattern_create_rgba(r / 255.0, g / 255.0, b / 255.0, a / 255.0), cairo_pattern_destroy);
    }

    void strokeStyle(uint32_t rgba)
    {
        strokeStyle((rgba >> 24) & 255, (rgba >> 16) & 255, (rgba >> 8) & 255, (rgba >> 0) & 255);
    }

    void set_strokeStyle(std::string style)
    {
        strokeStyle(color_from_string(style));
    }

    std::string get_strokeStyle()
    {
        char rgb[8];
        double r, g, b, a;
        cairo_pattern_get_rgba(m_strokePattern.get(), &a, &g, &b, &a);
        snprintf(&rgb[0], 8, "#%02d%02d%02d", static_cast<uint8_t>(r * 255), static_cast<uint8_t>(g * 255), static_cast<uint8_t>(b * 255));
        return std::string(&rgb[0]);
    }

    void fillStyle(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
    {
        m_fillPattern = std::unique_ptr<cairo_pattern_t, void (*)(cairo_pattern_t *)>(
            cairo_pattern_create_rgba(r / 255.0, g / 255.0, b / 255.0, a / 255.0), cairo_pattern_destroy);
    }

    void fillStyle(uint32_t rgba)
    {
        fillStyle((rgba >> 24) & 255, (rgba >> 16) & 255, (rgba >> 8) & 255, (rgba >> 0) & 255);
    }

    void set_fillStyle(std::string style)
    {
        fillStyle(color_from_string(style));
    }

    std::string get_fillStyle()
    {
        char rgb[8];
        double r, g, b, a;
        cairo_pattern_get_rgba(m_fillPattern.get(), &a, &g, &b, &a);
        snprintf(&rgb[0], 8, "#%02d%02d%02d", static_cast<uint8_t>(r * 255), static_cast<uint8_t>(g * 255), static_cast<uint8_t>(b * 255));
        return std::string(&rgb[0]);
    }

    void drawImage(cairo *src, int x, int y, double w, double h)
    {
        auto sw = w / src->width();
        auto sh = h / src->height();
        save();
        cairo_scale(m_cairo.get(), sw, sh);
        cairo_set_source_surface(m_cairo.get(), src->m_surface.get(), x / sw, y / sh);
        cairo_paint(m_cairo.get());
        restore();
    }

    // Drawing rectangles
    // https://developer.mozilla.org/en-US/docs/Web/API/CanvasRenderingContext2D/clearRect
    void clearRect(double x, double y, double width, double height)
    {
        cairo_save(m_cairo.get());
        cairo_set_source_rgba(m_cairo.get(), 0, 0, 0, 0);
        cairo_set_operator(m_cairo.get(), CAIRO_OPERATOR_SOURCE);
        cairo_paint(m_cairo.get());
        cairo_restore(m_cairo.get());
    }

    // https://developer.mozilla.org/en-US/docs/Web/API/CanvasRenderingContext2D/fillRect
    void fillRect(double x, double y, double width, double height)
    {
        save();
        cairo_set_source(m_cairo.get(), m_fillPattern.get());
        cairo_rectangle(m_cairo.get(), x, y, width, height);
        cairo_fill(m_cairo.get());
        restore();
    }

    // https://developer.mozilla.org/en-US/docs/Web/API/CanvasRenderingContext2D/strokeRect
    void strokeRect(double x, double y, double width, double height)
    {
        save();
        cairo_set_source(m_cairo.get(), m_strokePattern.get());
        cairo_rectangle(m_cairo.get(), x, y, width, height);
        cairo_stroke(m_cairo.get());
        restore();
    }
    // Drawing text
    // https://developer.mozilla.org/en-US/docs/Web/API/CanvasRenderingContext2D/fillText
    // fillText(text, x, y [, maxWidth]);
    // https://developer.mozilla.org/en-US/docs/Web/API/CanvasRenderingContext2D/strokeText
    // strokeText(text, x, y [, maxWidth]);
    // https://developer.mozilla.org/en-US/docs/Web/API/CanvasRenderingContext2D/measureText
    // measureText(text);

    // Line styles
    // https://developer.mozilla.org/en-US/docs/Web/API/CanvasRenderingContext2D/lineWidth
    double get_lineWidth() { return cairo_get_line_width(m_cairo.get()); }
    void set_lineWidth(double lineWidth) { cairo_set_line_width(m_cairo.get(), lineWidth); }

    // https://developer.mozilla.org/en-US/docs/Web/API/CanvasRenderingContext2D/lineCap
    std::string get_lineCap()
    {
        switch (cairo_get_line_cap(m_cairo.get()))
        {
        default:
        case CAIRO_LINE_CAP_BUTT:
            return "butt";
        case CAIRO_LINE_CAP_ROUND:
            return "round";
        case CAIRO_LINE_CAP_SQUARE:
            return "square";
        };
    }
    void set_lineCap(std::string lineCap) { cairo_set_line_cap(m_cairo.get(), lineCap == "square" ? CAIRO_LINE_CAP_SQUARE : lineCap == "round" ? CAIRO_LINE_CAP_ROUND
                                                                                                                                               : CAIRO_LINE_CAP_BUTT); }

    // https://developer.mozilla.org/en-US/docs/Web/API/CanvasRenderingContext2D/lineJoin
    std::string get_lineJoin()
    {
        switch (cairo_get_line_join(m_cairo.get()))
        {
        default:
        case CAIRO_LINE_JOIN_MITER:
            return "miter";
        case CAIRO_LINE_JOIN_ROUND:
            return "round";
        case CAIRO_LINE_JOIN_BEVEL:
            return "bevel";
        };
    }
    void set_lineJoin(std::string lineJoin) { cairo_set_line_join(m_cairo.get(), lineJoin == "bevel" ? CAIRO_LINE_JOIN_BEVEL : lineJoin == "round" ? CAIRO_LINE_JOIN_ROUND
                                                                                                                                                   : CAIRO_LINE_JOIN_MITER); }

    // https://developer.mozilla.org/en-US/docs/Web/API/CanvasRenderingContext2D/miterLimit
    double get_miterLimit() { return cairo_get_miter_limit(m_cairo.get()); }
    void set_miterLimit(double miterLimit) { cairo_set_miter_limit(m_cairo.get(), miterLimit); }

    // Paths
    // https://developer.mozilla.org/en-US/docs/Web/API/CanvasRenderingContext2D/beginPath
    void beginPath() { cairo_new_path(m_cairo.get()); }

    // https://developer.mozilla.org/en-US/docs/Web/API/CanvasRenderingContext2D/closePath
    void closePath() { cairo_close_path(m_cairo.get()); }

    // https://developer.mozilla.org/en-US/docs/Web/API/CanvasRenderingContext2D/moveTo
    void moveTo(double x, double y) { cairo_move_to(m_cairo.get(), x, y); }

    // https://developer.mozilla.org/en-US/docs/Web/API/CanvasRenderingContext2D/lineTo
    void lineTo(double x, double y) { cairo_line_to(m_cairo.get(), x, y); }

    // https://developer.mozilla.org/en-US/docs/Web/API/CanvasRenderingContext2D/bezierCurveTo
    void bezierCurveTo(double cp1x, double cp1y, double cp2x, double cp2y, double x, double y) { cairo_curve_to(m_cairo.get(), cp1x, cp1y, cp2x, cp2y, x, y); }

    // quadraticCurveTo()
    // https://developer.mozilla.org/en-US/docs/Web/API/CanvasRenderingContext2D/arc
    void arc(double x, double y, double radius, double startAngle, double endAngle) //, bool anticlockwise = false)
    {
        // TODO
        // if (anticlockwise) {

        cairo_arc(m_cairo.get(), x, y, radius, startAngle, endAngle);
    }
    // https://developer.mozilla.org/en-US/docs/Web/API/CanvasRenderingContext2D/arcTo
    // arcTo(x1, y1, x2, y2, radius);
    // void ellipse(x, y, radiusX, radiusY, rotation, startAngle, endAngle[, anticlockwise]);

    // https://developer.mozilla.org/en-US/docs/Web/API/CanvasRenderingContext2D/rect
    void rect(int x, int y, int width, int height)
    {
        cairo_rectangle(m_cairo.get(), x, y, width, height);
    }

    void stroke()
    {
        cairo_set_source(m_cairo.get(), m_strokePattern.get());
        cairo_stroke(m_cairo.get());
    }

    void scale(double sx, double sy) { cairo_scale(m_cairo.get(), sx, sy); }

    // Compositing
    // Drawing images
    // Pixel manipulation
    // The canvas state
    // https://developer.mozilla.org/en-US/docs/Web/API/CanvasRenderingContext2D/save
    void save() { cairo_save(m_cairo.get()); }

    // https://developer.mozilla.org/en-US/docs/Web/API/CanvasRenderingContext2D/restore
    void restore() { cairo_restore(m_cairo.get()); }
};
