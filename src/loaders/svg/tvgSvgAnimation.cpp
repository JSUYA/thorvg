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

#include <cstring>
#include <cstdlib>
#include <cmath>
#include "tvgMath.h"
#include "tvgSvgAnimation.h"
#include "tvgSvgBuilder.h"

/************************************************************************/
/* Attribute slots                                                      */
/************************************************************************/

enum class SlotKind : uint8_t { Invalid, Number, Opacity, Color, Transform, Visibility };
enum class AttrId : uint8_t {
    None = 0, Opacity, FillOpacity, StrokeOpacity, StrokeWidth, StrokeDashOffset,
    Cx, Cy, R, Rx, Ry, X, Y, Width, Height, X1, Y1, X2, Y2,
    Fill, Stroke, Transform, Visibility, Display
};

//Resolve attributeName to (slot, attr) enums once. STR_AS only runs here, never per-frame.
static void _classify(const SvgSmilAnim* anim, SlotKind& slot, AttrId& attr)
{
    slot = SlotKind::Invalid; attr = AttrId::None;
    if (anim->type == SvgAnimType::AnimateTransform) { slot = SlotKind::Transform; attr = AttrId::Transform; return; }

    auto a = anim->attributeName;
    if (!a) return;

    struct { const char* name; SlotKind s; AttrId a; } map[] = {
        {"visibility",SlotKind::Visibility,AttrId::Visibility}, {"display",SlotKind::Visibility,AttrId::Display},
        {"opacity",SlotKind::Opacity,AttrId::Opacity}, {"fill-opacity",SlotKind::Opacity,AttrId::FillOpacity},
        {"stroke-opacity",SlotKind::Opacity,AttrId::StrokeOpacity}, {"stroke-width",SlotKind::Number,AttrId::StrokeWidth},
        {"stroke-dashoffset",SlotKind::Number,AttrId::StrokeDashOffset},
        {"fill",SlotKind::Color,AttrId::Fill}, {"stroke",SlotKind::Color,AttrId::Stroke},
        {"cx",SlotKind::Number,AttrId::Cx}, {"cy",SlotKind::Number,AttrId::Cy}, {"r",SlotKind::Number,AttrId::R},
        {"rx",SlotKind::Number,AttrId::Rx}, {"ry",SlotKind::Number,AttrId::Ry},
        {"x",SlotKind::Number,AttrId::X}, {"y",SlotKind::Number,AttrId::Y},
        {"width",SlotKind::Number,AttrId::Width}, {"height",SlotKind::Number,AttrId::Height},
        {"x1",SlotKind::Number,AttrId::X1}, {"y1",SlotKind::Number,AttrId::Y1},
        {"x2",SlotKind::Number,AttrId::X2}, {"y2",SlotKind::Number,AttrId::Y2},
    };
    for (auto& m : map) if (STR_AS(a, m.name)) { slot = m.s; attr = m.a; return; }
}

static SlotKind _slotKind(const SvgSmilAnim* anim) { return (SlotKind) anim->slot; }
static AttrId _attrId(const SvgSmilAnim* anim) { return (AttrId) anim->attr; }


static bool _getNumber(SvgNode* node, AttrId a, float& out)
{
    auto& n = node->node;
    auto& s = node->style;

    switch (a) {
        case AttrId::Opacity: out = s->opacity / 255.0f; return true;
        case AttrId::FillOpacity: out = s->fill.opacity / 255.0f; return true;
        case AttrId::StrokeOpacity: out = s->stroke.opacity / 255.0f; return true;
        case AttrId::StrokeWidth: out = s->stroke.width; return true;
        case AttrId::StrokeDashOffset: out = s->stroke.dash.offset; return true;
        case AttrId::Cx: out = (node->type == SvgNodeType::Circle) ? n.circle.cx : n.ellipse.cx; return true;
        case AttrId::Cy: out = (node->type == SvgNodeType::Circle) ? n.circle.cy : n.ellipse.cy; return true;
        case AttrId::R: out = n.circle.r; return true;
        case AttrId::Rx: out = (node->type == SvgNodeType::Rect) ? n.rect.rx : n.ellipse.rx; return true;
        case AttrId::Ry: out = (node->type == SvgNodeType::Rect) ? n.rect.ry : n.ellipse.ry; return true;
        case AttrId::X: out = n.rect.x; return true;
        case AttrId::Y: out = n.rect.y; return true;
        case AttrId::Width: out = n.rect.w; return true;
        case AttrId::Height: out = n.rect.h; return true;
        case AttrId::X1: out = n.line.x1; return true;
        case AttrId::Y1: out = n.line.y1; return true;
        case AttrId::X2: out = n.line.x2; return true;
        case AttrId::Y2: out = n.line.y2; return true;
        default: return false;
    }
}


