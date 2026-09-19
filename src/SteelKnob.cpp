#include "SteelKnob.h"

#include "vstgui/lib/cdrawcontext.h"

#include <algorithm>
#include <cmath>

namespace HighGainGuitarFinisher {

namespace {

constexpr VSTGUI::CColor kShadow {0, 0, 0, 155};
constexpr VSTGUI::CColor kBezelOuter {45, 48, 54, 255};
constexpr VSTGUI::CColor kBezelMid {125, 129, 137, 255};
constexpr VSTGUI::CColor kBezelLight {210, 212, 216, 255};
constexpr VSTGUI::CColor kKnobOuter {24, 27, 31, 255};
constexpr VSTGUI::CColor kKnobInner {42, 45, 51, 255};
constexpr VSTGUI::CColor kKnobHighlight {86, 90, 98, 150};
constexpr VSTGUI::CColor kMarker {242, 243, 245, 255};
constexpr VSTGUI::CColor kTick {107, 111, 119, 210};
constexpr VSTGUI::CColor kAccent {215, 25, 32, 255};
constexpr VSTGUI::CColor kAccentDark {121, 13, 19, 255};

constexpr double kPi =
    3.141592653589793238462643383279502884;

}

SteelKnob::SteelKnob(
    const VSTGUI::CRect& size,
    VSTGUI::IControlListener* listener,
    int32_t tag,
    Style style)
: VSTGUI::CKnob(
      size,
      listener,
      tag,
      nullptr,
      nullptr),
  style_(style) {

    setStartAngle(
        static_cast<float>(
            135.0 / 180.0 * kPi));

    setRangeAngle(
        static_cast<float>(
            270.0 / 180.0 * kPi));

    setWantsFocus(true);
    setTransparency(true);
}

void SteelKnob::draw(
    VSTGUI::CDrawContext* context) {

    const auto r = getViewSize();
    const auto center = r.getCenter();
    const auto minDim =
        std::min(
            r.getWidth(),
            r.getHeight());

    const bool hero =
        style_ == Style::Hero;

    context->setDrawMode(
        VSTGUI::kAntiAliasing);

    const double normalizedValue =
        std::clamp(
            static_cast<double>(
                getValueNormalized()),
            0.0,
            1.0);

    const auto outerRadius =
        minDim * (hero ? 0.435 : 0.420);

    auto shadow = VSTGUI::CRect(
        center.x - outerRadius,
        center.y - outerRadius + (hero ? 7.0 : 5.0),
        center.x + outerRadius,
        center.y + outerRadius + (hero ? 7.0 : 5.0));

    context->setFillColor(kShadow);
    context->drawEllipse(
        shadow,
        VSTGUI::kDrawFilled);

    auto outer = VSTGUI::CRect(
        center.x - outerRadius,
        center.y - outerRadius,
        center.x + outerRadius,
        center.y + outerRadius);

    context->setFillColor(kBezelOuter);
    context->drawEllipse(
        outer,
        VSTGUI::kDrawFilled);

    const auto bezelRadius =
        outerRadius * 0.91;

    auto bezel = VSTGUI::CRect(
        center.x - bezelRadius,
        center.y - bezelRadius,
        center.x + bezelRadius,
        center.y + bezelRadius);

    context->setFillColor(kBezelMid);
    context->drawEllipse(
        bezel,
        VSTGUI::kDrawFilled);

    const auto lightRadius =
        outerRadius * 0.84;

    auto lightRing = VSTGUI::CRect(
        center.x - lightRadius,
        center.y - lightRadius,
        center.x + lightRadius,
        center.y + lightRadius);

    context->setFillColor(kBezelLight);
    context->drawEllipse(
        lightRing,
        VSTGUI::kDrawFilled);

    const auto bodyRadius =
        outerRadius * 0.77;

    auto body = VSTGUI::CRect(
        center.x - bodyRadius,
        center.y - bodyRadius,
        center.x + bodyRadius,
        center.y + bodyRadius);

    context->setFillColor(kKnobOuter);
    context->drawEllipse(
        body,
        VSTGUI::kDrawFilled);

    const auto innerRadius =
        bodyRadius * 0.88;

    auto inner = VSTGUI::CRect(
        center.x - innerRadius,
        center.y - innerRadius,
        center.x + innerRadius,
        center.y + innerRadius);

    context->setFillColor(kKnobInner);
    context->drawEllipse(
        inner,
        VSTGUI::kDrawFilled);

    auto highlight = inner;
    highlight.inset(
        innerRadius * 0.10,
        innerRadius * 0.10);

    highlight.bottom =
        highlight.top +
        highlight.getHeight() * 0.42;

    context->setFillColor(
        kKnobHighlight);

    context->drawEllipse(
        highlight,
        VSTGUI::kDrawFilled);

    const auto arcRadius =
        outerRadius * 1.08;

    auto arc = VSTGUI::CRect(
        center.x - arcRadius,
        center.y - arcRadius,
        center.x + arcRadius,
        center.y + arcRadius);

    context->setFrameColor(
        kAccentDark);

    context->setLineWidth(
        hero ? 7.0 : 4.0);

    context->drawArc(
        arc,
        135.f,
        405.f);

    context->setFrameColor(
        kAccent);

    context->setLineWidth(
        hero ? 7.0 : 4.0);

    context->drawArc(
        arc,
        135.f,
        static_cast<float>(
            135.0 +
            normalizedValue * 270.0));

    const int tickCount =
        hero ? 13 : 9;

    context->setFrameColor(kTick);
    context->setLineWidth(
        hero ? 1.5 : 1.0);

    for (int tick = 0;
         tick < tickCount;
         ++tick) {

        const double normalized =
            tickCount > 1
                ? static_cast<double>(tick) /
                    static_cast<double>(
                        tickCount - 1)
                : 0.0;

        const double angle =
            (135.0 +
             normalized * 270.0) *
            kPi /
            180.0;

        const double innerTick =
            arcRadius * 1.08;

        const double outerTick =
            arcRadius *
            (hero ? 1.17 : 1.14);

        const VSTGUI::CPoint p1(
            center.x +
                std::cos(angle) *
                    innerTick,
            center.y +
                std::sin(angle) *
                    innerTick);

        const VSTGUI::CPoint p2(
            center.x +
                std::cos(angle) *
                    outerTick,
            center.y +
                std::sin(angle) *
                    outerTick);

        context->drawLine(
            p1,
            p2);
    }

    const double angle =
        (135.0 +
         normalizedValue * 270.0) *
        kPi /
        180.0;

    const double markerInner =
        bodyRadius *
        (hero ? 0.18 : 0.22);

    const double markerOuter =
        bodyRadius * 0.70;

    const VSTGUI::CPoint p1(
        center.x +
            std::cos(angle) *
                markerInner,
        center.y +
            std::sin(angle) *
                markerInner);

    const VSTGUI::CPoint p2(
        center.x +
            std::cos(angle) *
                markerOuter,
        center.y +
            std::sin(angle) *
                markerOuter);

    context->setFrameColor(
        kMarker);

    context->setLineWidth(
        hero ? 5.0 : 3.2);

    context->drawLine(
        p1,
        p2);

    const double capRadius =
        bodyRadius * 0.12;

    auto cap = VSTGUI::CRect(
        center.x - capRadius,
        center.y - capRadius,
        center.x + capRadius,
        center.y + capRadius);

    context->setFillColor(
        kBezelMid);

    context->drawEllipse(
        cap,
        VSTGUI::kDrawFilled);

    setDirty(false);
}

} // namespace HighGainGuitarFinisher
