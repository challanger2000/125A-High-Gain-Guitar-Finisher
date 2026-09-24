#include "SteelPanelView.h"

#include "vstgui/lib/cdrawcontext.h"
#include "vstgui/lib/cgradient.h"
#include "vstgui/lib/cgraphicspath.h"

#include <algorithm>
#include <array>
#include <cstdint>

namespace HighGainGuitarFinisher {
namespace {

void fillRoundGradient(
    VSTGUI::CDrawContext* context,
    const VSTGUI::CRect& rect,
    double radius,
    const VSTGUI::CColor& top,
    const VSTGUI::CColor& bottom) {

    auto* path =
        context->createRoundRectGraphicsPath(
            rect,
            radius);

    if (!path)
        return;

    auto* gradient =
        VSTGUI::CGradient::create(
            0.0,
            1.0,
            top,
            bottom);

    if (gradient) {
        context->fillLinearGradient(
            path,
            *gradient,
            {rect.left, rect.top},
            {rect.left, rect.bottom});

        gradient->forget();
    }

    path->forget();
}

void strokeRound(
    VSTGUI::CDrawContext* context,
    const VSTGUI::CRect& rect,
    double radius,
    const VSTGUI::CColor& color,
    double width) {

    auto* path =
        context->createRoundRectGraphicsPath(
            rect,
            radius);

    if (!path)
        return;

    context->setFrameColor(color);
    context->setLineWidth(width);

    context->drawGraphicsPath(
        path,
        VSTGUI::CDrawContext::kPathStroked);

    path->forget();
}

void fillRadialEllipse(
    VSTGUI::CDrawContext* context,
    const VSTGUI::CRect& rect,
    const VSTGUI::CColor& inner,
    const VSTGUI::CColor& outer,
    const VSTGUI::CPoint& offset) {

    auto* path =
        context->createGraphicsPath();

    if (!path)
        return;

    path->addEllipse(rect);

    auto* gradient =
        VSTGUI::CGradient::create(
            0.0,
            1.0,
            inner,
            outer);

    if (gradient) {
        context->fillRadialGradient(
            path,
            *gradient,
            rect.getCenter(),
            std::max(
                rect.getWidth(),
                rect.getHeight()) * 0.52,
            offset);

        gradient->forget();
    }

    path->forget();
}

} // namespace

SteelPanelView::SteelPanelView(
    const VSTGUI::CRect& size)
: VSTGUI::CView(size) {
    setMouseEnabled(false);
}

SteelPanelView::SteelPanelView(
    const SteelPanelView& other)
: VSTGUI::CView(other) {
}

void SteelPanelView::draw(
    VSTGUI::CDrawContext* context) {

    const auto r = getViewSize();
    const double ox = r.left;
    const double oy = r.top;

    const auto rect =
        [&](double x,
            double y,
            double w,
            double h) {
            return VSTGUI::CRect(
                ox + x,
                oy + y,
                ox + x + w,
                oy + y + h);
        };

    const auto line =
        [&](double x1,
            double y1,
            double x2,
            double y2,
            const VSTGUI::CColor& color,
            double width = 1.0) {

            context->setFrameColor(color);
            context->setLineWidth(width);
            context->drawLine(
                {ox + x1, oy + y1},
                {ox + x2, oy + y2});
        };

    const auto raised =
        [&](double x,
            double y,
            double w,
            double h,
            double radius,
            bool accent) {

            const auto shadow =
                rect(
                    x + 1.0,
                    y + 4.0,
                    w,
                    h);

            fillRoundGradient(
                context,
                shadow,
                radius,
                {0, 0, 0, 135},
                {0, 0, 0, 225});

            const auto rr =
                rect(x, y, w, h);

            fillRoundGradient(
                context,
                rr,
                radius,
                accent
                    ? VSTGUI::CColor{34, 35, 48, 255}
                    : VSTGUI::CColor{36, 40, 47, 255},
                {13, 15, 20, 255});

            strokeRound(
                context,
                rr,
                radius,
                accent
                    ? VSTGUI::CColor{118, 104, 255, 82}
                    : VSTGUI::CColor{88, 95, 106, 126},
                1.0);

            auto inner = rr;
            inner.inset(2.0, 2.0);

            strokeRound(
                context,
                inner,
                std::max(
                    2.0,
                    radius - 2.0),
                {255, 255, 255, 16},
                1.0);

            line(
                x + 10.0,
                y + 9.0,
                x + w - 10.0,
                y + 9.0,
                accent
                    ? VSTGUI::CColor{142, 128, 255, 52}
                    : VSTGUI::CColor{220, 224, 232, 24},
                accent ? 1.5 : 1.0);
        };

    const auto well =
        [&](double x,
            double y,
            double w,
            double h,
            double radius) {

            const auto shadow =
                rect(
                    x,
                    y + 2.0,
                    w,
                    h);

            fillRoundGradient(
                context,
                shadow,
                radius,
                {0, 0, 0, 185},
                {0, 0, 0, 245});

            const auto rr =
                rect(x, y, w, h);

            fillRoundGradient(
                context,
                rr,
                radius,
                {7, 9, 13, 255},
                {17, 20, 26, 255});

            strokeRound(
                context,
                rr,
                radius,
                {70, 77, 88, 145},
                1.0);

            line(
                x + radius,
                y + 1.0,
                x + w - radius,
                y + 1.0,
                {0, 0, 0, 205},
                1.0);

            // Very soft lower reflection makes the recess read as dark glass
            // instead of another flat painted rectangle.
            line(
                x + radius + 2.0,
                y + h - 2.0,
                x + w - radius - 2.0,
                y + h - 2.0,
                {210, 218, 232, 16},
                1.0);
        };

    const auto screw =
        [&](double x,
            double y) {

            const auto sr =
                rect(
                    x - 3.5,
                    y - 3.5,
                    7.0,
                    7.0);

            fillRadialEllipse(
                context,
                sr,
                {112, 118, 128, 255},
                {16, 18, 22, 255},
                {-1.5, -1.5});

            context->setFrameColor(
                {3, 4, 6, 255});

            context->setLineWidth(1.0);

            context->drawEllipse(
                sr,
                VSTGUI::kDrawStroked);

            line(
                x - 1.6,
                y,
                x + 1.6,
                y,
                {4, 5, 7, 235},
                1.0);
        };

    context->setDrawMode(
        VSTGUI::kAntiAliasing);

    // Outer chassis.
    context->setFillColor(
        {5, 7, 10, 255});

    context->drawRect(
        r,
        VSTGUI::kDrawFilled);

    auto chassis = r;
    chassis.inset(
        9.0,
        9.0);

    fillRoundGradient(
        context,
        chassis,
        16.0,
        {43, 47, 55, 255},
        {11, 13, 18, 255});

    strokeRound(
        context,
        chassis,
        16.0,
        {1, 2, 4, 255},
        2.0);

    auto inner = chassis;
    inner.inset(
        4.0,
        4.0);

    strokeRound(
        context,
        inner,
        13.0,
        {185, 191, 201, 33},
        1.0);

    // Restrained brushed-anodised texture.
    for (int y = 17;
         y < 602;
         y += 4) {

        const auto alpha =
            static_cast<uint8_t>(
                (y % 16 == 1)
                    ? 10
                    : 4);

        line(
            14.0,
            static_cast<double>(y),
            1106.0,
            static_cast<double>(y),
            {196, 201, 210, alpha},
            1.0);
    }

    line(
        22,
        16,
        1098,
        16,
        {235, 238, 244, 34},
        1.0);

    line(
        22,
        602,
        1098,
        602,
        {0, 0, 0, 210},
        1.0);

    // Header / navigation bridge.
    raised(
        28,
        22,
        1064,
        80,
        12.0,
        false);

    raised(
        46,
        112,
        1028,
        158,
        12.0,
        false);

    // Main hardware bays.
    raised(
        54,
        296,
        230,
        238,
        12.0,
        false);

    raised(
        304,
        296,
        300,
        238,
        12.0,
        true);

    raised(
        624,
        296,
        270,
        238,
        12.0,
        false);

    raised(
        914,
        296,
        152,
        238,
        12.0,
        false);

    // Real recessed value/status wells.
    well(
        727,
        194,
        343,
        67,
        11.0);

    well(
        113,
        486,
        112,
        30,
        8.0);

    well(
        403,
        506,
        102,
        30,
        9.0);

    well(
        654,
        485,
        96,
        28,
        8.0);

    well(
        768,
        485,
        96,
        28,
        8.0);

    well(
        942,
        486,
        96,
        30,
        8.0);

    // Accent datum lines: enough identity without neon.
    line(
        48,
        106,
        1072,
        106,
        {118, 104, 255, 78},
        1.3);

    line(
        307,
        300,
        601,
        300,
        {134, 120, 255, 110},
        1.5);

    line(
        54,
        550,
        1066,
        550,
        {144, 150, 160, 38},
        1.0);

    // Hardware fastening follows one consistent mechanical rule:
    // chassis corners plus four fasteners on every lower module.
    const std::array<VSTGUI::CPoint, 20> screws {{
        {22, 22},
        {1098, 22},
        {22, 598},
        {1098, 598},

        {66, 308},
        {272, 308},
        {66, 522},
        {272, 522},

        {316, 308},
        {592, 308},
        {316, 522},
        {592, 522},

        {636, 308},
        {882, 308},
        {636, 522},
        {882, 522},

        {926, 308},
        {1054, 308},
        {926, 522},
        {1054, 522}
    }};

    for (const auto& p : screws)
        screw(p.x, p.y);

    setDirty(false);
}

} // namespace HighGainGuitarFinisher