static void _setNumber(SvgNode* node, AttrId a, float v)
{
    auto& n = node->node;
    auto& s = node->style;

    switch (a) {
        case AttrId::Opacity: s->opacity = (uint8_t) tvg::clamp(int(lroundf(v * 255.0f)), 0, 255); return;
        case AttrId::FillOpacity: s->fill.opacity = (uint8_t) tvg::clamp(int(lroundf(v * 255.0f)), 0, 255); return;
        case AttrId::StrokeOpacity: s->stroke.opacity = (uint8_t) tvg::clamp(int(lroundf(v * 255.0f)), 0, 255); return;
        case AttrId::StrokeWidth: s->stroke.width = v; return;
        case AttrId::StrokeDashOffset: s->stroke.dash.offset = v; return;
        case AttrId::Cx: if (node->type == SvgNodeType::Circle) n.circle.cx = v; else n.ellipse.cx = v; return;
        case AttrId::Cy: if (node->type == SvgNodeType::Circle) n.circle.cy = v; else n.ellipse.cy = v; return;
        case AttrId::R: n.circle.r = v; return;
        case AttrId::Rx: if (node->type == SvgNodeType::Rect) { n.rect.rx = v; n.rect.hasRx = true; } else n.ellipse.rx = v; return;
        case AttrId::Ry: if (node->type == SvgNodeType::Rect) { n.rect.ry = v; n.rect.hasRy = true; } else n.ellipse.ry = v; return;
        case AttrId::X: n.rect.x = v; return;
        case AttrId::Y: n.rect.y = v; return;
        case AttrId::Width: n.rect.w = v; return;
        case AttrId::Height: n.rect.h = v; return;
        case AttrId::X1: n.line.x1 = v; return;
        case AttrId::Y1: n.line.y1 = v; return;
        case AttrId::X2: n.line.x2 = v; return;
        case AttrId::Y2: n.line.y2 = v; return;
        default: return;
    }
}


static void _setColor(SvgNode* node, AttrId a, const SvgColor& c)
{
    auto& s = node->style;
    if (a == AttrId::Fill) {
        s->fill.paint.color = c;
        s->fill.paint.none = false;
        s->fill.paint.gradient = nullptr;
        s->fill.paint.curColor = false;
        tvg::free(s->fill.paint.url);
        s->fill.paint.url = nullptr;
    } else if (a == AttrId::Stroke) {
        s->stroke.paint.color = c;
        s->stroke.paint.none = false;
        s->stroke.paint.gradient = nullptr;
        s->stroke.paint.curColor = false;
        tvg::free(s->stroke.paint.url);
        s->stroke.paint.url = nullptr;
    }
}


static bool _getColor(SvgNode* node, AttrId a, SvgColor& out)
{
    auto& s = node->style;
    if (a == AttrId::Fill) { out = s->fill.paint.color; return true; }
    if (a == AttrId::Stroke) { out = s->stroke.paint.color; return true; }
    return false;
}

/************************************************************************/
/* Value parsing                                                        */
/************************************************************************/

static bool _parseNumber(const char* str, float& out)
{
    if (!str) return false;
    while (*str == ' ' || *str == '\t') ++str;
    char* end = nullptr;
    auto v = strtof(str, &end);
    if (end == str) return false;
    out = v;
    return true;
}


static int _hexToInt(char c)
{
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return 0;
}


