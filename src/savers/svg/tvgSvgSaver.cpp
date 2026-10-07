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

#include <cstdarg>
#include "tvgStr.h"
#include "tvgMath.h"
#include "tvgScene.h"
#include "tvgPicture.h"
#include "tvgText.h"
#include "tvgSvgSaver.h"

/************************************************************************/
/* Internal Class Implementation                                        */
/************************************************************************/

static constexpr const char* capTags[] = {"butt", "round", "square"};
static constexpr const char* joinTags[] = {"miter", "round", "bevel"};
static constexpr const char* spreadTags[] = {"pad", "reflect", "repeat"};
static constexpr const char* blendTags[] = {"normal", "multiply", "screen", "overlay", "darken", "lighten", "color-dodge", "color-burn", "hard-light", "soft-light", "difference", "exclusion", "hue", "saturation", "color", "luminosity", "plus-lighter"};

static void _printf(string& out, const char* fmt, ...)
{
    char buf[256];
    va_list args;
    va_start(args, fmt);
    auto len = vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    if (len > 0) out.append(buf, std::min(len, int(sizeof(buf)) - 1));
}

static void _appendColor(string& out, const char* name, uint8_t r, uint8_t g, uint8_t b)
{
    _printf(out, " %s=\"#%02x%02x%02x\"", name, r, g, b);
}

static void _appendOpacity(string& out, const char* name, uint8_t a)
{
    if (a < 255) _printf(out, " %s=\"%g\"", name, (double)(a / 255.0f));
}

static void _appendTransform(string& out, const char* name, const Matrix& m)
{
    if (!tvg::identity(&m)) _printf(out, " %s=\"matrix(%g %g %g %g %g %g)\"", name, (double)m.e11, (double)m.e21, (double)m.e12, (double)m.e22, (double)m.e13, (double)m.e23);
}

static void _appendBlend(string& out, const Paint* paint)
{
    auto method = PAINT(paint)->blendMethod;
    if (method != BlendMethod::Normal && method <= BlendMethod::Add) _printf(out, " style=\"mix-blend-mode:%s\"", blendTags[int(method)]);
}

static void _appendGroup(string& out, const char* attr, char prefix, uint32_t id, const Paint* blend)
{
    _printf(out, "<g %s=\"url(#%c%u)\"", attr, prefix, id);
    if (blend) _appendBlend(out, blend);
    out += ">";
}

static bool _stroking(const Shape* shape)
{
    return to<ShapeImpl>(shape)->rs.strokeWidth() > 0.0f;
}

static void _appendAttrs(string& out, const Paint* paint, const Matrix& m, bool blend)
{
    _appendTransform(out, "transform", m);
    _appendOpacity(out, "opacity", paint->opacity());
    if (blend) _appendBlend(out, paint);
}

static void _appendPath(string& out, const RenderShape& rs)
{
    RenderPath trimmed;
    auto& path = (rs.trimpath() && rs.stroke->trim.trim(rs.path, trimmed)) ? trimmed : rs.path;

    out += " d=\"";
    auto pt = path.pts.data;
    ARRAY_FOREACH(cmd, path.cmds)
    {
        switch (*cmd) {
            case PathCommand::MoveTo: {
                _printf(out, "M%g %g", (double)pt->x, (double)pt->y);
                ++pt;
                break;
            }
            case PathCommand::LineTo: {
                _printf(out, "L%g %g", (double)pt->x, (double)pt->y);
                ++pt;
                break;
            }
            case PathCommand::CubicTo: {
                _printf(out, "C%g %g %g %g %g %g", (double)pt[0].x, (double)pt[0].y, (double)pt[1].x, (double)pt[1].y, (double)pt[2].x, (double)pt[2].y);
                pt += 3;
                break;
            }
            case PathCommand::Close: {
                out += "Z";
                break;
            }
        }
    }
    out += "\"";
}

