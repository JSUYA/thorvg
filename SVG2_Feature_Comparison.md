# SVG 2 Feature Support Comparison

> ThorVG (main branch) vs librsvg (Cairo) vs resvg (tiny-skia) vs nanosvg vs Skia SVG
>
> - ✅ = Fully Supported
> - ⚠️ = Partially Supported
> - ❌ = Not Supported
> - ➖ = Not Applicable (static renderer limitation / spec removed)

---

## 1. Document Structure

| 중분류 | 소분류 (Element / Feature) | ThorVG | librsvg | resvg | nanosvg | Skia SVG |
|--------|---------------------------|--------|---------|-------|---------|----------|
| **Root** | `<svg>` (width, height, viewBox) | ✅ | ✅ | ✅ | ✅ | ✅ |
| | `preserveAspectRatio` (meet/slice, 9 align) | ✅ | ✅ | ✅ | ❌ | ✅ |
| | Nested `<svg>` | ❌ | ✅ | ✅ | ❌ | ✅ |
| **Grouping** | `<g>` | ✅ | ✅ | ✅ | ✅ | ✅ |
| | `<defs>` | ✅ | ✅ | ✅ | ⚠️ | ✅ |
| | `<symbol>` (viewBox, preserveAspectRatio) | ✅ | ✅ | ✅ | ❌ | ❌ |
| | `<use>` (href, x, y, width, height) | ✅ | ✅ | ✅ | ❌ | ✅ |
| | `<use>` external file reference | ❌ | ❌ | ❌ | ❌ | ❌ |
| **Conditional** | `<switch>` | ❌ | ⚠️ | ⚠️ | ❌ | ❌ |
| | `systemLanguage` | ❌ | ⚠️ | ⚠️ | ❌ | ❌ |
| **Descriptive** | `<title>` | ❌ | ✅ | ✅ | ❌ | ❌ |
| | `<desc>` | ❌ | ✅ | ✅ | ❌ | ❌ |
| | `<metadata>` | ❌ | ✅ | ❌ | ❌ | ❌ |

---

## 2. Styling (CSS Integration)

| 중분류 | 소분류 (Feature) | ThorVG | librsvg | resvg | nanosvg | Skia SVG |
|--------|-----------------|--------|---------|-------|---------|----------|
| **Style 적용** | `style` attribute (inline) | ✅ | ✅ | ✅ | ✅ | ✅ |
| | `<style>` element (embedded CSS) | ⚠️ | ✅ | ⚠️ | ❌ | ❌ |
| | External stylesheet (`<?xml-stylesheet?>`) | ❌ | ✅ | ⚠️ | ❌ | ❌ |
| | Presentation attributes | ✅ | ✅ | ✅ | ✅ | ✅ |
| **CSS Selectors** | Type selector (`rect`, `circle`) | ⚠️ | ✅ | ✅ | ❌ | ❌ |
| | Class selector (`.class`) | ✅ | ✅ | ✅ | ❌ | ❌ |
| | ID selector (`#id`) | ⚠️ | ✅ | ✅ | ❌ | ❌ |
| | Descendant / child combinator | ❌ | ✅ | ⚠️ | ❌ | ❌ |
| | Pseudo-classes (`:hover`, `:nth-child`) | ❌ | ⚠️ | ❌ | ❌ | ❌ |
| | Pseudo-elements (`::first-line`) | ❌ | ❌ | ❌ | ❌ | ❌ |
| | Attribute selectors (`[attr=val]`) | ❌ | ✅ | ⚠️ | ❌ | ❌ |
| **CSS Cascade** | `!important` | ✅ | ✅ | ✅ | ❌ | ❌ |
| | Specificity-based cascade | ⚠️ | ✅ | ✅ | ❌ | ❌ |
| | Inheritance | ✅ | ✅ | ✅ | ⚠️ | ✅ |
| **CSS At-rules** | `@font-face` | ⚠️ | ✅ | ✅ | ❌ | ❌ |
| | `@media` | ❌ | ✅ | ❌ | ❌ | ❌ |
| | `@import` | ❌ | ✅ | ❌ | ❌ | ❌ |

---

## 3. Coordinate Systems & Transformations