//Minimal color parser: #rgb, #rrggbb, rgb(r,g,b). Unknown -> false.
static bool _parseColor(const char* str, SvgColor& out)
{
    if (!str) return false;
    while (*str == ' ' || *str == '\t') ++str;

    if (*str == '#') {
        ++str;
        auto len = strlen(str);
        if (len >= 6) {
            out.r = _hexToInt(str[0]) * 16 + _hexToInt(str[1]);
            out.g = _hexToInt(str[2]) * 16 + _hexToInt(str[3]);
            out.b = _hexToInt(str[4]) * 16 + _hexToInt(str[5]);
            return true;
        } else if (len >= 3) {
            out.r = _hexToInt(str[0]) * 17;
            out.g = _hexToInt(str[1]) * 17;
            out.b = _hexToInt(str[2]) * 17;
            return true;
        }
        return false;
    }

    if (!strncmp(str, "rgb", 3)) {
        auto p = strchr(str, '(');
        if (!p) return false;
        ++p;
        float c[3] = {0.0f, 0.0f, 0.0f};
        for (int i = 0; i < 3; ++i) {
            char* end = nullptr;
            c[i] = strtof(p, &end);
            if (end == p) return false;
            if (*end == '%') { c[i] = c[i] * 255.0f / 100.0f; ++end; }
            p = end;
            while (*p == ',' || *p == ' ' || *p == '\t') ++p;
        }
        out.r = (uint8_t) tvg::clamp(int(lroundf(c[0])), 0, 255);
        out.g = (uint8_t) tvg::clamp(int(lroundf(c[1])), 0, 255);
        out.b = (uint8_t) tvg::clamp(int(lroundf(c[2])), 0, 255);
        return true;
    }

    return false;
}

/************************************************************************/
/* Easing / interpolation                                               */
/************************************************************************/

//Solve a cubic-bezier easing curve: given progress x in [0,1] returns eased y.
static float _bezierEasing(float x, float x1, float y1, float x2, float y2)
{
    if (x <= 0.0f) return 0.0f;
    if (x >= 1.0f) return 1.0f;

    auto sampleX = [&](float t) {
        auto mt = 1.0f - t;
        return 3.0f * mt * mt * t * x1 + 3.0f * mt * t * t * x2 + t * t * t;
    };
    auto sampleY = [&](float t) {
        auto mt = 1.0f - t;
        return 3.0f * mt * mt * t * y1 + 3.0f * mt * t * t * y2 + t * t * t;
    };

    //bisection to find t for the given x (robust, fast enough for keyframes)
    auto lo = 0.0f, hi = 1.0f, t = x;
    for (int i = 0; i < 32; ++i) {
        auto xt = sampleX(t);
        if (fabsf(xt - x) < 1e-5f) break;
        if (xt < x) lo = t;
        else hi = t;
        t = 0.5f * (lo + hi);
    }
    return sampleY(t);
}


//Find the keyframe segment for normalized progress p, returns segment index and local factor [0,1].
static int _segment(SvgSmilAnim* anim, float p, float& local)
{
    auto cnt = anim->values.count;
    if (cnt < 2) { local = 0.0f; return 0; }

    auto& kt = anim->keyTimes;
    for (uint32_t i = 0; i + 1 < cnt; ++i) {
        auto t0 = kt[i];
        auto t1 = kt[i + 1];
        if (p <= t1 || i + 2 == cnt) {
            auto span = t1 - t0;
            local = (span > 0.0f) ? tvg::clamp((p - t0) / span, 0.0f, 1.0f) : 0.0f;
            return (int) i;
        }
    }
    local = 1.0f;
    return (int) cnt - 2;
}


static float _ease(SvgSmilAnim* anim, int seg, float local)
{
    if (anim->calcMode == SvgAnimCalcMode::Discrete) return 0.0f;
    if (anim->calcMode == SvgAnimCalcMode::Spline && anim->keySplines.count >= (uint32_t)(seg + 1) * 4) {
        auto s = &anim->keySplines[seg * 4];
        return _bezierEasing(local, s[0], s[1], s[2], s[3]);
    }
    return local; //linear / paced (approximated as linear)
}

/************************************************************************/
/* Timing                                                               */
/************************************************************************/