// the picture origin is not considered in the paint bounds
static void _bounds(const Paint* paint, const Matrix& pm, BBox& box)
{
    Point pt4[4];
    if (paint->type() == Type::Scene) {
        auto m = pm * PAINT(paint)->transform();
        for (auto child : static_cast<const Scene*>(paint)->paints())
            _bounds(child, m, box);
        return;
    } else if (paint->type() == Type::Picture) {
        auto p = to<PictureImpl>(paint);
        auto m = pm * PAINT(paint)->transform();
        translateR(&m, {-p->origin.x * p->w, -p->origin.y * p->h});
        if (!p->bounds(pt4, m, false)) return;
    } else if (!PAINT(paint)->bounds(pt4, &pm, false)) return;

    for (int i = 0; i < 4; ++i) {
        box.min = tvg::min(box.min, pt4[i]);
        box.max = tvg::max(box.max, pt4[i]);
    }
}

static string _region(const Paint* paint)
{
    string out;
    BBox box;
    box.init();
    _bounds(paint, tvg::identity(), box);
    if (box.min.x > box.max.x) return out;

    // enough margin for the strokes and the effects
    auto margin = std::max(box.max.x - box.min.x, box.max.y - box.min.y);
    box.min = box.min - margin;
    box.max = box.max + margin;
    _printf(out, " x=\"%g\" y=\"%g\" width=\"%g\" height=\"%g\"", (double)box.min.x, (double)box.min.y, (double)(box.max.x - box.min.x), (double)(box.max.y - box.min.y));
    return out;
}

static void _appendStroke(string& out, const RenderStroke& stroke)
{
    _printf(out, " stroke-width=\"%g\"", (double)stroke.width);
    if (stroke.cap != StrokeCap::Butt) _printf(out, " stroke-linecap=\"%s\"", capTags[int(stroke.cap)]);
    if (stroke.join != StrokeJoin::Miter) _printf(out, " stroke-linejoin=\"%s\"", joinTags[int(stroke.join)]);
    else if (stroke.miterlimit != 4.0f) _printf(out, " stroke-miterlimit=\"%g\"", (double)stroke.miterlimit);
    if (stroke.dash.count > 0) {
        out += " stroke-dasharray=\"";
        for (uint32_t i = 0; i < stroke.dash.count; ++i)
            _printf(out, "%g ", (double)stroke.dash.pattern[i]);
        out += "\"";
        if (stroke.dash.offset != 0.0f) _printf(out, " stroke-dashoffset=\"%g\"", (double)stroke.dash.offset);
    }
    if (stroke.first) out += " paint-order=\"stroke\"";
}

static float _deviation(float sigma, uint8_t quality)
{
    // the engine approximates the gaussian with N box blur passes (SwGaussianBlur::MAX_LEVEL == 3
    // scaled by quality, see _gaussianInit() in tvgSwPostEffect.cpp). convert the box-blur sigma
    // back to the true gaussian stdDeviation: sigma * sqrt(N / 3).
    // ref: Kovesi, "Fast Almost-Gaussian Filtering" (2010).
    constexpr int MAX_PASSES = 3;
    auto passes = int(MAX_PASSES * ((quality - 1) * 0.01f)) + 1;
    return sigma * sqrtf(passes / float(MAX_PASSES));
}

static void _appendToning(string& out, const char* in, const uint8_t* tones[], int cnt, float weight)
{
    // map rgb to the luminance that Tint/Tritone use. the coefficients are the ITU-R BT.709
    // relative luma (0.2126R + 0.7152G + 0.0722B), matching the engine's luma function.
    _printf(out, "<feColorMatrix in=\"%s\" values=\"0.2126 0.7152 0.0722 0 0 0.2126 0.7152 0.0722 0 0 0.2126 0.7152 0.0722 0 0 0 0 0 1 0\"/>", in);
    out += "<feComponentTransfer>";
    for (int c = 0; c < 3; ++c) {
        _printf(out, "<feFunc%c type=\"table\" tableValues=\"", "RGB"[c]);
        for (int i = 0; i < cnt; ++i)
            _printf(out, "%g ", (double)(tones[i][c] / 255.0f));
        out += "\"/>";
    }
    out += "</feComponentTransfer>";
    _printf(out, "<feComposite in2=\"%s\" operator=\"arithmetic\" k2=\"%g\" k3=\"%g\" result=\"r\"/>", in, (double)weight, (double)(1.0f - weight));
}