| 중분류 | 소분류 (Feature) | ThorVG | librsvg | resvg | nanosvg | Skia SVG |
|--------|-----------------|--------|---------|-------|---------|----------|
| **Viewport** | `viewBox` | ✅ | ✅ | ✅ | ✅ | ✅ |
| | Viewport (new) via `<symbol>`, `<image>` | ✅ | ✅ | ✅ | ❌ | ⚠️ |
| **Transform 함수** | `translate(tx, ty)` | ✅ | ✅ | ✅ | ✅ | ✅ |
| | `scale(sx, sy)` | ✅ | ✅ | ✅ | ✅ | ✅ |
| | `rotate(angle, cx, cy)` | ✅ | ✅ | ✅ | ✅ | ✅ |
| | `skewX(angle)` / `skewY(angle)` | ✅ | ✅ | ✅ | ❌ | ✅ |
| | `matrix(a,b,c,d,e,f)` | ✅ | ✅ | ✅ | ✅ | ✅ |
| **SVG 2 Transform** | `transform-origin` | ❌ | ✅ | ✅ | ❌ | ❌ |
| | `transform-box` | ❌ | ⚠️ | ⚠️ | ❌ | ❌ |
| **Units** | `px, pt, pc, mm, cm, in` | ✅ | ✅ | ✅ | ✅ | ✅ |
| | `em, ex` | ✅ | ✅ | ✅ | ❌ | ⚠️ |
| | `%` (percentage) | ✅ | ✅ | ✅ | ❌ | ✅ |
| **좌표 단위** | `userSpaceOnUse` | ✅ | ✅ | ✅ | ❌ | ✅ |
| | `objectBoundingBox` | ✅ | ✅ | ✅ | ❌ | ✅ |
| **Vector Effects** | `non-scaling-stroke` | ❌ | ✅ | ✅ | ❌ | ❌ |
| | `non-scaling-size` (SVG 2) | ❌ | ❌ | ❌ | ❌ | ❌ |
| | `non-rotation` (SVG 2) | ❌ | ❌ | ❌ | ❌ | ❌ |
| | `fixed-position` (SVG 2) | ❌ | ❌ | ❌ | ❌ | ❌ |

---

## 4. Basic Shapes

| 중분류 | 소분류 (Element / Attribute) | ThorVG | librsvg | resvg | nanosvg | Skia SVG |
|--------|----------------------------|--------|---------|-------|---------|----------|
| **Shapes** | `<rect>` (x, y, width, height, rx, ry) | ✅ | ✅ | ✅ | ✅ | ✅ |
| | `<circle>` (cx, cy, r) | ✅ | ✅ | ✅ | ✅ | ✅ |
| | `<ellipse>` (cx, cy, rx, ry) | ✅ | ✅ | ✅ | ✅ | ✅ |
| | `<line>` (x1, y1, x2, y2) | ✅ | ✅ | ✅ | ✅ | ✅ |
| | `<polyline>` (points) | ✅ | ✅ | ✅ | ✅ | ✅ |
| | `<polygon>` (points) | ✅ | ✅ | ✅ | ✅ | ✅ |
| **SVG 2 Shape** | `rx`/`ry` auto value | ❌ | ✅ | ⚠️ | ❌ | ❌ |
| | `pathLength` on all shapes | ❌ | ✅ | ⚠️ | ❌ | ❌ |
| | Geometry attributes via CSS (`d`, `cx` 등) | ❌ | ❌ | ❌ | ❌ | ❌ |

---

## 5. Paths

| 중분류 | 소분류 (Path Command) | ThorVG | librsvg | resvg | nanosvg | Skia SVG |
|--------|----------------------|--------|---------|-------|---------|----------|
| **기본 명령** | M/m (moveTo) | ✅ | ✅ | ✅ | ✅ | ✅ |
| | L/l (lineTo) | ✅ | ✅ | ✅ | ✅ | ✅ |
| | H/h (horizontal lineTo) | ✅ | ✅ | ✅ | ✅ | ✅ |
| | V/v (vertical lineTo) | ✅ | ✅ | ✅ | ✅ | ✅ |
| | Z/z (closePath) | ✅ | ✅ | ✅ | ✅ | ✅ |
| **곡선 명령** | C/c (cubic bezier) | ✅ | ✅ | ✅ | ✅ | ✅ |
| | S/s (smooth cubic bezier) | ✅ | ✅ | ✅ | ✅ | ✅ |
| | Q/q (quadratic bezier) | ✅ | ✅ | ✅ | ✅ | ✅ |
| | T/t (smooth quadratic bezier) | ✅ | ✅ | ✅ | ✅ | ✅ |
| | A/a (elliptical arc) | ✅ | ✅ | ✅ | ✅ | ✅ |
| **SVG 2** | B/b (bearing, proposed) | ❌ | ❌ | ❌ | ❌ | ❌ |
| | `d` as CSS property | ❌ | ❌ | ❌ | ❌ | ❌ |
| **Attribute** | `pathLength` | ❌ | ✅ | ✅ | ❌ | ❌ |