//Returns the normalized progress [0,1] at the given time, or -1 if inactive.
//cycleOut receives the completed-iteration count (for accumulate="sum").
static float _progress(SvgSmilAnim* anim, float time, int* cycleOut = nullptr)
{
    if (cycleOut) *cycleOut = 0;

    //pick the most recent begin <= time (begins[] is sorted ascending; supports lists & syncbase)
    float begin = INFINITY;
    ARRAY_FOREACH(b, anim->begins) { if (*b <= time) begin = *b; }
    if (std::isinf(begin)) return -1.0f;   //before the first start

    auto t = time - begin;
    if (t < 0.0f) return -1.0f;

    if (anim->dur <= 0.0f) {
        //indefinite duration: <set> holds its value from begin onward
        return (anim->type == SvgAnimType::Set) ? 1.0f : -1.0f;
    }

    auto active = anim->dur * anim->repeatCount;  //INFINITY if indefinite
    if (t >= active) {
        if (anim->fill == SvgAnimFill::Freeze) {
            if (cycleOut && std::isfinite(anim->repeatCount)) *cycleOut = (int) anim->repeatCount - 1;
            return 1.0f;
        }
        return -1.0f;
    }

    if (cycleOut) *cycleOut = (int) (t / anim->dur);
    auto local = fmodf(t, anim->dur) / anim->dur;
    return tvg::clamp(local, 0.0f, 1.0f);
}

/************************************************************************/
/* Base snapshot                                                        */
/************************************************************************/

//Capture the target attribute's base value at resolve time (node still holds parsed values).
static void _snapshot(SvgSmilAnim* anim)
{
    anim->baseValid = false;
    if (!anim->target || !anim->target->style) return;

    switch (_slotKind(anim)) {
        case SlotKind::Number:
        case SlotKind::Opacity:
            anim->baseValid = _getNumber(anim->target, _attrId(anim), anim->baseNumber);
            break;
        case SlotKind::Color:
            anim->baseValid = _getColor(anim->target, _attrId(anim), anim->baseColor);
            break;
        case SlotKind::Transform:
            anim->baseMatrix = anim->target->transform ? *anim->target->transform : tvg::identity();
            anim->baseValid = true;
            break;
        case SlotKind::Visibility:
            anim->baseNumber = 1.0f;   //elements start visible
            anim->baseValid = true;
            break;
        default: break;
    }
}

/************************************************************************/
/* Transform builder                                                    */
/************************************************************************/

static Matrix _buildTransform(SvgAnimTransform type, const float* v, int cnt)
{
    auto m = tvg::identity();
    switch (type) {
        case SvgAnimTransform::Translate: {
            auto tx = (cnt > 0) ? v[0] : 0.0f;
            auto ty = (cnt > 1) ? v[1] : 0.0f;
            m.e13 = tx; m.e23 = ty;
            break;
        }
        case SvgAnimTransform::Scale: {
            auto sx = (cnt > 0) ? v[0] : 1.0f;
            auto sy = (cnt > 1) ? v[1] : sx;
            m.e11 = sx; m.e22 = sy;
            break;
        }
        case SvgAnimTransform::Rotate: {
            auto rad = deg2rad((cnt > 0) ? v[0] : 0.0f);
            auto cx = (cnt > 1) ? v[1] : 0.0f;
            auto cy = (cnt > 2) ? v[2] : 0.0f;
            auto cs = cosf(rad), sn = sinf(rad);
            //translate(cx,cy) * rotate * translate(-cx,-cy)
            m.e11 = cs; m.e12 = -sn; m.e21 = sn; m.e22 = cs;
            m.e13 = cx - cs * cx + sn * cy;
            m.e23 = cy - sn * cx - cs * cy;
            break;
        }
        case SvgAnimTransform::SkewX:
            m.e12 = tanf(deg2rad((cnt > 0) ? v[0] : 0.0f));
            break;
        case SvgAnimTransform::SkewY:
            m.e21 = tanf(deg2rad((cnt > 0) ? v[0] : 0.0f));
            break;
        default: break;
    }
    return m;
}


//Parse up to 3 whitespace/comma separated floats from a transform value string.
static int _parseFloats(const char* str, float* out, int max)
{
    int n = 0;
    const char* p = str;
    while (p && *p && n < max) {
        while (*p == ' ' || *p == ',' || *p == '\t') ++p;
        if (!*p) break;
        char* end = nullptr;
        auto v = strtof(p, &end);
        if (end == p) break;
        out[n++] = v;
        p = end;
    }
    return n;
}

/************************************************************************/
/* In-place paint update                                                */
/************************************************************************/