static void _appendU32(string& out, uint32_t v)
{
    char buf[4] = {char(v >> 24), char(v >> 16), char(v >> 8), char(v)};
    out.append(buf, 4);
}

// CRC-32 for the png chunks. 0xedb88320 is the reversed IEEE 802.3 polynomial (PNG spec, Annex D).
static uint32_t _crc(const char* data, size_t len, uint32_t crc)
{
    for (size_t i = 0; i < len; ++i) {
        crc ^= uint8_t(data[i]);
        for (int k = 0; k < 8; ++k)
            crc = (crc >> 1) ^ (0xedb88320 & (0 - (crc & 1)));
    }
    return crc;
}

static void _appendChunk(string& png, const char* type, const string& data)
{
    _appendU32(png, data.size());
    png.append(type, 4);
    png += data;
    _appendU32(png, ~_crc(data.data(), data.size(), _crc(type, 4, 0xffffffff)));
}

static void _appendBase64(string& out, const string& in)
{
    static constexpr const char* table = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    auto len = in.size();
    for (size_t i = 0; i < len; i += 3) {
        uint32_t v = uint8_t(in[i]) << 16;
        if (i + 1 < len) v |= uint8_t(in[i + 1]) << 8;
        if (i + 2 < len) v |= uint8_t(in[i + 2]);
        out += table[(v >> 18) & 63];
        out += table[(v >> 12) & 63];
        out += (i + 1 < len) ? table[(v >> 6) & 63] : '=';
        out += (i + 2 < len) ? table[v & 63] : '=';
    }
}

// encode the bitmap as a png embedded via a data uri. the deflate stream uses only stored(raw)
// blocks so no compression library is needed (RFC 1951). larger output, but dependency-free.
static void _appendPng(string& out, const RenderSurface& surface)
{
    auto abgr = (surface.cs == ColorSpace::ABGR8888 || surface.cs == ColorSpace::ABGR8888S);

    string raw;
    raw.reserve(surface.h * (surface.w * 4 + 1));
    for (uint32_t y = 0; y < surface.h; ++y) {
        raw += '\0';
        auto src = surface.buf32 + y * surface.stride;
        for (uint32_t x = 0; x < surface.w; ++x) {
            uint8_t c[4] = {uint8_t(src[x]), uint8_t(src[x] >> 8), uint8_t(src[x] >> 16), uint8_t(src[x] >> 24)};
            if (!abgr) std::swap(c[0], c[2]);
            if (surface.alphaIgnored) c[3] = 255;
            // png stores non-premultiplied colors, so undo the premultiplication: c / alpha.
            else if (surface.premultiplied && c[3] > 0 && c[3] < 255) {
                for (int i = 0; i < 3; ++i)
                    c[i] = std::min(c[i] * 255 / c[3], 255);
            }
            raw.append((char*)c, 4);
        }
    }

    // zlib stream: 2-byte header(0x78 0x01: deflate, 32K window) + deflate data + adler-32 (RFC 1950).
    string zlib = "\x78\x01";
    // adler-32 checksum of the raw data. 65521 is the largest prime below 2^16 (RFC 1950).
    uint32_t a = 1, b = 0;
    for (auto c : raw) {
        a = (a + uint8_t(c)) % 65521;
        b = (b + a) % 65521;
    }
    size_t pos = 0;
    do {
        // stored block: 0xffff is the max length a single deflate block can hold (RFC 1951).
        auto n = std::min(raw.size() - pos, size_t(65535));
        char header[5] = {char(pos + n == raw.size()), char(n), char(n >> 8), char(~n), char(~n >> 8)};
        zlib.append(header, 5);
        zlib.append(raw, pos, n);
        pos += n;
    } while (pos < raw.size());
    _appendU32(zlib, (b << 16) | a);

    string ihdr;
    _appendU32(ihdr, surface.w);
    _appendU32(ihdr, surface.h);
    ihdr.append("\x08\x06\x00\x00\x00", 5);

    string png = "\x89PNG\r\n\x1a\n";
    _appendChunk(png, "IHDR", ihdr);
    _appendChunk(png, "IDAT", zlib);
    _appendChunk(png, "IEND", "");

    _appendBase64(out, png);
}

