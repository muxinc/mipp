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

#include "cairo.hpp"
#include "mipp.h"

#include <iostream>
#include <stdio.h>

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        fprintf(stderr, "usage: %s <url>\n", argv[0]);
        return 1;
    }

    int w = 1920;
    int h = 1080;
    std::vector<uint8_t> video_data(w * h * 4);

    int channels = 2;
    int sample_count = 1024;
    int _sample_rate = 48000;
    std::vector<uint8_t> audio_data(sample_count * channels * 2);

    mipp_t mipp;
    mipp_init(
        &mipp, argv[1], nullptr,
        [](void *opaque, int width, int height, double pts, uint8_t *data) -> int
        {
            fprintf(stderr, "received video frame: %dx%d\n", width, height);
            auto c = cairo(width, height, data);
            c.save_png("test.png");
            return 0;
        },
        [](int level, const char *msg)
        {
            fprintf(stderr, "log: %d: %s\n", level, msg);
        });

    fprintf(stderr, "sending video frame: %dx%d\n", w, h);
    mipp_send_video_frame(&mipp, w, h, w * 4, 0.0, video_data.data(), 0);

    mipp_free(&mipp);
}