---

## 6. Text

| 중분류 | 소분류 (Element / Feature) | ThorVG | librsvg | resvg | nanosvg | Skia SVG |
|--------|---------------------------|--------|---------|-------|---------|----------|
| **Text Elements** | `<text>` (x, y) | ✅ | ✅ | ✅ | ❌ | ✅ |
| | `<tspan>` (x, y, dx, dy) | ❌ | ✅ | ✅ | ❌ | ✅ |
| | `<textPath>` (href, startOffset) | ❌ | ❌ | ✅ | ❌ | ⚠️ |
| | `<textPath>` inline path (SVG 2) | ❌ | ❌ | ❌ | ❌ | ❌ |
| | `rotate` attribute on text/tspan | ❌ | ❌ | ⚠️ | ❌ | ❌ |
| | `textLength` / `lengthAdjust` | ❌ | ⚠️ | ⚠️ | ❌ | ❌ |
| **Typography** | `font-family` | ✅ | ✅ | ✅ | ❌ | ✅ |
| | `font-size` (px, pt, em 등) | ✅ | ✅ | ✅ | ❌ | ✅ |
| | `font-weight` | ❌ | ✅ | ✅ | ❌ | ✅ |
| | `font-style` (italic, oblique) | ❌ | ✅ | ✅ | ❌ | ✅ |
| | `font-variant` | ❌ | ⚠️ | ⚠️ | ❌ | ❌ |
| | `font-stretch` | ❌ | ⚠️ | ⚠️ | ❌ | ❌ |
| | `font-feature-settings` (SVG 2) | ❌ | ⚠️ | ❌ | ❌ | ❌ |
| | `font-kerning` (SVG 2) | ❌ | ⚠️ | ❌ | ❌ | ❌ |
| **Text Layout** | `text-anchor` (start/middle/end) | ❌ | ✅ | ✅ | ❌ | ✅ |
| | `dominant-baseline` | ⚠️ | ✅ | ✅ | ❌ | ⚠️ |
| | `alignment-baseline` | ⚠️ | ✅ | ✅ | ❌ | ❌ |
| | `baseline-shift` | ❌ | ✅ | ⚠️ | ❌ | ❌ |
| | `letter-spacing` | ❌ | ✅ | ✅ | ❌ | ❌ |
| | `word-spacing` | ❌ | ✅ | ✅ | ❌ | ❌ |
| | `writing-mode` (horizontal/vertical) | ❌ | ✅ | ✅ | ❌ | ❌ |
| | `direction` (ltr/rtl) | ❌ | ✅ | ❌ | ❌ | ❌ |
| | `unicode-bidi` | ❌ | ✅ | ❌ | ❌ | ❌ |
| **Text Decoration** | `text-decoration` | ❌ | ✅ | ✅ | ❌ | ❌ |
| | `text-decoration-line/style/color` | ❌ | ⚠️ | ⚠️ | ❌ | ❌ |
| **SVG 2 Text** | `inline-size` (auto-wrapping) | ❌ | ❌ | ❌ | ❌ | ❌ |
| | `shape-inside` / `shape-subtract` | ❌ | ❌ | ❌ | ❌ | ❌ |
| | `white-space` (replaces xml:space) | ❌ | ❌ | ⚠️ | ❌ | ❌ |

---

## 7. Embedded Content

| 중분류 | 소분류 (Element / Feature) | ThorVG | librsvg | resvg | nanosvg | Skia SVG |
|--------|---------------------------|--------|---------|-------|---------|----------|
| **Image** | `<image>` (href, x, y, w, h) | ✅ | ✅ | ✅ | ❌ | ✅ |
| | `xlink:href` (deprecated) | ✅ | ✅ | ✅ | ❌ | ❌ |
| | `href` (SVG 2) | ✅ | ✅ | ✅ | ❌ | ✅ |
| | Base64 embedded image | ✅ | ✅ | ✅ | ❌ | ✅ |
| | External file reference | ✅ | ✅ | ✅ | ❌ | ✅ |
| | `preserveAspectRatio` on image | ✅ | ✅ | ✅ | ❌ | ✅ |
| | SVG-in-SVG (recursive) | ❌ | ✅ | ✅ | ❌ | ❌ |
| | `crossorigin` (SVG 2) | ❌ | ❌ | ❌ | ❌ | ❌ |
| **Foreign** | `<foreignObject>` | ❌ | ❌ | ❌ | ❌ | ❌ |
| **HTML in SVG** | `<video>`, `<audio>`, `<canvas>` (SVG 2) | ➖ | ➖ | ➖ | ➖ | ➖ |
| | `<iframe>` (SVG 2) | ➖ | ➖ | ➖ | ➖ | ➖ |