uint32_t SvgSaver::writeGradient(const Fill* fill)
{
    auto id = ++uid;

    if (fill->type() == Type::LinearGradient) {
        float x1, y1, x2, y2;
        static_cast<const LinearGradient*>(fill)->linear(&x1, &y1, &x2, &y2);
        _printf(defs, "<linearGradient id=\"g%u\" x1=\"%g\" y1=\"%g\" x2=\"%g\" y2=\"%g\"", id, (double)x1, (double)y1, (double)x2, (double)y2);
    } else {
        float cx, cy, r, fx, fy, fr;
        static_cast<const RadialGradient*>(fill)->radial(&cx, &cy, &r, &fx, &fy, &fr);
        _printf(defs, "<radialGradient id=\"g%u\" cx=\"%g\" cy=\"%g\" r=\"%g\" fx=\"%g\" fy=\"%g\" fr=\"%g\"", id, (double)cx, (double)cy, (double)r, (double)fx, (double)fy, (double)fr);
    }
    defs += " gradientUnits=\"userSpaceOnUse\"";
    if (fill->spread() != FillSpread::Pad) _printf(defs, " spreadMethod=\"%s\"", spreadTags[int(fill->spread())]);
    _appendTransform(defs, "gradientTransform", fill->transform());
    defs += ">";

    const Fill::ColorStop* stops;
    auto cnt = fill->colorStops(&stops);
    for (uint32_t i = 0; i < cnt; ++i) {
        _printf(defs, "<stop offset=\"%g\"", (double)stops[i].offset);
        _appendColor(defs, "stop-color", stops[i].r, stops[i].g, stops[i].b);
        _appendOpacity(defs, "stop-opacity", stops[i].a);
        defs += "/>";
    }
    defs += (fill->type() == Type::LinearGradient) ? "</linearGradient>" : "</radialGradient>";

    return id;
}

uint32_t SvgSaver::writeClip(const Shape* clipper)
{
    auto& rs = to<ShapeImpl>(clipper)->rs;
    auto inner = clipper->clip() ? writeClip(clipper->clip()) : 0;
    auto id = ++uid;

    string out;
    // the stroke region of the clipper is the clipping area if available
    if (_stroking(clipper)) {
        _printf(out, "<mask id=\"m%u\" maskUnits=\"userSpaceOnUse\"", id);
        out += _region(clipper) + ">";
        if (inner) _printf(out, "<g clip-path=\"url(#c%u)\">", inner);
        out += "<path";
        _appendTransform(out, "transform", PAINT(clipper)->transform());
        _appendPath(out, rs);
        out += " fill=\"none\" stroke=\"#fff\"";
        _appendStroke(out, *rs.stroke);
        out += (inner ? "/></g></mask>" : "/></mask>");
    } else {
        _printf(out, "<clipPath id=\"c%u\"", id);
        if (inner) _printf(out, " clip-path=\"url(#c%u)\"", inner);
        out += "><path";
        _appendTransform(out, "transform", PAINT(clipper)->transform());
        _appendPath(out, rs);
        if (rs.rule == FillRule::EvenOdd) out += " clip-rule=\"evenodd\"";
        out += "/></clipPath>";
    }

    defs += out;
    return id;
}

