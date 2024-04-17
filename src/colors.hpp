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

#include <cstdint>
#include <string>
#include <unordered_map>

static uint32_t color_from_rgba(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
    return static_cast<uint32_t>(r) << 24 | static_cast<uint32_t>(g) << 16 | static_cast<uint32_t>(b) << 8 | static_cast<uint32_t>(a);
}

static uint32_t color_from_name(std::string name)
{
    const static std::unordered_map<std::string, uint32_t> map = {
        {"black", 0x000000},
        {"navy", 0x000080},
        {"darkblue", 0x00008b},
        {"mediumblue", 0x0000cd},
        {"blue", 0x0000ff},
        {"darkgreen", 0x006400},
        {"green", 0x008000},
        {"teal", 0x008080},
        {"darkcyan", 0x008b8b},
        {"deepskyblue", 0x00bfff},
        {"darkturquoise", 0x00ced1},
        {"mediumspringgreen", 0x00fa9a},
        {"lime", 0x00ff00},
        {"springgreen", 0x00ff7f},
        {"aqua", 0x00ffff},
        {"cyan", 0x00ffff},
        {"midnightblue", 0x191970},
        {"dodgerblue", 0x1e90ff},
        {"lightseagreen", 0x20b2aa},
        {"forestgreen", 0x228b22},
        {"seagreen", 0x2e8b57},
        {"darkslategray", 0x2f4f4f},
        {"limegreen", 0x32cd32},
        {"mediumseagreen", 0x3cb371},
        {"turquoise", 0x40e0d0},
        {"royalblue", 0x4169e1},
        {"steelblue", 0x4682b4},
        {"darkslateblue", 0x483d8b},
        {"mediumturquoise", 0x48d1cc},
        {"indigo", 0x4b0082},
        {"darkolivegreen", 0x556b2f},
        {"cadetblue", 0x5f9ea0},
        {"cornflowerblue", 0x6495ed},
        {"rebeccapurple", 0x663399},
        {"mediumaquamarine", 0x66cdaa},
        {"dimgrey", 0x696969},
        {"dimgray", 0x696969},
        {"slateblue", 0x6a5acd},
        {"olivedrab", 0x6b8e23},
        {"slategrey", 0x708090},
        {"slategray", 0x708090},
        {"lightslategrey", 0x778899},
        {"lightslategray", 0x778899},
        {"mediumslateblue", 0x7b68ee},
        {"lawngreen", 0x7cfc00},
        {"chartreuse", 0x7fff00},
        {"aquamarine", 0x7fffd4},
        {"maroon", 0x800000},
        {"purple", 0x800080},
        {"olive", 0x808000},
        {"grey", 0x808080},
        {"gray", 0x808080},
        {"skyblue", 0x87ceeb},
        {"lightskyblue", 0x87cefa},
        {"blueviolet", 0x8a2be2},
        {"darkred", 0x8b0000},
        {"darkmagenta", 0x8b008b},
        {"saddlebrown", 0x8b4513},
        {"darkseagreen", 0x8fbc8f},
        {"lightgreen", 0x90ee90},
        {"mediumpurple", 0x9370db},
        {"darkviolet", 0x9400d3},
        {"palegreen", 0x98fb98},
        {"darkorchid", 0x9932cc},
        {"yellowgreen", 0x9acd32},
        {"sienna", 0xa0522d},
        {"brown", 0xa52a2a},
        {"darkgrey", 0xa9a9a9},
        {"darkgray", 0xa9a9a9},
        {"lightblue", 0xadd8e6},
        {"greenyellow", 0xadff2f},
        {"paleturquoise", 0xafeeee},
        {"lightsteelblue", 0xb0c4de},
        {"powderblue", 0xb0e0e6},
        {"firebrick", 0xb22222},
        {"darkgoldenrod", 0xb8860b},
        {"mediumorchid", 0xba55d3},
        {"rosybrown", 0xbc8f8f},
        {"darkkhaki", 0xbdb76b},
        {"silver", 0xc0c0c0},
        {"mediumvioletred", 0xc71585},
        {"indianred", 0xcd5c5c},
        {"peru", 0xcd853f},
        {"chocolate", 0xd2691e},
        {"tan", 0xd2b48c},
        {"lightgrey", 0xd3d3d3},
        {"lightgray", 0xd3d3d3},
        {"thistle", 0xd8bfd8},
        {"orchid", 0xda70d6},
        {"goldenrod", 0xdaa520},
        {"palevioletred", 0xdb7093},
        {"crimson", 0xdc143c},
        {"gainsboro", 0xdcdcdc},
        {"plum", 0xdda0dd},
        {"burlywood", 0xdeb887},
        {"lightcyan", 0xe0ffff},
        {"lavender", 0xe6e6fa},
        {"darksalmon", 0xe9967a},
        {"violet", 0xee82ee},
        {"palegoldenrod", 0xeee8aa},
        {"lightcoral", 0xf08080},
        {"khaki", 0xf0e68c},
        {"aliceblue", 0xf0f8ff},
        {"honeydew", 0xf0fff0},
        {"azure", 0xf0ffff},
        {"sandybrown", 0xf4a460},
        {"wheat", 0xf5deb3},
        {"beige", 0xf5f5dc},
        {"whitesmoke", 0xf5f5f5},
        {"mintcream", 0xf5fffa},
        {"ghostwhite", 0xf8f8ff},
        {"salmon", 0xfa8072},
        {"antiquewhite", 0xfaebd7},
        {"linen", 0xfaf0e6},
        {"lightgoldenrodyellow", 0xfafad2},
        {"oldlace", 0xfdf5e6},
        {"red", 0xff0000},
        {"fuchsia", 0xff00ff},
        {"magenta", 0xff00ff},
        {"deeppink", 0xff1493},
        {"orangered", 0xff4500},
        {"tomato", 0xff6347},
        {"hotpink", 0xff69b4},
        {"coral", 0xff7f50},
        {"darkorange", 0xff8c00},
        {"lightsalmon", 0xffa07a},
        {"orange", 0xffa500},
        {"lightpink", 0xffb6c1},
        {"pink", 0xffc0cb},
        {"gold", 0xffd700},
        {"peachpuff", 0xffdab9},
        {"navajowhite", 0xffdead},
        {"moccasin", 0xffe4b5},
        {"bisque", 0xffe4c4},
        {"mistyrose", 0xffe4e1},
        {"blanchedalmond", 0xffebcd},
        {"papayawhip", 0xffefd5},
        {"lavenderblush", 0xfff0f5},
        {"seashell", 0xfff5ee},
        {"cornsilk", 0xfff8dc},
        {"lemonchiffon", 0xfffacd},
        {"floralwhite", 0xfffaf0},
        {"snow", 0xfffafa},
        {"yellow", 0xffff00},
        {"lightyellow", 0xffffe0},
        {"ivory", 0xfffff0},
        {"white", 0xffffff},
    };

    std::transform(name.begin(), name.end(), name.begin(),
                   [](unsigned char c)
                   { return std::tolower(c); });

    auto iter = map.find(name);
    if (iter == map.end())
    {
        return 0;
    }

    return iter->second << 8 | 255;
};

static uint32_t color_from_string(std::string name)
{
    float d = 1.0;
    uint8_t r = 0, g = 0, b = 0, a = 255;
    if (3 == std::sscanf(name.c_str(), "rgb ( %hhu , %hhu , %hhu )", &r, &g, &b))
    {
        return color_from_rgba(r, g, b, 255);
    }
    if (3 == std::sscanf(name.c_str(), "rgba ( %hhu , %hhu , %hhu , %f )", &r, &g, &b, &d))
    {
        return color_from_rgba(r, g, b, static_cast<uint8_t>(std::min(255.0, d * 255.0)));
    }
    auto i = std::sscanf(name.c_str(), "#%02hhx%02hhx%02hhx%02hhx", &r, &g, &b, &a);
    if (3 == i || 4 == i)
    {
        return color_from_rgba(r, g, b, a);
    }

    return color_from_name(name);
}