---

## 8. Painting (Fill, Stroke, Markers)

| 중분류 | 소분류 (Property / Feature) | ThorVG | librsvg | resvg | nanosvg | Skia SVG |
|--------|---------------------------|--------|---------|-------|---------|----------|
| **Fill** | `fill` (color, url(), none) | ✅ | ✅ | ✅ | ✅ | ✅ |
| | `fill-opacity` | ✅ | ✅ | ✅ | ✅ | ✅ |
| | `fill-rule` (nonzero, evenodd) | ✅ | ✅ | ✅ | ✅ | ✅ |
| **Stroke** | `stroke` (color, url(), none) | ✅ | ✅ | ✅ | ✅ | ✅ |
| | `stroke-width` | ✅ | ✅ | ✅ | ✅ | ✅ |
| | `stroke-opacity` | ✅ | ✅ | ✅ | ✅ | ✅ |
| | `stroke-linecap` (butt/round/square) | ✅ | ✅ | ✅ | ✅ | ✅ |
| | `stroke-linejoin` (miter/round/bevel) | ✅ | ✅ | ✅ | ✅ | ✅ |
| | `stroke-linejoin: miter-clip` (SVG 2) | ❌ | ❌ | ❌ | ❌ | ❌ |
| | `stroke-linejoin: arcs` (SVG 2) | ❌ | ❌ | ❌ | ❌ | ❌ |
| | `stroke-miterlimit` | ✅ | ✅ | ✅ | ✅ | ✅ |
| | `stroke-dasharray` | ✅ | ✅ | ✅ | ✅ | ✅ |
| | `stroke-dashoffset` | ✅ | ✅ | ✅ | ✅ | ✅ |
| **Paint Order** | `paint-order` (SVG 2) | ✅ | ✅ | ✅ | ❌ | ❌ |
| **Opacity** | `opacity` (element/group) | ✅ | ✅ | ✅ | ✅ | ✅ |
| **SVG 2 Paint** | `context-fill` | ❌ | ⚠️ | ❌ | ❌ | ❌ |
| | `context-stroke` | ❌ | ⚠️ | ❌ | ❌ | ❌ |
| **Markers** | `<marker>` element | ❌ | ✅ | ✅ | ❌ | ❌ |
| | `marker-start` | ❌ | ✅ | ✅ | ❌ | ❌ |
| | `marker-mid` | ❌ | ✅ | ✅ | ❌ | ❌ |
| | `marker-end` | ❌ | ✅ | ✅ | ❌ | ❌ |
| | `orient: auto-start-reverse` (SVG 2) | ❌ | ✅ | ✅ | ❌ | ❌ |
| | Marker on all shapes (SVG 2) | ❌ | ⚠️ | ⚠️ | ❌ | ❌ |
| **Rendering Hints** | `shape-rendering` | ❌ | ✅ | ⚠️ | ❌ | ❌ |
| | `text-rendering` | ❌ | ✅ | ⚠️ | ❌ | ❌ |
| | `image-rendering` | ❌ | ✅ | ✅ | ❌ | ❌ |
| **Visibility** | `display` (none / inline) | ✅ | ✅ | ✅ | ✅ | ✅ |
| | `visibility` | ✅ | ✅ | ✅ | ⚠️ | ✅ |
| | `overflow` | ❌ | ✅ | ✅ | ❌ | ⚠️ |

---

## 9. Color