//Push the node's (already animation-mutated) attribute onto its rendered Paint, in place.
//This avoids rebuilding the whole scene every frame - only the animated paints are touched.
static void _syncPaint(SvgSmilAnim* anim)
{
    auto node = anim->target;
    auto paint = node->paint;
    if (!paint) return;

    auto s = node->style;

    //opacity and transform apply to any Paint (Shape or group Scene).
    if (_attrId(anim) == AttrId::Transform) { paint->transform(node->transform ? *node->transform : tvg::identity()); return; }
    if (_attrId(anim) == AttrId::Opacity) { paint->opacity(s->opacity); return; }

    //fill/stroke/geometry are Shape-only - a group target (Scene) has no such paint to drive.
    if (paint->type() != Type::Shape) return;
    auto shape = static_cast<Shape*>(paint);

    switch (_attrId(anim)) {
        case AttrId::Fill:
        case AttrId::FillOpacity: shape->fill(s->fill.paint.color.r, s->fill.paint.color.g, s->fill.paint.color.b, s->fill.opacity); return;
        case AttrId::Stroke:
        case AttrId::StrokeOpacity: shape->strokeFill(s->stroke.paint.color.r, s->stroke.paint.color.g, s->stroke.paint.color.b, s->stroke.opacity); return;
        case AttrId::StrokeWidth: shape->strokeWidth(s->stroke.width); return;
        case AttrId::StrokeDashOffset: shape->strokeDash(s->stroke.dash.array.data, s->stroke.dash.array.count, s->stroke.dash.offset); return;
        case AttrId::None: return;
        default: svgShapeReset(node, paint); return;   //geometry attribute -> regenerate this shape's path only
    }
}

/************************************************************************/
/* Apply                                                                */
/************************************************************************/

//Monotonic stamp so each apply pass can compose multiple transforms per node.
static uint32_t _frameStamp = 0;

static void _applyOne(SvgSmilAnim* anim, float time)
{
    auto kind = _slotKind(anim);
    int cycle = 0;
    auto p = _progress(anim, time, &cycle);

    //--- Apply with change-detection: if the value is identical to the last frame, skip the
    //--- node/paint write entirely. No dirty flag -> no RLE regen for held/inactive shapes.
    if (kind == SlotKind::Number || kind == SlotKind::Opacity) {
        float val;
        if (p < 0.0f) { if (!anim->baseValid) return; val = anim->baseNumber; }
        else {
            if (anim->fvals.count == 0) return;
            float local; auto seg = _segment(anim, p, local); auto f = _ease(anim, seg, local);
            auto i0 = (uint32_t) seg; auto i1 = (anim->values.count > 1) ? i0 + 1 : i0;
            val = anim->fvals[i0] + (anim->fvals[i1] - anim->fvals[i0]) * f;
            //accumulate="sum": each completed iteration adds the full first->last delta
            if (anim->accumulate && cycle > 0) val += cycle * (anim->fvals[anim->fvals.count - 1] - anim->fvals[0]);
            if (anim->additive == SvgAnimAdditive::Sum && anim->baseValid) val += anim->baseNumber;
        }
        if (anim->lastValid && fabsf(val - anim->lastNumber) < 1e-3f) return;
        anim->lastNumber = val; anim->lastValid = true;
        _setNumber(anim->target, _attrId(anim), val);
        _syncPaint(anim);
        return;
    }

    if (kind == SlotKind::Visibility) {
        float val;
        if (p < 0.0f) val = anim->baseNumber;   //before begin: visible
        else {
            if (anim->fvals.count == 0) return;
            float local; auto seg = _segment(anim, p, local);
            val = anim->fvals[(uint32_t) seg];   //discrete: hold the segment's value
        }
        if (anim->lastValid && fabsf(val - anim->lastNumber) < 0.5f) return;
        anim->lastNumber = val; anim->lastValid = true;
        if (anim->target->paint) anim->target->paint->opacity(val > 0.5f ? anim->target->style->opacity : 0);
        return;
    }

    if (kind == SlotKind::Color) {
        SvgColor out;
        if (p < 0.0f) { if (!anim->baseValid) return; out = anim->baseColor; }
        else {
            if (anim->cvals.count == 0) return;
            float local; auto seg = _segment(anim, p, local); auto f = _ease(anim, seg, local);
            auto i0 = (uint32_t) seg; auto i1 = (anim->values.count > 1) ? i0 + 1 : i0;
            auto& a = anim->cvals[i0]; auto& b = anim->cvals[i1];
            out.r = (uint8_t) tvg::clamp(int(lroundf(a.r + (b.r - a.r) * f)), 0, 255);
            out.g = (uint8_t) tvg::clamp(int(lroundf(a.g + (b.g - a.g) * f)), 0, 255);
            out.b = (uint8_t) tvg::clamp(int(lroundf(a.b + (b.b - a.b) * f)), 0, 255);
        }
        uint32_t pk = ((uint32_t)out.r << 16) | ((uint32_t)out.g << 8) | out.b;
        if (anim->lastValid && anim->lastColor == pk) return;
        anim->lastColor = pk; anim->lastValid = true;
        _setColor(anim->target, _attrId(anim), out);
        _syncPaint(anim);
        return;
    }

    if (kind == SlotKind::Transform) {
        //inactive contributes identity; the static base is restored by the per-frame reset below
        Matrix m = tvg::identity();
        if (p >= 0.0f) {
            auto stride = anim->xfStride;
            float local; auto seg = _segment(anim, p, local); auto f = _ease(anim, seg, local);
            auto i0 = (uint32_t) seg; auto i1 = (anim->values.count > 1) ? i0 + 1 : i0;
            if (stride == 0 || anim->fvals.count < (uint32_t)(i1 + 1) * stride) return;
            auto va = &anim->fvals[i0 * stride]; auto vb = &anim->fvals[i1 * stride];
            float vi[3] = {0, 0, 0};
            for (uint8_t k = 0; k < stride; ++k) vi[k] = va[k] + (vb[k] - va[k]) * f;
            m = _buildTransform(anim->transform, vi, stride);
        }
        //Compose multiple animateTransforms on the same node: reset to the static base once
        //per frame, then post-multiply each animation's matrix in document order.
        if (!anim->target->transform) anim->target->transform = tvg::malloc<Matrix>(sizeof(Matrix));
        if (anim->target->xfStamp != _frameStamp) {
            anim->target->xfStamp = _frameStamp;
            *anim->target->transform = anim->baseMatrix;
        }
        *anim->target->transform = *anim->target->transform * m;
        _syncPaint(anim);
        return;
    }
}

