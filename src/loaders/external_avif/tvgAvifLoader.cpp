/*
 * Copyright (c) 2025 the ThorVG project. All rights reserved.

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

#include "tvgAvifLoader.h"

#include <avif/avif.h>

/************************************************************************/
/* Internal Class Implementation                                        */
/************************************************************************/


/************************************************************************/
/* External Class Implementation                                        */
/************************************************************************/

AvifLoader::AvifLoader() : ImageLoader(FileType::Avif)
{
    decoder = avifDecoderCreate();
}


AvifLoader::~AvifLoader()
{
    //avifRGBImageFreePixels(&rgb);
    //avifDecoderDestroy(decoder);
}


bool AvifLoader::open(const char* path)
{
#ifdef THORVG_FILE_IO_SUPPORT
    auto avifFile = fopen(path, "rb");
    if (!avifFile) return false;

    auto ret = false;
    avifResult result;

    //determine size
    if (fseek(avifFile, 0, SEEK_END) < 0) goto finalize;
    if (((size = ftell(avifFile)) < 1)) goto finalize;
    if (fseek(avifFile, 0, SEEK_SET)) goto finalize;

    data = tvg::malloc<unsigned char*>(size);
    if (!data) goto finalize;

    freeData = true;

    if (fread(data, size, 1, avifFile) < 1) return false;

    int width, height, subSample, colorSpace;
    result = avifDecoderSetIOMemory(decoder, data, size);

    if (result != AVIF_RESULT_OK) {
        TVGERR("AVIF LOADER", "%s", avifResultToString(result));
        avifDecoderDestroy(decoder);
        //goto failure;
        return false;
    }

    result = avifDecoderParse(decoder);
    if (result != AVIF_RESULT_OK) {
        TVGERR("AVIF LOADER","%s", avifResultToString(result));
        return false;
    }

    image = decoder->image;

printf("Parsed AVIF: %ux%u (%ubpc)\n", decoder->image->width, decoder->image->height, decoder->image->depth);


    w = static_cast<float>(image->width);
    h = static_cast<float>(image->height);
    ret = true;



finalize:
    fclose(avifFile);
    return ret;
#else
    return false;
#endif
}


bool AvifLoader::open(const char* data, uint32_t size, TVG_UNUSED const char* rpath, bool copy)
{
    int width, height, subSample, colorSpace;
    avifResult result = avifDecoderSetIOMemory(decoder, reinterpret_cast<const unsigned char*>(data), size);


    if (result != AVIF_RESULT_OK) {
        TVGERR("AVIF LOADER", "%s", avifResultToString(result));
        avifDecoderDestroy(decoder);
        return false;
    }

    if (copy) {
        this->data = tvg::malloc<unsigned char*>(size);
        if (!this->data) return false;
        memcpy((unsigned char *)this->data, data, size);
        freeData = true;
    } else {
        this->data = (unsigned char *) data;
        freeData = false;
    }

    w = static_cast<float>(width);
    h = static_cast<float>(height);
    this->size = size;

    return true;
}

void ConvertRGBToARGB(avifRGBImage *rgb, unsigned char *argbBuffer) {
    int width = rgb->width;
    int height = rgb->height;
    int rowBytes = rgb->rowBytes;
    int pixelBytes = rgb->format == AVIF_RGB_FORMAT_RGBA ? 4 : 3;

    for (int y = 0; y < height; ++y) {
        const unsigned char *srcRow = rgb->pixels + y * rowBytes;
        for (int x = 0; x < width; ++x) {
            const unsigned char *srcPixel = srcRow + x * pixelBytes;
            unsigned char *dstPixel = argbBuffer + (y * width + x) * 4;
            unsigned char a = (pixelBytes == 4) ? srcPixel[3] : 255; // A
            unsigned char r = srcPixel[0]; // R
            unsigned char g = srcPixel[1]; // G
            unsigned char b = srcPixel[2]; // B

            // ARGB 포맷으로 변환
            dstPixel[0] = b;//(pixelBytes == 4) ? srcPixel[3] : 255; // A
            dstPixel[1] = g;//srcPixel[0]; // R
            dstPixel[2] = r;//srcPixel[1]; // G
            dstPixel[3] = a;//srcPixel[2]; // B
        }
    }
}

bool AvifLoader::read()
{
    if (!LoadModule::read()) return true;

    if (w == 0 || h == 0) return false;
/*
    //determine the image format
    TJPF format;
    if (ImageLoader::cs == ColorSpace::ARGB8888 || ImageLoader::cs == ColorSpace::ARGB8888S) {
        format = TJPF_BGRX;
        surface.cs = ColorSpace::ARGB8888;
    } else {
        format = TJPF_RGBX;
        surface.cs = ColorSpace::ABGR8888;
    }

    auto image = (unsigned char *)tjAlloc(static_cast<int>(w) * static_cast<int>(h) * tjPixelSize[format]);
    if (!image) return false;

    //decompress jpg image
    if (tjDecompress2(jpegDecompressor, data, size, image, static_cast<int>(w), 0, static_cast<int>(h), format, 0) < 0) {
        TVGERR("JPG LOADER", "%s", tjGetErrorStr());
        tjFree(image);
        image = nullptr;
        return false;
    }*/
        surface.cs = ColorSpace::ARGB8888;
        //surface.cs = ColorSpace::ABGR8888;
printf("%p\n", image);
    avifRGBImage rgb;
    rgb.chromaUpsampling = AVIF_CHROMA_UPSAMPLING_FASTEST;
rgb.ignoreAlpha = true;
    rgb.alphaPremultiplied = true;
while (avifDecoderNextImage(decoder) == AVIF_RESULT_OK) {
    avifRGBImageSetDefaults(&rgb, image);
    avifRGBImageAllocatePixels(&rgb);
    //avifImageYUVToRGB(image, &rgb);
    if (avifImageYUVToRGB(image, &rgb) != AVIF_RESULT_OK) {
        printf("Conversion from YUV failed\n");
    }
printf("asd\n");
for(int  i = 0; i < 10; i++)
{
printf("%u ", rgb.pixels[i]);
}
printf("\n");
}
printf("%f %f\n", w, h);
    uint16_t * pixels = (uint16_t *)rgb.pixels;
printf("%d\n", rgb.depth);
printf("%d\n", rgb.format);
/*        for (int y = 0; y < image->width; y++) {
            for (int x = 0; x < image->height; x += 4) {
                int w = image->width;
                uint16_t r = pixels[y * w + x + 0];
                uint16_t g = pixels[y * w + x + 1];
                uint16_t b = pixels[y * w + x + 2];
                uint16_t a = pixels[y * w + x + 3];

                //pixels[y * w + x + 0] = a;
                //pixels[y * w + x + 1] = b;
                //pixels[y * w + x + 2] = g;
                //pixels[y * w + x + 3] = r;

            }
        }*/
    ConvertRGBToARGB(&rgb, rgb.pixels);

    //setup the surface
    surface.buf8 = reinterpret_cast<unsigned char*>(rgb.pixels);
    surface.stride = w;
    surface.w = w;
    surface.h = h;
    surface.channelSize = sizeof(uint16_t);
    surface.premultiplied = true;

    return true;
}