| 중분류 | 소분류 (Feature) | ThorVG | librsvg | resvg | nanosvg | Skia SVG |
|--------|-----------------|--------|---------|-------|---------|----------|
| **Color Values** | Named colors (CSS) | ✅ | ✅ | ✅ | ✅ | ✅ |
| | `#RGB` / `#RRGGBB` hex | ✅ | ✅ | ✅ | ✅ | ✅ |
| | `rgb(r, g, b)` | ✅ | ✅ | ✅ | ✅ | ✅ |
| | `rgba(r, g, b, a)` | ⚠️ | ✅ | ✅ | ❌ | ❌ |
| | `hsl(h, s, l)` | ✅ | ✅ | ✅ | ❌ | ❌ |
| | `hsla(h, s, l, a)` | ⚠️ | ✅ | ✅ | ❌ | ❌ |
| | `currentColor` | ✅ | ✅ | ✅ | ❌ | ✅ |
| | `transparent` | ⚠️ | ✅ | ✅ | ❌ | ✅ |
| **Color Interpolation** | `color-interpolation` (sRGB/linearRGB) | ❌ | ✅ | ⚠️ | ❌ | ❌ |
| | `color-interpolation-filters` | ❌ | ✅ | ⚠️ | ❌ | ❌ |

---

## 10. Gradients & Patterns

| 중분류 | 소분류 (Element / Attribute) | ThorVG | librsvg | resvg | nanosvg | Skia SVG |
|--------|----------------------------|--------|---------|-------|---------|----------|
| **Linear Gradient** | `<linearGradient>` (x1,y1,x2,y2) | ✅ | ✅ | ✅ | ⚠️ | ✅ |
| | `gradientUnits` | ✅ | ✅ | ✅ | ❌ | ✅ |
| | `gradientTransform` | ✅ | ✅ | ✅ | ⚠️ | ✅ |
| | `spreadMethod` (pad/reflect/repeat) | ✅ | ✅ | ✅ | ❌ | ✅ |
| | `href` (gradient inheritance) | ✅ | ✅ | ✅ | ❌ | ❌ |
| **Radial Gradient** | `<radialGradient>` (cx,cy,r,fx,fy) | ✅ | ✅ | ✅ | ⚠️ | ✅ |
| | `fr` (focal radius, SVG 2) | ✅ | ✅ | ✅ | ❌ | ❌ |
| | `gradientUnits` | ✅ | ✅ | ✅ | ❌ | ✅ |
| | `gradientTransform` | ✅ | ✅ | ✅ | ⚠️ | ✅ |
| | `spreadMethod` (pad/reflect/repeat) | ✅ | ✅ | ✅ | ❌ | ✅ |
| **Gradient Stops** | `<stop>` (offset, stop-color, stop-opacity) | ✅ | ✅ | ✅ | ✅ | ✅ |
| **Pattern** | `<pattern>` (x,y,w,h) | ❌ | ✅ | ✅ | ❌ | ✅ |
| | `patternUnits` / `patternContentUnits` | ❌ | ✅ | ✅ | ❌ | ✅ |
| | `patternTransform` | ❌ | ✅ | ✅ | ❌ | ✅ |
| | `viewBox` on pattern | ❌ | ✅ | ✅ | ❌ | ⚠️ |
| **Deferred (SVG 2)** | Mesh gradients (`<meshgradient>`) | ❌ | ❌ | ❌ | ❌ | ❌ |
| | Hatch patterns (`<hatch>`) | ❌ | ❌ | ❌ | ❌ | ❌ |

---

## 11. Clipping, Masking & Compositing

| 중분류 | 소분류 (Element / Property) | ThorVG | librsvg | resvg | nanosvg | Skia SVG |
|--------|---------------------------|--------|---------|-------|---------|----------|
| **Clipping** | `<clipPath>` element | ✅ | ✅ | ✅ | ❌ | ✅ |
| | `clip-path` property | ✅ | ✅ | ✅ | ❌ | ✅ |
| | `clipPathUnits` | ✅ | ✅ | ✅ | ❌ | ✅ |
| | `clip-rule` (nonzero/evenodd) | ⚠️ | ✅ | ✅ | ❌ | ✅ |
| | CSS basic-shape in clip-path (SVG 2) | ❌ | ❌ | ❌ | ❌ | ❌ |
| **Masking** | `<mask>` element | ✅ | ✅ | ✅ | ❌ | ✅ |
| | `mask` property | ✅ | ✅ | ✅ | ❌ | ✅ |
| | `maskUnits` / `maskContentUnits` | ✅ | ✅ | ✅ | ❌ | ✅ |
| | `mask-type` (luminance/alpha) | ✅ | ✅ | ✅ | ❌ | ⚠️ |
| | CSS mask-image, mask-repeat 등 (CSS Masking 1) | ❌ | ⚠️ | ❌ | ❌ | ❌ |
| **Compositing** | `mix-blend-mode` (16 modes) | ✅ | ✅ | ✅ | ❌ | ⚠️ |
| | `isolation` (auto/isolate) | ❌ | ✅ | ✅ | ❌ | ❌ |