/************************************************************************/
/* Public                                                               */
/************************************************************************/

//Pre-parse keyframe value strings into typed numeric arrays so per-frame work
//never touches strtof again (mirrors how the Lottie loader bakes keyframes at load).
static void _prebake(SvgSmilAnim* anim)
{
    auto n = anim->values.count;
    switch (_slotKind(anim)) {
        case SlotKind::Visibility:
            //visibility/display are discrete keywords -> 0 (hidden/none/collapse) or 1 (shown)
            anim->calcMode = SvgAnimCalcMode::Discrete;
            for (uint32_t i = 0; i < n; ++i) {
                auto v = anim->values[i];
                anim->fvals.push((STR_AS(v, "hidden") || STR_AS(v, "none") || STR_AS(v, "collapse")) ? 0.0f : 1.0f);
            }
            break;
        case SlotKind::Color:
            for (uint32_t i = 0; i < n; ++i) { SvgColor c{}; _parseColor(anim->values[i], c); anim->cvals.push(c); }
            break;
        case SlotKind::Transform: {
            uint8_t stride = 1;
            for (uint32_t i = 0; i < n; ++i) { float v[3] = {0,0,0}; auto cnt = _parseFloats(anim->values[i], v, 3); if (cnt > (int)stride) stride = (uint8_t)cnt; }
            anim->xfStride = stride;
            for (uint32_t i = 0; i < n; ++i) { float v[3] = {0,0,0}; _parseFloats(anim->values[i], v, 3); for (uint8_t k = 0; k < stride; ++k) anim->fvals.push(v[k]); }
            break;
        }
        case SlotKind::Number:
        case SlotKind::Opacity:
            for (uint32_t i = 0; i < n; ++i) { float v = 0.0f; _parseNumber(anim->values[i], v); anim->fvals.push(v); }
            break;
        default: break;
    }

    //calcMode="paced": distribute keyTimes by value distance so motion has constant pace
    if (anim->calcMode == SvgAnimCalcMode::Paced && n > 1 && anim->keyTimes.count == n) {
        Array<float> seg;
        float total = 0.0f;
        for (uint32_t i = 0; i + 1 < n; ++i) {
            float d = 0.0f;
            if (anim->slot == (uint8_t) SlotKind::Color) {
                auto& a = anim->cvals[i]; auto& b = anim->cvals[i + 1];
                d = fabsf((float)b.r - a.r) + fabsf((float)b.g - a.g) + fabsf((float)b.b - a.b);
            } else if (anim->slot == (uint8_t) SlotKind::Transform) {
                auto s = anim->xfStride; auto va = &anim->fvals[i * s]; auto vb = &anim->fvals[(i + 1) * s];
                for (uint8_t k = 0; k < s; ++k) d += fabsf(vb[k] - va[k]);
            } else d = fabsf(anim->fvals[i + 1] - anim->fvals[i]);
            seg.push(d); total += d;
        }
        if (total > 0.0f) {
            anim->keyTimes.clear();
            anim->keyTimes.push(0.0f);
            float acc = 0.0f;
            for (uint32_t i = 0; i + 1 < n; ++i) { acc += seg[i]; anim->keyTimes.push(acc / total); }
        }
        seg.reset();
    }
}


