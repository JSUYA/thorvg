/*
 * Copyright (c) 2020 - 2026 ThorVG project. All rights reserved.

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

#include <math.h>
#include <cstring>
#include "tvgStr.h"
#include "tvgMath.h"
#include "tvgLoader.h"
#include "tvgSvgLoader.h"
#include "tvgSvgParser.h"
#include "tvgSvgBuilder.h"

/************************************************************************/
/* Internal Class Implementation                                        */
/************************************************************************/

void SvgLoader::clear(bool all)
{
    //flush out the intermediate data
    tvg::free(loaderData.svgParse);
    loaderData.svgParse = nullptr;

    ARRAY_FOREACH(p, loaderData.gradients) {
        (*p)->clear();
        tvg::free(*p);
    }
    loaderData.gradients.reset();
    loaderData.gradientStack.reset();

    _free(loaderData.doc);
    loaderData.doc = nullptr;
    loaderData.stack.reset();

    if (!all) return;

    ARRAY_FOREACH(p, loaderData.images) tvg::free(*p);
    loaderData.images.reset();

    ARRAY_FOREACH(p, loaderData.fonts) {
        Text::unload(p->name);
        tvg::free(p->decoded);
        tvg::free(p->name);
    }
    loaderData.fonts.reset();

    if (copy) tvg::free((char*)content);

    if (root) {
        root->unref();
        root = nullptr;
    }

    size = 0;
    content = nullptr;
    copy = false;
}


void SvgLoader::run(unsigned tid)
{
    //According to the SVG standard the value of the width/height of the viewbox set to 0 disables rendering
    if ((viewFlag & SvgViewFlag::Viewbox) && (fabsf(vbox.w) <= FLOAT_EPSILON || fabsf(vbox.h) <= FLOAT_EPSILON)) {
        TVGLOG("SVG", "The <viewBox> width and/or height set to 0 - rendering disabled.");
        root = Scene::gen();
    } else {
        SvgParser parser(content, size, loaderData);
        if (parser.parse()) {
            if (loaderData.doc) {
                SvgBuilder builder;
                root = builder.build(loaderData, vbox, w, h, align, meetOrSlice, svgPath, viewFlag);

                //In case no viewbox and width/height data is provided the completion of loading
                //has to be forced, in order to establish this data based on the whole picture.
                if (!(viewFlag & SvgViewFlag::Viewbox)) {
                    //Override viewbox & size again after svg loading.
                    vbox = loaderData.doc->node.doc.vbox;
                    w = loaderData.doc->node.doc.w;
                    h = loaderData.doc->node.doc.h;
                }
            }
        }
    }
    root->ref();
    clear(false);
}


/************************************************************************/
/* External Class Implementation                                        */
/************************************************************************/

SvgLoader::SvgLoader() : ImageLoader(FileType::Svg)
{
}


SvgLoader::~SvgLoader()
{
    done();
    clear();
}


bool SvgLoader::header()
{
    //For valid check, only <svg> tag is parsed first.
    //If the <svg> tag is found, the loaded file is valid and stores viewbox information.
    //After that, the remaining content data is parsed in order with async.
    viewFlag = SvgViewFlag::None;

    {
        SvgParser parser(content, size, loaderData);
        if (!parser.header()) {
            TVGLOG("SVG", "No SVG File. There is no <svg/>");
            return false;
        }
    }

    viewFlag = loaderData.doc->node.doc.viewFlag;
    align = loaderData.doc->node.doc.align;
    meetOrSlice = loaderData.doc->node.doc.meetOrSlice;

    if (viewFlag & SvgViewFlag::Viewbox) {
        vbox = loaderData.doc->node.doc.vbox;

        if (viewFlag & SvgViewFlag::Width) w = loaderData.doc->node.doc.w;
        else {
            w = loaderData.doc->node.doc.vbox.w;
            if (viewFlag & SvgViewFlag::WidthInPercent) {
                w *= loaderData.doc->node.doc.w;
                viewFlag = (viewFlag ^ SvgViewFlag::WidthInPercent);
            }
            viewFlag = (viewFlag | SvgViewFlag::Width);
        }
        if (viewFlag & SvgViewFlag::Height) h = loaderData.doc->node.doc.h;
        else {
            h = loaderData.doc->node.doc.vbox.h;
            if (viewFlag & SvgViewFlag::HeightInPercent) {
                h *= loaderData.doc->node.doc.h;
                viewFlag = (viewFlag ^ SvgViewFlag::HeightInPercent);
            }
            viewFlag = (viewFlag | SvgViewFlag::Height);
        }
    //In case no viewbox and width/height data is provided the completion of loading
    //has to be forced, in order to establish this data based on the whole picture.
    } else {
        //Before loading, set default viewbox & size if they are empty
        vbox.x = vbox.y = 0.0f;
        if (viewFlag & SvgViewFlag::Width) {
            vbox.w = w = loaderData.doc->node.doc.w;
        } else {
            vbox.w = 1.0f;
            if (viewFlag & SvgViewFlag::WidthInPercent) {
                w = loaderData.doc->node.doc.w;
            } else w = 1.0f;
        }

        if (viewFlag & SvgViewFlag::Height) {
            vbox.h = h = loaderData.doc->node.doc.h;
        } else {
            vbox.h = 1.0f;
            if (viewFlag & SvgViewFlag::HeightInPercent) {
                h = loaderData.doc->node.doc.h;
            } else h = 1.0f;
        }

        run(0);
    }
    return true;
}


bool SvgLoader::open(const char* data, uint32_t size, TVG_UNUSED const char* rpath, bool copy)
{
    if (copy) {
        content = tvg::malloc<char>(size + 1);
        if (!content) return false;
        memcpy((char*)content, data, size);
        content[size] = '\0';
    } else content = (char*)data;

    this->size = size;
    this->copy = copy;

    return header();
}


bool SvgLoader::open(const char* path)
{
#ifdef THORVG_FILE_IO_SUPPORT
    if ((content = LoadModule::open(path, size, true))) {
        copy = true;
        return header();
    }
#endif
    return false;
}


bool SvgLoader::resize(Paint* paint, float w, float h)
{
    if (!paint) return false;

    auto sx = w / this->w;
    auto sy = h / this->h;
    Matrix m = {sx, 0, 0, 0, sy, 0, 0, 0, 1};
    paint->transform(m);

    return true;
}


bool SvgLoader::read()
{
    if (!content || size == 0) return false;

    if (!LoadModule::read() || root) return true;

    TaskScheduler::request(this);

    return true;
}


bool SvgLoader::close()
{
    if (!LoadModule::close()) return false;
    this->done();
    clear();
    return true;
}


Paint* SvgLoader::paint()
{
    this->done();
    if (root) {
        //Primary usage: sharing the svg
        if (root->refCnt() == 1) return root;
        return root->duplicate();
    }
    return nullptr;
}