---

## 12. Filter Effects

| 중분류 | 소분류 (Element / Primitive) | ThorVG | librsvg | resvg | nanosvg | Skia SVG |
|--------|----------------------------|--------|---------|-------|---------|----------|
| **Container** | `<filter>` (x,y,w,h, filterUnits) | ⚠️ | ✅ | ✅ | ❌ | ✅ |
| | `primitiveUnits` | ⚠️ | ✅ | ✅ | ❌ | ✅ |
| | Filter chaining (`in`, `result`) | ❌ | ✅ | ✅ | ❌ | ✅ |
| **Blur** | `<feGaussianBlur>` | ✅ | ✅ | ✅ | ❌ | ✅ |
| | `edgeMode` (duplicate/wrap/none) | ❌ | ✅ | ⚠️ | ❌ | ⚠️ |
| **Blending** | `<feBlend>` (16 modes) | ❌ | ✅ | ✅ | ❌ | ✅ |
| **Color** | `<feColorMatrix>` (matrix/saturate/hueRotate/luminanceToAlpha) | ❌ | ✅ | ✅ | ❌ | ✅ |
| | `<feComponentTransfer>` + `<feFuncR/G/B/A>` | ❌ | ✅ | ✅ | ❌ | ❌ |
| **Compositing** | `<feComposite>` (over/in/out/atop/xor/lighter/arithmetic) | ❌ | ✅ | ✅ | ❌ | ✅ |
| **Merge** | `<feMerge>` / `<feMergeNode>` | ❌ | ✅ | ✅ | ❌ | ❌ |
| **Flood** | `<feFlood>` (flood-color, flood-opacity) | ❌ | ✅ | ✅ | ❌ | ✅ |
| **Offset** | `<feOffset>` (dx, dy) | ❌ | ✅ | ✅ | ❌ | ✅ |
| **Morphology** | `<feMorphology>` (erode/dilate) | ❌ | ✅ | ✅ | ❌ | ✅ |
| **Displacement** | `<feDisplacementMap>` | ❌ | ✅ | ✅ | ❌ | ✅ |
| **Convolution** | `<feConvolveMatrix>` | ❌ | ✅ | ✅ | ❌ | ❌ |
| **Tile** | `<feTile>` | ❌ | ✅ | ✅ | ❌ | ❌ |
| **Noise** | `<feTurbulence>` (fractalNoise/turbulence) | ❌ | ✅ | ✅ | ❌ | ✅ |
| **Image** | `<feImage>` (href) | ❌ | ✅ | ✅ | ❌ | ⚠️ |
| **Lighting** | `<feDiffuseLighting>` | ❌ | ✅ | ✅ | ❌ | ✅ |
| | `<feSpecularLighting>` | ❌ | ✅ | ✅ | ❌ | ✅ |
| | `<feDistantLight>` | ❌ | ✅ | ✅ | ❌ | ✅ |
| | `<fePointLight>` | ❌ | ✅ | ⚠️ | ❌ | ✅ |
| | `<feSpotLight>` | ❌ | ✅ | ⚠️ | ❌ | ✅ |
| **Convenience** | `<feDropShadow>` (Filter Effects 1) | ❌ | ✅ | ✅ | ❌ | ❌ |

---

## 13. Interactivity & Scripting

| 중분류 | 소분류 (Feature) | ThorVG | librsvg | resvg | nanosvg | Skia SVG |
|--------|-----------------|--------|---------|-------|---------|----------|
| **Scripting** | `<script>` element | ➖ | ➖ | ➖ | ➖ | ➖ |
| | Event attributes (onclick 등) | ➖ | ➖ | ➖ | ➖ | ➖ |
| **Pointer Events** | `pointer-events` property | ❌ | ✅ | ❌ | ❌ | ❌ |
| **Focus** | `tabindex` (SVG 2) | ➖ | ➖ | ➖ | ➖ | ➖ |
| | `autofocus` (SVG 2) | ➖ | ➖ | ➖ | ➖ | ➖ |
| **Cursor** | `cursor` property | ❌ | ✅ | ❌ | ❌ | ❌ |

> ➖ = Scripting/interactivity는 static rendering library에서 본질적으로 지원 불가

---

## 14. Animation (SMIL)

