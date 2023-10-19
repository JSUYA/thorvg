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

#include <Elementary.h>
#include <iostream>
#include <string.h>
#include <thread>
#include <thorvg.h>

using namespace std;

void exportGif()
{
    auto animation = tvg::Animation::gen();
    auto picture = animation->picture();
    if (picture->load(EXAMPLE_DIR"/walker.json") != tvg::Result::Success) {
        cout << "Lottie is not supported. Did you enable Lottie Loader?" << endl;
        return;
    }

    auto saver = tvg::Saver::gen();
    if (saver->save(std::move(animation), EXAMPLE_DIR"/test.gif") == tvg::Result::Success) {
        saver->sync();
        cout << "Successfully exported to test.gif." << endl;
        return;
    }
    cout << "Problem with saving the json file. Did you enable Gif Saver?" << endl;
}

void lottieDirCallback(const char* name, const char* path, void* data)
{

    //ignore if not lottie file.
    const char *ext = name + strlen(name) - 4;
    if (strcmp(ext, "json")) return;

    char buf[PATH_MAX];
    snprintf(buf, sizeof(buf), "/%s/%s", path, name);

    //Animation Controller
    auto animation = tvg::Animation::gen();
    auto picture = animation->picture();

    if (picture->load(buf) != tvg::Result::Success) {
        cout << "Lottie is not supported. Did you enable Lottie Loader?" << endl;
        return;
    }
    buf[strlen(buf) - 5] = '\0';
    strcat(buf, ".gif");
    auto saver = tvg::Saver::gen();
    if (saver->save(std::move(animation), buf) == tvg::Result::Success) {
        saver->sync();
        cout << "Successfully exported to "<< buf << endl;
        return;
    }
    cout << "Problem with saving the json file. Did you enable Gif Saver?" << endl;
}


/************************************************************************/
/* Main Code                                                            */
/************************************************************************/

int main(int argc, char **argv)
{
    tvg::CanvasEngine tvgEngine = tvg::CanvasEngine::Sw;

    if (argc > 1) {
        if (!strcmp(argv[1], "gl")) tvgEngine = tvg::CanvasEngine::Gl;
    }

    //Initialize ThorVG Engine
    if (tvgEngine == tvg::CanvasEngine::Sw) {
        cout << "tvg engine: software" << endl;
    } else {
        cout << "tvg engine: opengl" << endl;
    }

    //Threads Count
    auto threads = std::thread::hardware_concurrency();

    //Initialize ThorVG Engine
    if (tvg::Initializer::init(tvgEngine, threads) == tvg::Result::Success) {

        eina_file_dir_list(EXAMPLE_DIR, EINA_TRUE, lottieDirCallback, nullptr);
        exportGif();

        //Terminate ThorVG Engine
        tvg::Initializer::term(tvgEngine);

    } else {
        cout << "engine is not supported" << endl;
    }
    return 0;
}
