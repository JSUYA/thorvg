/*
 * Copyright (c) 2026 ThorVG project. All rights reserved.

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

#ifndef _TVG_SVG_ANIMATION_H_
#define _TVG_SVG_ANIMATION_H_

#include "tvgSvgCommon.h"

/*
 * SVG SMIL animation engine.
 *
 * The parser collects every <animate>/<set>/<animateTransform>/<animateMotion>
 * element into SvgParserContext::animations, each pointing at the target SvgNode.
 * Once parsing is complete svgAnimationResolve() pre-computes timing/keyframe data
 * and a snapshot of every animated attribute's base value. During playback
 * svgAnimationApply() mutates the target SvgNode attributes for a given time so the
 * regular scene builder can render the animated frame.
 */

//Finalize timing/keyframe data and snapshot base values. Returns total duration (sec).
float svgAnimationResolve(Array<SvgSmilAnim*>& animations);

//Mutate every target SvgNode so it reflects the animation state at the given time (sec).
void svgAnimationApply(Array<SvgSmilAnim*>& animations, float time);

//Whether the given attribute name can be animated by the current engine.
bool svgAnimationSupported(SvgSmilAnim* anim);

#endif //_TVG_SVG_ANIMATION_H_