| 중분류 | 소분류 (Element / Feature) | ThorVG | librsvg | resvg | nanosvg | Skia SVG |
|--------|---------------------------|--------|---------|-------|---------|----------|
| **Animation Elements** | `<animate>` | ❌ | ❌ | ❌ | ❌ | ❌ |
| | `<animateTransform>` | ❌ | ❌ | ❌ | ❌ | ❌ |
| | `<animateMotion>` / `<mpath>` | ❌ | ❌ | ❌ | ❌ | ❌ |
| | `<set>` | ❌ | ❌ | ❌ | ❌ | ❌ |
| | `<discard>` (SVG 2) | ❌ | ❌ | ❌ | ❌ | ❌ |
| **Timing** | begin, end, dur, repeatCount | ❌ | ❌ | ❌ | ❌ | ❌ |
| | keyTimes, keySplines, calcMode | ❌ | ❌ | ❌ | ❌ | ❌ |
| **Value Control** | from, to, by, values | ❌ | ❌ | ❌ | ❌ | ❌ |
| | additive, accumulate | ❌ | ❌ | ❌ | ❌ | ❌ |

> Note: SMIL은 별도 모듈로 분리됨. Browser에서만 지원 (Chrome, Firefox, Safari). Standalone renderer 중 지원하는 것 없음. ThorVG는 Lottie animation을 별도 로더로 지원.

---

## 15. Fonts

| 중분류 | 소분류 (Feature) | ThorVG | librsvg | resvg | nanosvg | Skia SVG |
|--------|-----------------|--------|---------|-------|---------|----------|
| **CSS Font** | `@font-face` (embedded) | ⚠️ | ✅ | ✅ | ❌ | ❌ |
| | WOFF / WOFF2 format | ❌ | ✅ | ✅ | ❌ | ❌ |
| | TTF / OTF format | ⚠️ | ✅ | ✅ | ❌ | ❌ |
| | Base64 embedded font data | ⚠️ | ✅ | ✅ | ❌ | ❌ |
| | System font fallback | ❌ | ✅ | ✅ | ❌ | ⚠️ |
| **SVG 1.1 Fonts** | `<font>`, `<glyph>` 등 (SVG 2에서 제거됨) | ➖ | ➖ | ➖ | ➖ | ➖ |

---

## 16. Linking

