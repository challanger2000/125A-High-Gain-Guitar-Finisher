#include "HighGainGuitarFinisherController.h"
#include "HighGainGuitarFinisherIDs.h"
#include "BrandLogoView.h"
#include "SteelKnob.h"
#include "SteelPanelView.h"
#include "dsp/LowCutMapping.h"

#include "base/source/fstreamer.h"
#include "pluginterfaces/base/ustring.h"
#include "public.sdk/source/vst/vstparameters.h"
#include "vstgui/lib/controls/ccontrol.h"
#include "vstgui/plugin-bindings/vst3editor.h"
#include "vstgui/uidescription/uiattributes.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

namespace HighGainGuitarFinisher {

using namespace Steinberg;
using namespace Steinberg::Vst;

namespace {

constexpr int32 kGuiZoomTag =
    9000;

void copyAscii(
    const std::string& text,
    String128 destination) {

    std::size_t i = 0;

    for (;
         i < text.size() &&
         i < 127;
         ++i) {

        destination[i] =
            static_cast<TChar>(
                static_cast<
                    unsigned char>(
                        text[i]));
    }

    destination[i] = 0;
}

std::string percentText(
    ParamValue value) {

    char buffer[32] {};

    std::snprintf(
        buffer,
        sizeof(buffer),
        "%.0f %%",
        std::clamp(
            value,
            0.0,
            1.0) *
            100.0);

    return buffer;
}

std::string outputText(
    ParamValue value) {

    const double dB =
        std::clamp(
            value,
            0.0,
            1.0) *
            24.0 -
        12.0;

    char buffer[32] {};

    std::snprintf(
        buffer,
        sizeof(buffer),
        "%+.1f dB",
        dB);

    return buffer;
}

class LowCutParameter final :
    public Parameter {
public:
    LowCutParameter()
    : Parameter(
        STR16("Low Cut"),
        kLowCut80,
        STR16("Hz"),
        0.0,
        kStepCountContinuous,
        ParameterInfo::kCanAutomate) {
    }

    void toString(
        ParamValue normalizedValue,
        String128 string) const
        SMTG_OVERRIDE {

        UString128 result;

        if (!dsp::lowCutEnabled(
                normalizedValue)) {

            result.fromAscii(
                "Off");
        } else {
            result.printFloat(
                dsp::
                    lowCutFrequencyFromNormalized(
                        normalizedValue),
                0);
        }

        result.copyTo(
            string,
            128);
    }

    bool fromString(
        const TChar* string,
        ParamValue& normalizedResult) const
        SMTG_OVERRIDE {

        if (!string)
            return false;

        UString value(
            const_cast<TChar*>(
                string),
            strlen16(string));

        ParamValue frequency =
            0.0;

        if (!value.scanFloat(
                frequency)) {
            return false;
        }

        normalizedResult =
            dsp::
                lowCutNormalizedFromFrequency(
                    frequency);

        return true;
    }

    ParamValue toPlain(
        ParamValue normalizedValue) const
        SMTG_OVERRIDE {

        return dsp::lowCutEnabled(
                   normalizedValue)
            ? dsp::
                lowCutFrequencyFromNormalized(
                    normalizedValue)
            : 0.0;
    }

    ParamValue toNormalized(
        ParamValue plainValue) const
        SMTG_OVERRIDE {

        return dsp::
            lowCutNormalizedFromFrequency(
                plainValue);
    }

    OBJ_METHODS(
        LowCutParameter,
        Parameter)
};

}

tresult PLUGIN_API Controller::initialize(
    FUnknown* context) {

    const auto result =
        EditController::initialize(
            context);

    if (result !=
        kResultOk) {
        return result;
    }

    constexpr int32 automate =
        ParameterInfo::kCanAutomate;

    parameters.addParameter(
        STR16("Finish"),
        STR16("%"),
        0,
        0.0,
        automate,
        kFinish);

    parameters.addParameter(
        STR16("Wet"),
        STR16("%"),
        0,
        0.0,
        automate,
        kRoom);

    parameters.addParameter(
        STR16("Decay"),
        STR16("%"),
        0,
        0.5,
        automate,
        kRoomDecay);

    parameters.addParameter(
        new RangeParameter(
            STR16("Output"),
            kOutput,
            STR16("dB"),
            -12.0,
            12.0,
            0.0));

    parameters.addParameter(
        STR16("Bypass"),
        STR16(""),
        1,
        0.0,
        ParameterInfo::kCanAutomate |
            ParameterInfo::kIsBypass,
        kBypass);

    parameters.addParameter(
        new LowCutParameter());

    return kResultOk;
}

tresult PLUGIN_API Controller::setState(
    IBStream* state) {

    if (!state)
        return kInvalidArgument;

    IBStreamer stream(
        state,
        kLittleEndian);

    double zoom = 1.0;

    if (!stream.readDouble(
            zoom)) {
        return kResultFalse;
    }

    guiZoom_ =
        zoom >= 1.25
            ? 1.5
            : 1.0;

    if (editor_)
        editor_->setZoomFactor(
            guiZoom_);

    return kResultOk;
}

tresult PLUGIN_API Controller::getState(
    IBStream* state) {

    if (!state)
        return kInvalidArgument;

    IBStreamer stream(
        state,
        kLittleEndian);

    return stream.writeDouble(
               guiZoom_)
        ? kResultOk
        : kResultFalse;
}

IPlugView* PLUGIN_API Controller::createView(
    FIDString name) {

    if (!name ||
        std::strcmp(
            name,
            ViewType::kEditor) != 0) {
        return nullptr;
    }

    auto* editor =
        new VSTGUI::VST3Editor(
            this,
            "view",
            "HighGainGuitarFinisher.uidesc");

    editor->setAllowedZoomFactors(
        std::vector<double> {
            1.0,
            1.5
        });

    editor->
        setEditorSizeConstrains(
            VSTGUI::CPoint(
                760.0,
                430.0),
            VSTGUI::CPoint(
                760.0,
                430.0));

    editor_ =
        editor;

    editor_->setZoomFactor(
        guiZoom_);

    return editor;
}

VSTGUI::CView*
Controller::createCustomView(
    VSTGUI::UTF8StringPtr name,
    const VSTGUI::UIAttributes&
        attributes,
    const VSTGUI::IUIDescription*,
    VSTGUI::VST3Editor* editor) {

    if (!name ||
        !editor) {
        return nullptr;
    }

    VSTGUI::CPoint origin {
        0.0,
        0.0
    };

    VSTGUI::CPoint size {
        80.0,
        80.0
    };

    attributes.getPointAttribute(
        "origin",
        origin);

    attributes.getPointAttribute(
        "size",
        size);

    const VSTGUI::CRect rect(
        origin.x,
        origin.y,
        origin.x + size.x,
        origin.y + size.y);

    if (std::strcmp(
            name,
            "HGGFPanel") == 0) {

        return new SteelPanelView(
            rect);
    }

    if (std::strcmp(
            name,
            "HGGFBrandLogo") == 0) {

        return new BrandLogoView(
            rect);
    }

    int32 tag = -1;
    auto style =
        SteelKnob::Style::Small;

    if (std::strcmp(
            name,
            "HGGFKnobFinish") == 0) {

        tag = kFinish;
        style =
            SteelKnob::Style::Hero;
    } else if (std::strcmp(
                   name,
                   "HGGFKnobLowCut") == 0) {

        tag = kLowCut80;
    } else if (std::strcmp(
                   name,
                   "HGGFKnobWet") == 0) {

        tag = kRoom;
    } else if (std::strcmp(
                   name,
                   "HGGFKnobDecay") == 0) {

        tag = kRoomDecay;
    } else if (std::strcmp(
                   name,
                   "HGGFKnobOutput") == 0) {

        tag = kOutput;
    } else {
        return nullptr;
    }

    return new SteelKnob(
        rect,
        editor,
        tag,
        style);
}

VSTGUI::CView*
Controller::verifyView(
    VSTGUI::CView* view,
    const VSTGUI::UIAttributes&,
    const VSTGUI::IUIDescription*,
    VSTGUI::VST3Editor* editor) {

    if (auto* control =
            dynamic_cast<
                VSTGUI::CControl*>(
                    view)) {

        if (control->getTag() ==
            kGuiZoomTag) {

            editor_ = editor;

            control->setListener(
                this);

            control->setMin(0.f);
            control->setMax(1.f);

            control->setValueNormalized(
                guiZoom_ >= 1.25
                    ? 1.f
                    : 0.f);
        }
    }

    return view;
}

void Controller::valueChanged(
    VSTGUI::CControl* control) {

    if (!control ||
        control->getTag() !=
            kGuiZoomTag ||
        !editor_) {
        return;
    }

    guiZoom_ =
        control->
            getValueNormalized() >=
                0.5f
            ? 1.5
            : 1.0;

    editor_->setZoomFactor(
        guiZoom_);
}

void Controller::willClose(
    VSTGUI::VST3Editor* editor) {

    if (editor_ == editor)
        editor_ = nullptr;
}

tresult PLUGIN_API
Controller::getParamStringByValue(
    Steinberg::Vst::ParamID id,
    ParamValue valueNormalized,
    String128 string) {

    switch (id) {
        case kFinish:
        case kRoom:
        case kRoomDecay:
            copyAscii(
                percentText(
                    valueNormalized),
                string);
            return kResultTrue;

        case kOutput:
            copyAscii(
                outputText(
                    valueNormalized),
                string);
            return kResultTrue;

        case kBypass:
            copyAscii(
                valueNormalized >= 0.5
                    ? "BYPASS"
                    : "ACTIVE",
                string);
            return kResultTrue;

        default:
            return EditController::
                getParamStringByValue(
                    id,
                    valueNormalized,
                    string);
    }
}

tresult PLUGIN_API
Controller::setComponentState(
    IBStream* state) {

    if (!state)
        return kInvalidArgument;

    IBStreamer stream(
        state,
        kLittleEndian);

    int32 version = 0;

    if (!stream.readInt32(
            version) ||
        version <
            kFirstSupportedStateVersion ||
        version >
            kStateVersion) {

        return kResultFalse;
    }

    const ParamID legacyIds[4] {
        kFinish,
        kRoom,
        kOutput,
        kBypass
    };

    double legacyRoom =
        0.0;

    for (const auto id :
         legacyIds) {

        double value = 0.0;

        if (!stream.readDouble(
                value) ||
            !std::isfinite(
                value)) {

            return kResultFalse;
        }

        const double normalized =
            std::clamp(
                value,
                0.0,
                1.0);

        setParamNormalized(
            id,
            normalized);

        if (id == kRoom)
            legacyRoom =
                normalized;
    }

    if (version >= 2) {
        double lowCut = 0.0;

        if (!stream.readDouble(
                lowCut) ||
            !std::isfinite(
                lowCut)) {

            return kResultFalse;
        }

        const double
            normalizedLowCut =
                version >= 4
                    ? std::clamp(
                        lowCut,
                        0.0,
                        1.0)
                    : (lowCut >= 0.5
                        ? dsp::
                            lowCutNormalizedFromFrequency(
                                80.0)
                        : 0.0);

        setParamNormalized(
            kLowCut80,
            normalizedLowCut);
    } else {
        setParamNormalized(
            kLowCut80,
            0.0);
    }

    if (version >= 3) {
        double decay = 0.0;

        if (!stream.readDouble(
                decay) ||
            !std::isfinite(
                decay)) {

            return kResultFalse;
        }

        setParamNormalized(
            kRoomDecay,
            std::clamp(
                decay,
                0.0,
                1.0));
    } else {
        setParamNormalized(
            kRoomDecay,
            legacyRoom);
    }

    return kResultOk;
}

} // namespace HighGainGuitarFinisher