bool svgAnimationSupported(SvgSmilAnim* anim)
{
    SlotKind slot; AttrId attr;
    _classify(anim, slot, attr);
    return slot != SlotKind::Invalid;
}

/************************************************************************/
/* begin resolution (lists + syncbase)                                  */
/************************************************************************/

//Parse a SMIL clock value ("2s", "500ms", "-0.5", "1.5min", "indefinite").
static float _clock(const char* str, const char* end)
{
    while (*str == ' ' || *str == '\t') ++str;
    if (!strncmp(str, "indefinite", 10)) return INFINITY;
    char* e = nullptr;
    auto v = strtof(str, &e);
    if (e == str) return 0.0f;
    while (*e == ' ') ++e;
    if (!strncmp(e, "ms", 2)) return v * 0.001f;
    if (*e == 's') return v;
    if (*e == 'm' && e[1] != 's') return v * 60.0f;
    if (*e == 'h') return v * 3600.0f;
    return v;
    (void) end;
}


static SvgSmilAnim* _findById(Array<SvgSmilAnim*>& anims, const char* id, size_t len)
{
    ARRAY_FOREACH(p, anims) {
        if ((*p)->id && strlen((*p)->id) == len && !strncmp((*p)->id, id, len)) return *p;
    }
    return nullptr;
}


//Resolve one begin token. ret: 0=concrete, 1=resolved syncbase, 2=pending(ref not ready), 3=skip(event/indefinite).
static int _resolveToken(const char* tok, const char* tokEnd, Array<SvgSmilAnim*>& anims, float& out)
{
    while (tok < tokEnd && (*tok == ' ' || *tok == '\t')) ++tok;
    if (tok >= tokEnd) return 3;

    //find a '.' that separates "id.begin"/"id.end" (syncbase/event)
    const char* dot = nullptr;
    for (auto c = tok; c < tokEnd; ++c) if (*c == '.') { dot = c; break; }

    if (dot) {
        auto kw = dot + 1;
        bool isBegin = !strncmp(kw, "begin", 5);
        bool isEnd = !strncmp(kw, "end", 3);
        if (!isBegin && !isEnd) return 3;   //event-based (id.click, ...) - unsupported
        auto ref = _findById(anims, tok, dot - tok);
        if (!ref || ref->begins.count == 0) return 2;   //target not resolved yet
        auto base = ref->begins[0];
        if (isEnd) base += (std::isinf(ref->repeatCount) || ref->dur <= 0.0f) ? ref->dur : ref->dur * ref->repeatCount;
        //optional +/- offset after the keyword
        auto off = kw + (isBegin ? 5 : 3);
        float delta = 0.0f;
        while (off < tokEnd && *off == ' ') ++off;
        if (off < tokEnd && (*off == '+' || *off == '-')) delta = _clock(off, tokEnd);
        out = base + delta;
        return 1;
    }

    if (!strncmp(tok, "indefinite", 10)) return 3;
    //a bare word that isn't a number is an event (e.g. "click") - unsupported
    if (!((*tok >= '0' && *tok <= '9') || *tok == '+' || *tok == '-' || *tok == '.')) return 3;
    out = _clock(tok, tokEnd);
    return 0;
}


