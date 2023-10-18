/*
 * Copyright (c) 2023 the ThorVG project. All rights reserved.

 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "gif.h"
#include "tvgGifSaver.h"

#ifdef _WIN32
    #include <malloc.h>
#elif defined(__linux__)
    #include <alloca.h>
#else
    #include <stdlib.h>
#endif

/************************************************************************/
/* Internal Class Implementation                                        */
/************************************************************************/

bool GifSaver::writeFrame(Animation* animation, const string& path, int width, int height, int frameDelay)
{
    std::unique_ptr<tvg::SwCanvas> canvas = nullptr;
    canvas = tvg::SwCanvas::gen();
    if (canvas == nullptr) { 
        return false;
    }

    buffer = (uint32_t*)realloc(buffer, sizeof(uint32_t) * width * height);
    canvas->push(tvg::cast<tvg::Picture>(animation->picture()));
    auto sw_canvas = reinterpret_cast<tvg::SwCanvas*>(canvas.get());
    sw_canvas->target(buffer, width, width, height, tvg::SwCanvas::ABGR8888); 

    GifWriter handle;
    GifBegin(&handle, path.c_str(), width, height, 2);

    size_t frameCount = animation->totalFrame();
    for (size_t frame = 0; frame < frameCount; frame++) {
        animation->frame(frame);
        canvas->update(animation->picture());

        if (canvas->draw() == tvg::Result::Success) { 
            canvas->sync();
        }

        GifWriteFrame(&handle, reinterpret_cast<uint8_t*>(buffer), width, height, frameDelay);
    }

    GifEnd(&handle);
    return true;
}


/************************************************************************/
/* External Class Implementation                                        */
/************************************************************************/

GifSaver::~GifSaver()
{
    close();
}


bool GifSaver::close()
{
    if (buffer) {
        free(buffer);
        buffer = nullptr;
    }
    return true;
}


bool GifSaver::save(Paint* paint, const string& path, bool compress)
{
    TVGLOG("GIF_SAVER", "tvg:Paint to Gif saver is not supported yet.");
    return false;
}


bool GifSaver::save(Animation* animation, const string& path, int frameDelay)
{
    auto picture = animation->picture();
    close();

    float x, y, width, height;
    x = y = 0;
    picture->bounds(&x, &y, &width, &height, false);

    //cut off the negative space
    if (x < 0) width += x;
    if (y < 0) height += y;

    if (width < FLT_EPSILON || height < FLT_EPSILON) {
        TVGLOG("GIF_SAVER", "Saving animation(%p) has zero view size.", animation);
        return false;
    }

    if (!writeFrame(animation, path, width, height, frameDelay)) {
        TVGLOG("GIF_SAVER", "Fail to make gif frame.");
        return false;
    }

    return true;
}
