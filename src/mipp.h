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

#ifndef _MIPP_H
#define _MIPP_H

/**
 * @brief C bindings for Mipp
 *
 */

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
    typedef struct mipp
    {
        void *priv;
        int video_in_count;
    } mipp_t;

    extern int mipp_init(mipp_t *mipp, char *script_path, void *opaque,
                         int (*receive_video_frame)(void *, int width, int height, double pts, uint8_t *data),
                         void log(int level, const char *msg));

    extern void mipp_free(mipp_t *mipp);

    extern int mipp_send_video_frame(mipp_t *mipp, int width, int height, int stride, double pts, uint8_t *data, int in_pad);

#ifdef __cplusplus
} // extern "C"
#endif
#endif // _MIPP_H