//Resolve every animation's beginRaw into begins[] (handles ';' lists and id.begin/id.end syncbase).
static void _resolveBegins(Array<SvgSmilAnim*>& anims)
{
    Array<bool> done;
    ARRAY_FOREACH(p, anims) { (void)p; done.push(false); }

    bool progress = true;
    int guard = 0;
    while (progress && guard++ < (int) anims.count + 4) {
        progress = false;
        for (uint32_t i = 0; i < anims.count; ++i) {
            if (done[i]) continue;
            auto anim = anims[i];
            if (!anim->beginRaw) { anim->begins.push(0.0f); done[i] = true; progress = true; continue; }

            //parse all ';'-separated tokens; defer if any syncbase ref isn't ready
            bool pending = false;
            Array<float> tmp;
            auto s = anim->beginRaw;
            while (*s) {
                while (*s == ' ' || *s == ';') ++s;
                if (!*s) break;
                auto e = s; while (*e && *e != ';') ++e;
                float v;
                auto r = _resolveToken(s, e, anims, v);
                if (r == 2) { pending = true; break; }
                if (r == 0 || r == 1) tmp.push(v);
                s = e;
            }
            if (!pending) {
                ARRAY_FOREACH(t, tmp) anim->begins.push(*t);
                done[i] = true; progress = true;
            }
            tmp.reset();
        }
    }

    //leftover unresolved (cyclic/missing refs): keep only their concrete begins
    for (uint32_t i = 0; i < anims.count; ++i) {
        if (done[i]) continue;
        auto anim = anims[i];
        auto s = anim->beginRaw;
        while (s && *s) {
            while (*s == ' ' || *s == ';') ++s;
            if (!*s) break;
            auto e = s; while (*e && *e != ';') ++e;
            float v;
            if (_resolveToken(s, e, anims, v) == 0) anim->begins.push(v);
            s = e;
        }
    }

    //finalize: sort each begins ascending (simple insertion - lists are tiny) and set the primary
    ARRAY_FOREACH(p, anims) {
        auto anim = *p;
        auto& b = anim->begins;
        for (uint32_t i = 1; i < b.count; ++i) {
            auto v = b[i]; int j = (int) i - 1;
            while (j >= 0 && b[j] > v) { b[j + 1] = b[j]; --j; }
            b[j + 1] = v;
        }
        anim->begin = (b.count > 0) ? b[0] : INFINITY;   //INFINITY => never starts
    }
}


float svgAnimationResolve(Array<SvgSmilAnim*>& animations)
{
    float total = 0.0f;

    //resolve begin lists / syncbase references across all animations first
    _resolveBegins(animations);

    ARRAY_FOREACH(p, animations) {
        auto anim = *p;

        //resolve the attribute to enums once, up front
        SlotKind slot; AttrId attr;
        _classify(anim, slot, attr);
        anim->slot = (uint8_t) slot;
        anim->attr = (uint8_t) attr;

        //default keyTimes: evenly spaced if absent or mismatched
        if (anim->keyTimes.count != anim->values.count && anim->values.count > 0) {
            anim->keyTimes.clear();
            auto n = anim->values.count;
            if (n == 1) anim->keyTimes.push(0.0f);
            else for (uint32_t i = 0; i < n; ++i) anim->keyTimes.push((float) i / (float) (n - 1));
        }

        //precompute end time from the latest begin
        auto lastBegin = (anim->begins.count > 0) ? anim->begins[anim->begins.count - 1] : anim->begin;
        if (std::isinf(lastBegin)) anim->end = 0.0f;   //never starts
        else if (std::isinf(anim->repeatCount) || anim->dur <= 0.0f) anim->end = lastBegin + (anim->dur > 0.0f ? anim->dur : 0.0f);
        else anim->end = lastBegin + anim->dur * anim->repeatCount;
        if (std::isfinite(anim->end) && anim->end > total) total = anim->end;

        _prebake(anim);
        _snapshot(anim);
    }

    return total;
}


void svgAnimationApply(Array<SvgSmilAnim*>& animations, float time)
{
    ++_frameStamp;
    ARRAY_FOREACH(p, animations) _applyOne(*p, time);
}