| 중분류 | 소분류 (Element / Feature) | ThorVG | librsvg | resvg | nanosvg | Skia SVG |
|--------|---------------------------|--------|---------|-------|---------|----------|
| **Hyperlinks** | `<a>` element (href, target) | ❌ | ✅ | ❌ | ❌ | ❌ |
| | `download`, `ping`, `rel` (SVG 2) | ❌ | ❌ | ❌ | ❌ | ❌ |
| **Views** | `<view>` element | ❌ | ✅ | ⚠️ | ❌ | ❌ |
| | Fragment identifiers (#id) | ⚠️ | ✅ | ✅ | ❌ | ⚠️ |

---

## 17. Metadata & Accessibility

| 중분류 | 소분류 (Feature) | ThorVG | librsvg | resvg | nanosvg | Skia SVG |
|--------|-----------------|--------|---------|-------|---------|----------|
| **Descriptive** | `<title>` | ❌ | ✅ | ✅ | ❌ | ❌ |
| | `<desc>` | ❌ | ✅ | ✅ | ❌ | ❌ |
| | `<metadata>` (RDF/Dublin Core) | ❌ | ✅ | ❌ | ❌ | ❌ |
| **WAI-ARIA** | `role` attribute (SVG 2) | ❌ | ⚠️ | ❌ | ❌ | ❌ |
| | `aria-*` attributes (SVG 2) | ❌ | ⚠️ | ❌ | ❌ | ❌ |
| **Language** | `lang` attribute (SVG 2) | ❌ | ✅ | ⚠️ | ❌ | ❌ |

---

## Summary: Support Count by Category

| Category | ThorVG | librsvg | resvg | nanosvg | Skia SVG |
|----------|--------|---------|-------|---------|----------|
| Document Structure | 7/13 | 12/13 | 11/13 | 3/13 | 7/13 |
| Styling (CSS) | 7/17 | 16/17 | 11/17 | 3/17 | 3/17 |
| Transforms & Coords | 12/17 | 16/17 | 16/17 | 7/17 | 11/17 |
| Basic Shapes | 6/9 | 9/9 | 8/9 | 6/9 | 6/9 |
| Paths | 10/13 | 12/13 | 12/13 | 10/13 | 10/13 |
| Text | 5/32 | 26/32 | 21/32 | 0/32 | 8/32 |
| Embedded Content | 7/11 | 9/11 | 9/11 | 0/11 | 6/11 |
| Painting | 18/29 | 28/29 | 27/29 | 12/29 | 15/29 |
| Color | 6/10 | 10/10 | 9/10 | 3/10 | 5/10 |
| Gradients & Patterns | 10/16 | 16/16 | 16/16 | 3/16 | 12/16 |
| Clipping/Masking/Compositing | 8/12 | 12/12 | 11/12 | 0/12 | 8/12 |
| Filter Effects | 1/22 | 22/22 | 22/22 | 0/22 | 15/22 |
| Interactivity | 0/2 | 2/2 | 0/2 | 0/2 | 0/2 |
| Animation (SMIL) | 0/10 | 0/10 | 0/10 | 0/10 | 0/10 |
| Fonts | 2/5 | 5/5 | 5/5 | 0/5 | 1/5 |
| Linking | 1/4 | 3/4 | 2/4 | 0/4 | 1/4 |
| Metadata/Accessibility | 0/5 | 4/5 | 2/5 | 0/5 | 0/5 |
| **Total** | **100/227** | **202/227** | **182/227** | **47/227** | **108/227** |

> 수치는 ✅ 또는 ⚠️를 지원으로 카운트. ❌ 및 ➖는 미지원으로 카운트.

---

## ThorVG: Priority Implementation Candidates

구현하면 지원 가능한 주요 기능 (난이도 순):

### High Priority (구현 가능성 높음)
| Feature | 난이도 | 비고 |
|---------|--------|------|
| `<tspan>` | 중 | 파싱 코드 존재, 렌더링 로직 추가 필요 |
| `text-anchor` | 하 | 텍스트 위치 계산 추가 |
| `font-weight` / `font-style` | 하 | 폰트 선택 로직 확장 |
| Nested `<svg>` | 중 | 로그에서 미지원 명시, viewport 생성 로직 필요 |
| `<pattern>` | 중~상 | 타일링 렌더링 로직 구현 필요 |
| `<marker>` | 중 | path vertex 계산 + 마커 렌더링 |
| `<feColorMatrix>` | 중 | 색상 행렬 연산 구현 |
| `<feBlend>` | 중 | 기존 blend mode 활용 가능 |
| `<feOffset>` | 하 | 단순 좌표 이동 |
| `<feFlood>` | 하 | 단순 색상 채우기 |
| `<feMerge>` | 중 | 레이어 합성 |
| `<feComposite>` | 중 | Porter-Duff 합성 연산 |
| `transform-origin` | 하~중 | 변환 행렬 계산 수정 |
| `non-scaling-stroke` | 중 | stroke 렌더링 시 변환 보정 |

### Medium Priority (구현 가능하지만 복잡)
| Feature | 난이도 | 비고 |
|---------|--------|------|
| `<textPath>` | 상 | 경로 위의 글자 배치 알고리즘 필요 |
| `letter-spacing` / `word-spacing` | 중 | 글자 간격 계산 |
| `writing-mode` (vertical) | 상 | 세로 쓰기 레이아웃 |
| `text-decoration` | 중 | underline/overline 렌더링 |
| Filter chaining (in/result) | 상 | 필터 그래프 실행 엔진 필요 |
| `<feComponentTransfer>` | 중 | 채널별 전달 함수 |
| `<feTurbulence>` | 상 | Perlin noise 알고리즘 구현 |
| `<feDiffuseLighting>` / `<feSpecularLighting>` | 상 | 조명 모델 구현 |
| CSS selector combinators | 중~상 | CSS 파서 확장 |
| `isolation` property | 중 | 그룹 합성 격리 |

### Low Priority / 지원 불가
| Feature | 사유 |
|---------|------|
| `<script>` / Event handling | Static renderer - 스크립트 엔진 없음 |
| SMIL Animation | Runtime loop 필요, Lottie로 대체 |
| `<foreignObject>` | HTML 렌더링 엔진 필요 |
| `<video>` / `<audio>` / `<canvas>` | Browser 전용 기능 |
| WAI-ARIA | 렌더링과 무관, DOM 접근성 API 필요 |
| Pseudo-classes (`:hover`) | 인터랙션 상태 없음 |
| Mesh gradients / Hatch patterns | SVG 2에서도 deferred |
| `d` as CSS property | CSS Shapes 엔진 필요 |