uint32_t SvgSaver::writeMask(const Paint* source, const Paint* target, MaskMethod method, bool masking)
{
    auto region = _region(source);
    if (region.empty()) return 0;

    auto id = ++uid;
    string out;
    _printf(out, "<mask id=\"m%u\" maskUnits=\"userSpaceOnUse\"", id);
    out += region;
    if (method == MaskMethod::Alpha) out += " mask-type=\"alpha\"";
    out += ">";
    if (method == MaskMethod::InvAlpha || method == MaskMethod::InvLuma) {
        auto inner = writeMask(source, target, method == MaskMethod::InvAlpha ? MaskMethod::Alpha : MaskMethod::Luma, masking);
        out += "<rect" + region + " fill=\"#fff\"/><rect" + region;
        _printf(out, " mask=\"url(#m%u)\"/>", inner);
    } else {
        writePaint(out, target, masking);
    }
    out += "</mask>";

    defs += out;
    return id;
}

uint32_t SvgSaver::writeEffects(const Array<RenderEffect*>& effects)
{
    auto id = ++uid;
    auto in = "SourceGraphic";

    // the default filter region is -10%~110% of the bbox (SVG 1.1 15.7.5) which clips blur/shadow.
    // the engine does not clip effects, so expand to -100%~200% (x=-1, width=3) to avoid cropping.
    _printf(defs, "<filter id=\"f%u\" x=\"-1\" y=\"-1\" width=\"3\" height=\"3\" color-interpolation-filters=\"sRGB\">", id);
    ARRAY_FOREACH(p, effects)
    {
        switch ((*p)->type) {
            case SceneEffect::GaussianBlur: {
                auto e = static_cast<RenderEffectGaussianBlur*>(*p);
                auto s = _deviation(e->sigma, e->quality);
                _printf(defs, "<feGaussianBlur in=\"%s\" stdDeviation=\"%g %g\"", in, (double)(e->direction == 2 ? 0.0f : s), (double)(e->direction == 1 ? 0.0f : s));
                if (e->border == 1) defs += " edgeMode=\"wrap\"";
                defs += " result=\"r\"/>";
                break;
            }
            case SceneEffect::DropShadow: {
                auto e = static_cast<RenderEffectDropShadow*>(*p);
                auto radian = tvg::deg2rad(90.0f - e->angle);
                _printf(defs, "<feDropShadow in=\"%s\" dx=\"%g\" dy=\"%g\" stdDeviation=\"%g\"", in, (double)(e->distance * cosf(radian)), (double)(-e->distance * sinf(radian)), (double)_deviation(e->sigma, e->quality));
                _appendColor(defs, "flood-color", e->color[0], e->color[1], e->color[2]);
                _appendOpacity(defs, "flood-opacity", e->color[3]);
                defs += " result=\"r\"/>";
                break;
            }
            case SceneEffect::Fill: {
                auto e = static_cast<RenderEffectFill*>(*p);
                defs += "<feFlood";
                _appendColor(defs, "flood-color", e->color[0], e->color[1], e->color[2]);
                _appendOpacity(defs, "flood-opacity", e->color[3]);
                _printf(defs, "/><feComposite in2=\"%s\" operator=\"in\" result=\"r\"/>", in);
                break;
            }
            case SceneEffect::Tint: {
                auto e = static_cast<RenderEffectTint*>(*p);
                const uint8_t* tones[] = {e->black, e->white};
                _appendToning(defs, in, tones, 2, e->intensity / 255.0f);
                break;
            }
            case SceneEffect::Tritone: {
                auto e = static_cast<RenderEffectTritone*>(*p);
                const uint8_t* tones[] = {e->shadow, e->midtone, e->highlight};
                _appendToning(defs, in, tones, 3, 1.0f - e->blender / 255.0f);
                break;
            }
            default: continue;
        }
        in = "r";
    }
    defs += "</filter>";

    return id;
}

