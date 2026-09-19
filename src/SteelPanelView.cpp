#include "SteelPanelView.h"

#include "vstgui/lib/cdrawcontext.h"

#include <array>

namespace HighGainGuitarFinisher {

namespace {

constexpr VSTGUI::CColor kBase {15, 17, 20, 255};
constexpr VSTGUI::CColor kBaseLineA {20, 22, 26, 255};
constexpr VSTGUI::CColor kBaseLineB {11, 13, 16, 255};
constexpr VSTGUI::CColor kOuterFrame {66, 69, 75, 255};
constexpr VSTGUI::CColor kInnerFrame {31, 34, 39, 255};
constexpr VSTGUI::CColor kPlate {24, 27, 31, 255};
constexpr VSTGUI::CColor kPlateInner {18, 20, 24, 255};
constexpr VSTGUI::CColor kPlateTop {84, 87, 94, 150};
constexpr VSTGUI::CColor kPlateBottom {0, 0, 0, 145};
constexpr VSTGUI::CColor kAccent {215, 25, 32, 255};
constexpr VSTGUI::CColor kAccentSoft {215, 25, 32, 70};
constexpr VSTGUI::CColor kScrewOuter {8, 9, 11, 255};
constexpr VSTGUI::CColor kScrew {119, 123, 130, 255};
constexpr VSTGUI::CColor kScrewLight {204, 207, 212, 180};
constexpr VSTGUI::CColor kScrewSlot {45, 48, 53, 255};

void drawScrew(
    VSTGUI::CDrawContext* context,
    double x,
    double y) {

    auto shadow =
        VSTGUI::CRect(
            x - 6.0,
            y - 5.0,
            x + 6.0,
            y + 7.0);

    context->setFillColor(
        kScrewOuter);

    context->drawEllipse(
        shadow,
        VSTGUI::kDrawFilled);

    auto body =
        VSTGUI::CRect(
            x - 5.0,
            y - 5.0,
            x + 5.0,
            y + 5.0);

    context->setFillColor(kScrew);
    context->drawEllipse(
        body,
        VSTGUI::kDrawFilled);

    auto shine = body;
    shine.inset(1.8, 1.8);
    shine.bottom =
        shine.top +
        shine.getHeight() * 0.45;

    context->setFillColor(
        kScrewLight);

    context->drawEllipse(
        shine,
        VSTGUI::kDrawFilled);

    context->setFrameColor(
        kScrewSlot);

    context->setLineWidth(1.2);

    context->drawLine(
        VSTGUI::CPoint(
            x - 2.4,
            y + 2.4),
        VSTGUI::CPoint(
            x + 2.4,
            y - 2.4));
}

void drawPlate(
    VSTGUI::CDrawContext* context,
    const VSTGUI::CRect& rect) {

    context->setFillColor(
        kPlateBottom);

    auto shadow = rect;
    shadow.offset(0.0, 4.0);

    context->drawRect(
        shadow,
        VSTGUI::kDrawFilled);

    context->setFillColor(
        kPlate);

    context->setFrameColor(
        kOuterFrame);

    context->setLineWidth(1.0);

    context->drawRect(
        rect,
        VSTGUI::kDrawFilledAndStroked);

    auto inner = rect;
    inner.inset(5.0, 5.0);

    context->setFillColor(
        kPlateInner);

    context->setFrameColor(
        kInnerFrame);

    context->drawRect(
        inner,
        VSTGUI::kDrawFilledAndStroked);

    context->setFrameColor(
        kPlateTop);

    context->drawLine(
        VSTGUI::CPoint(
            rect.left + 1.0,
            rect.top + 1.0),
        VSTGUI::CPoint(
            rect.right - 1.0,
            rect.top + 1.0));
}

}

SteelPanelView::SteelPanelView(
    const VSTGUI::CRect& size)
: VSTGUI::CView(size) {

    setMouseEnabled(false);
    setTransparency(false);
}

void SteelPanelView::draw(
    VSTGUI::CDrawContext* context) {

    const auto r =
        getViewSize();

    context->setDrawMode(
        VSTGUI::kAntiAliasing);

    context->setFillColor(kBase);
    context->drawRect(
        r,
        VSTGUI::kDrawFilled);

    for (double y =
             r.top + 1.0;
         y <
             r.bottom;
         y += 3.0) {

        context->setFrameColor(
            static_cast<int>(y) % 2 == 0
                ? kBaseLineA
                : kBaseLineB);

        context->setLineWidth(1.0);

        context->drawLine(
            VSTGUI::CPoint(
                r.left,
                y),
            VSTGUI::CPoint(
                r.right,
                y));
    }

    auto outer = r;
    outer.inset(
        8.0,
        8.0);

    context->setFrameColor(
        kOuterFrame);

    context->setLineWidth(
        1.0);

    context->drawRect(
        outer,
        VSTGUI::kDrawStroked);

    auto inner = outer;
    inner.inset(
        7.0,
        7.0);

    context->setFrameColor(
        kInnerFrame);

    context->drawRect(
        inner,
        VSTGUI::kDrawStroked);

    auto header =
        VSTGUI::CRect(
            25.0,
            22.0,
            735.0,
            94.0);

    drawPlate(
        context,
        header);

    drawPlate(
        context,
        VSTGUI::CRect(
            34.0,
            112.0,
            184.0,
            395.0));

    drawPlate(
        context,
        VSTGUI::CRect(
            203.0,
            112.0,
            458.0,
            395.0));

    drawPlate(
        context,
        VSTGUI::CRect(
            477.0,
            112.0,
            726.0,
            395.0));

    context->setFillColor(
        kAccentSoft);

    context->drawRect(
        VSTGUI::CRect(
            217.0,
            126.0,
            444.0,
            130.0),
        VSTGUI::kDrawFilled);

    context->setFillColor(
        kAccent);

    context->drawRect(
        VSTGUI::CRect(
            217.0,
            126.0,
            331.0,
            130.0),
        VSTGUI::kDrawFilled);

    const std::array<double, 5>
        roomLines {
            0.0,
            12.0,
            24.0,
            36.0,
            48.0
        };

    for (std::size_t i = 0;
         i < roomLines.size();
         ++i) {

        const double inset =
            static_cast<double>(i) *
            8.0;

        context->setFrameColor(
            i == 0
                ? kAccentSoft
                : kInnerFrame);

        context->setLineWidth(1.0);

        context->drawLine(
            VSTGUI::CPoint(
                507.0 + inset,
                300.0 +
                    roomLines[i] * 0.30),
            VSTGUI::CPoint(
                696.0 - inset,
                300.0 +
                    roomLines[i] * 0.30));
    }

    drawScrew(
        context,
        20.0,
        20.0);

    drawScrew(
        context,
        740.0,
        20.0);

    drawScrew(
        context,
        20.0,
        410.0);

    drawScrew(
        context,
        740.0,
        410.0);

    setDirty(false);
}

} // namespace HighGainGuitarFinisher
