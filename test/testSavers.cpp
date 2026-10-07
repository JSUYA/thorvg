/*
 * Copyright (c) 2021 - 2026 ThorVG project. All rights reserved.

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

#include <thorvg.h>
#include <fstream>
#include "config.h"
#include "testFramework.h"

using namespace tvg;
using namespace std;

TEST_CASE("Saver Creation", "[tvgSavers]")
{
    auto saver = unique_ptr<Saver>(Saver::gen());
    REQUIRE(saver);
}

#if defined(THORVG_GIF_SAVER_SUPPORT) && defined(THORVG_LOTTIE_LOADER_SUPPORT)

TEST_CASE("Save a lottie into gif", "[tvgSavers]") {
    REQUIRE(Initializer::init() == Result::Success);
    {
        auto animation = Animation::gen();
        auto picture = animation->picture();
        REQUIRE(picture->load(TEST_DIR"/test.lot") == Result::Success);
        REQUIRE(picture->size(100, 100) == Result::Success);

        auto bg = Shape::gen();
        REQUIRE(bg->fill(255, 255, 255) == Result::Success);
        REQUIRE(bg->appendRect(0, 0, 100, 100) == Result::Success);

        auto saver =  unique_ptr<Saver>(Saver::gen());
        REQUIRE(saver);

        REQUIRE(saver->background(bg) == Result::Success);
        REQUIRE(saver->save(animation, TEST_DIR"/test.gif") == Result::Success);
        REQUIRE(saver->sync() == Result::Success);
    }
    REQUIRE(Initializer::term() == Result::Success);
}
#endif

#ifdef THORVG_SVG_SAVER_SUPPORT

TEST_CASE("Save a paint into svg", "[tvgSavers]")
{
    REQUIRE(Initializer::init() == Result::Success);
    {
        auto scene = Scene::gen();
        REQUIRE(scene->add(SceneEffect::GaussianBlur, 2.0, 0, 0, 100) == Result::Success);

        auto shape = Shape::gen();
        REQUIRE(shape->appendRect(0, 0, 100, 100, 10, 10) == Result::Success);
        auto fill = LinearGradient::gen();
        REQUIRE(fill->linear(0, 0, 100, 100) == Result::Success);
        Fill::ColorStop stops[2] = {{0, 255, 0, 0, 255}, {1, 0, 0, 255, 128}};
        REQUIRE(fill->colorStops(stops, 2) == Result::Success);
        REQUIRE(shape->fill(fill) == Result::Success);
        REQUIRE(shape->strokeWidth(5) == Result::Success);
        REQUIRE(shape->strokeFill(0, 0, 0) == Result::Success);
        REQUIRE(shape->blend(BlendMethod::Multiply) == Result::Success);

        auto clipper = Shape::gen();
        REQUIRE(clipper->appendCircle(50, 50, 50, 50) == Result::Success);
        REQUIRE(shape->clip(clipper) == Result::Success);

        auto mask = Shape::gen();
        REQUIRE(mask->appendCircle(50, 50, 30, 30) == Result::Success);
        REQUIRE(mask->fill(255, 255, 255) == Result::Success);
        REQUIRE(shape->mask(mask, MaskMethod::InvAlpha) == Result::Success);
        REQUIRE(scene->add(shape) == Result::Success);

#ifdef THORVG_PNG_LOADER_SUPPORT
        auto picture = Picture::gen();
        REQUIRE(picture->load(TEST_DIR "/test.png") == Result::Success);
        REQUIRE(picture->size(50, 50) == Result::Success);
        REQUIRE(scene->add(picture) == Result::Success);
#endif

        auto saver = unique_ptr<Saver>(Saver::gen());
        REQUIRE(saver);
        REQUIRE(saver->save(scene, TEST_DIR "/test.svg") == Result::Success);
        REQUIRE(saver->sync() == Result::Success);

        // empty paint has no view size
        REQUIRE(saver->save(Shape::gen(), TEST_DIR "/test.svg") == Result::Unknown);

        // animation is not supported
        REQUIRE(saver->save(Animation::gen(), TEST_DIR "/test.svg") != Result::Success);
    }
    REQUIRE(Initializer::term() == Result::Success);
}

#if defined(THORVG_SVG_LOADER_SUPPORT)

TEST_CASE("Load the saved svg", "[tvgSavers]")
{
    REQUIRE(Initializer::init() == Result::Success);
    {
        auto picture = Picture::gen();
        REQUIRE(picture->load(TEST_DIR "/test.svg") == Result::Success);

        float w, h;
        REQUIRE(picture->size(&w, &h) == Result::Success);
        REQUIRE(w == Approx(102.5f).margin(0.000001));
        REQUIRE(h == Approx(102.5f).margin(0.000001));

        Paint::rel(picture);
    }
    REQUIRE(Initializer::term() == Result::Success);
}

#ifdef THORVG_PNG_LOADER_SUPPORT

TEST_CASE("Save a picture with origin into svg", "[tvgSavers]")
{
    REQUIRE(Initializer::init() == Result::Success);
    {
        auto picture = Picture::gen();
        REQUIRE(picture->load(TEST_DIR "/test.png") == Result::Success);
        REQUIRE(picture->size(100, 100) == Result::Success);
        REQUIRE(picture->origin(0.5f, 0.5f) == Result::Success);
        REQUIRE(picture->translate(100, 100) == Result::Success);

        auto saver = unique_ptr<Saver>(Saver::gen());
        REQUIRE(saver->save(picture, TEST_DIR "/test.svg") == Result::Success);
        REQUIRE(saver->sync() == Result::Success);

        auto loaded = Picture::gen();
        REQUIRE(loaded->load(TEST_DIR "/test.svg") == Result::Success);

        float w, h;
        REQUIRE(loaded->size(&w, &h) == Result::Success);
        REQUIRE(w == Approx(150.0f).margin(0.000001));
        REQUIRE(h == Approx(150.0f).margin(0.000001));

        Paint::rel(loaded);
    }
    REQUIRE(Initializer::term() == Result::Success);
}

#endif

#endif

#endif