void SvgSaver::writeStroke(string& out, const RenderStroke& stroke)
{
    if (stroke.fill) _printf(out, " stroke=\"url(#g%u)\"", writeGradient(stroke.fill));
    else {
        _appendColor(out, "stroke", stroke.color.r, stroke.color.g, stroke.color.b);
        _appendOpacity(out, "stroke-opacity", stroke.color.a);
    }
    _appendStroke(out, stroke);
}

void SvgSaver::writeShape(string& out, const Shape* shape, bool blend)
{
    auto& rs = to<ShapeImpl>(shape)->rs;

    out += "<path";
    _appendAttrs(out, shape, PAINT(shape)->transform(), blend);
    _appendPath(out, rs);
    if (rs.fill) _printf(out, " fill=\"url(#g%u)\"", writeGradient(rs.fill));
    else if (rs.color.a == 0) out += " fill=\"none\"";
    else {
        _appendColor(out, "fill", rs.color.r, rs.color.g, rs.color.b);
        _appendOpacity(out, "fill-opacity", rs.color.a);
    }
    if (rs.rule == FillRule::EvenOdd) out += " fill-rule=\"evenodd\"";
    if (rs.strokeWidth() > 0.0f) writeStroke(out, *rs.stroke);
    out += "/>";
}

void SvgSaver::writeScene(string& out, const Scene* scene, bool blend)
{
    out += "<g";
    _appendAttrs(out, scene, PAINT(scene)->transform(), blend);
    if (auto effects = to<SceneImpl>(scene)->effects) _printf(out, " filter=\"url(#f%u)\"", writeEffects(*effects));
    out += ">";
    for (auto paint : scene->paints())
        writePaint(out, paint);
    out += "</g>";
}

void SvgSaver::writePicture(string& out, const Picture* picture, bool blend)
{
    auto p = to<PictureImpl>(picture);
    p->load();

    auto m = PAINT(picture)->transform();
    translateR(&m, {-p->origin.x * p->w, -p->origin.y * p->h});

    if (p->vector) {
        if (p->resizing) {
            p->loader->resize(p->vector, p->w, p->h);
            p->resizing = false;
        }
        out += "<g";
        _appendAttrs(out, picture, m, blend);
        out += ">";
        writePaint(out, p->vector);
        out += "</g>";
    } else if (p->bitmap && p->bitmap->channelSize == sizeof(uint32_t)) {
        auto scale = std::min(p->w / p->loader->w, p->h / p->loader->h);
        scaleR(&m, {scale, scale});
        out += "<image";
        _appendAttrs(out, picture, m, blend);
        _printf(out, " width=\"%g\" height=\"%g\" preserveAspectRatio=\"none\"", (double)p->loader->w, (double)p->loader->h);
        if (p->filter == FilterMethod::Nearest) out += " image-rendering=\"pixelated\"";
        out += " href=\"data:image/png;base64,";
        _appendPng(out, *p->bitmap);
        out += "\"/>";
    }
}

void SvgSaver::writeText(string& out, const Text* text, bool blend)
{
    auto t = to<TextImpl>(text);
    if (!t->load()) return;

    if (t->outlineWidth > 0.0f) t->shape->strokeWidth(t->outlineWidth * t->fm.scale);

    out += "<g";
    _appendAttrs(out, text, PAINT(text)->transform(), blend);
    out += ">";
    writePaint(out, t->shape);
    out += "</g>";
}

void SvgSaver::writeNode(string& out, const Paint* paint, uint32_t mask)
{
    auto clipper = paint->clip();
    auto wrap = 0;

    // mask and clip are defined in the parent space of the paint
    if (clipper) {
        auto id = writeClip(clipper);
        if (_stroking(clipper)) _appendGroup(out, "mask", 'm', id, paint);
        else _appendGroup(out, "clip-path", 'c', id, paint);
        ++wrap;
    }
    if (mask) _appendGroup(out, "mask", 'm', mask, wrap++ ? nullptr : paint);

    switch (paint->type()) {
        case Type::Shape: {
            writeShape(out, static_cast<const Shape*>(paint), !wrap);
            break;
        }
        case Type::Scene: {
            writeScene(out, static_cast<const Scene*>(paint), !wrap);
            break;
        }
        case Type::Picture: {
            writePicture(out, static_cast<const Picture*>(paint), !wrap);
            break;
        }
        case Type::Text: {
            writeText(out, static_cast<const Text*>(paint), !wrap);
            break;
        }
        default: break;
    }

    while (wrap--)
        out += "</g>";
}

void SvgSaver::writePaint(string& out, const Paint* paint, bool masking)
{
    if (!paint->visible()) return;

    const Paint* target = nullptr;
    auto method = masking ? paint->mask(&target) : MaskMethod::None;

    // lighten and darken are approximated by the union and the intersection
    switch (method) {
        case MaskMethod::Add:
        case MaskMethod::Lighten: {
            out += "<g>";
            writePaint(out, paint, false);
            writePaint(out, target);
            out += "</g>";
            return;
        }
        case MaskMethod::Difference: {
            out += "<g>";
            writeNode(out, paint, writeMask(paint, target, MaskMethod::InvAlpha));
            _printf(out, "<g mask=\"url(#m%u)\">", writeMask(target, paint, MaskMethod::InvAlpha, false));
            writePaint(out, target);
            out += "</g></g>";
            return;
        }
        case MaskMethod::Intersect:
        case MaskMethod::Darken: {
            method = MaskMethod::Alpha;
            break;
        }
        case MaskMethod::Subtract: {
            method = MaskMethod::InvAlpha;
            break;
        }
        default: break;
    }

    writeNode(out, paint, (method == MaskMethod::None) ? 0 : writeMask(paint, target, method));
}

void SvgSaver::run(TVG_UNUSED unsigned tid)
{
    string body;
    if (bg) writePaint(body, bg);
    writePaint(body, paint);

    auto f = fopen(path, "w");
    if (!f) {
        TVGERR("SVG_SAVER", "Failed to open %s", path);
        return;
    }
    fprintf(f, "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"%g\" height=\"%g\" viewBox=\"0 0 %g %g\">", (double)vsize[0], (double)vsize[1], (double)vsize[0], (double)vsize[1]);
    if (!defs.empty()) fprintf(f, "<defs>%s</defs>", defs.c_str());
    fwrite(body.data(), 1, body.size(), f);
    fputs("</svg>\n", f);
    fclose(f);
}

/************************************************************************/
/* External Class Implementation                                        */
/************************************************************************/

SvgSaver::~SvgSaver()
{
    close();
}

bool SvgSaver::close()
{
    this->done();

    if (bg) bg->unref();
    bg = nullptr;

    if (paint) paint->unref();
    paint = nullptr;

    tvg::free(path);
    path = nullptr;

    defs.clear();
    uid = 0;

    return true;
}

bool SvgSaver::save(Paint* paint, Paint* bg, const char* filename, TVG_UNUSED uint32_t quality)
{
    close();

    BBox box;
    box.init();
    _bounds(paint, PAINT(paint)->ptransform(), box);

    // keep the origin, cut off the negative space
    vsize[0] = box.max.x;
    vsize[1] = box.max.y;

    if (vsize[0] < FLOAT_EPSILON || vsize[1] < FLOAT_EPSILON) {
        TVGLOG("SVG_SAVER", "Saving paint(%p) has zero view size.", paint);
        return false;
    }

    if (!filename) return false;
    this->path = duplicate(filename);

    paint->ref();
    this->paint = paint;

    if (bg) {
        bg->ref();
        this->bg = bg;
    }

    TaskScheduler::request(this);

    return true;
}

bool SvgSaver::save(TVG_UNUSED Animation* animation, TVG_UNUSED Paint* bg, TVG_UNUSED const char* filename, TVG_UNUSED uint32_t quality, TVG_UNUSED uint32_t fps)
{
    TVGLOG("SVG_SAVER", "Animation is not supported.");
    return false;
}
