#include "HighGainGuitarFinisherController.h"
#include "HighGainGuitarFinisherIDs.h"
#include "dsp/LowCutMapping.h"

#include "base/source/fstreamer.h"
#include "pluginterfaces/base/ustring.h"
#include "public.sdk/source/vst/vstparameters.h"

#include <algorithm>
#include <cmath>

namespace HighGainGuitarFinisher {

using namespace Steinberg;
using namespace Steinberg::Vst;

namespace {

class LowCutParameter final : public Parameter {
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
        ParamValue valueNormalized,
        String128 string) const SMTG_OVERRIDE {

        UString128 result;

        if (!dsp::lowCutEnabled(valueNormalized)) {
            result.fromAscii("Off");
        } else {
            result.printFloat(
                dsp::lowCutFrequencyFromNormalized(
                    valueNormalized),
                0);
        }

        result.copyTo(string, 128);
    }

    bool fromString(
        const TChar* string,
        ParamValue& valueNormalized) const SMTG_OVERRIDE {

        UString value(
            const_cast<TChar*>(string),
            strlen16(string));

        ParamValue frequency = 0.0;

        if (!value.scanFloat(frequency))
            return false;

        valueNormalized =
            dsp::lowCutNormalizedFromFrequency(
                frequency);

        return true;
    }

    ParamValue toPlain(
        ParamValue valueNormalized) const SMTG_OVERRIDE {

        return dsp::lowCutEnabled(valueNormalized)
            ? dsp::lowCutFrequencyFromNormalized(
                valueNormalized)
            : 0.0;
    }

    ParamValue toNormalized(
        ParamValue plainValue) const SMTG_OVERRIDE {

        return dsp::lowCutNormalizedFromFrequency(
            plainValue);
    }

    OBJ_METHODS(LowCutParameter, Parameter)
};

} // namespace

tresult PLUGIN_API Controller::initialize(FUnknown* context) {
    const auto result = EditController::initialize(context);
    if (result != kResultOk)
        return result;

    constexpr int32 automate = ParameterInfo::kCanAutomate;

    parameters.addParameter(
        STR16("Finish"), STR16("%"), 0, 0.0, automate, kFinish);

    // Keep the original ROOM parameter ID for backward compatibility,
    // but expose it as the wet/dry amount from state version 3 onward.
    parameters.addParameter(
        STR16("Wet"), STR16("%"), 0, 0.0, automate, kRoom);

    parameters.addParameter(
        STR16("Decay"), STR16("%"), 0, 0.5, automate, kRoomDecay);

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
        ParameterInfo::kCanAutomate | ParameterInfo::kIsBypass,
        kBypass);

    parameters.addParameter(
        new LowCutParameter());

    return kResultOk;
}

tresult PLUGIN_API Controller::setComponentState(IBStream* state) {
    if (!state)
        return kInvalidArgument;

    IBStreamer stream(state, kLittleEndian);

    int32 version = 0;
    if (!stream.readInt32(version) ||
        version < kFirstSupportedStateVersion ||
        version > kStateVersion) {
        return kResultFalse;
    }

    const ParamID legacyIds[4] {
        kFinish,
        kRoom,
        kOutput,
        kBypass
    };

    double legacyRoom = 0.0;

    for (const auto id : legacyIds) {
        double value = 0.0;

        if (!stream.readDouble(value) ||
            !std::isfinite(value)) {
            return kResultFalse;
        }

        const double normalized =
            std::clamp(value, 0.0, 1.0);

        setParamNormalized(
            id,
            normalized);

        if (id == kRoom)
            legacyRoom = normalized;
    }

    if (version >= 2) {
        double lowCut = 0.0;

        if (!stream.readDouble(lowCut) ||
            !std::isfinite(lowCut)) {
            return kResultFalse;
        }

        const double normalizedLowCut =
            version >= 4
                ? std::clamp(lowCut, 0.0, 1.0)
                : (lowCut >= 0.5
                    ? dsp::lowCutNormalizedFromFrequency(80.0)
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

        if (!stream.readDouble(decay) ||
            !std::isfinite(decay)) {
            return kResultFalse;
        }

        setParamNormalized(
            kRoomDecay,
            std::clamp(decay, 0.0, 1.0));
    } else {
        // The old ROOM macro controlled both wet level and tail length.
        // Mapping its saved value to both new controls preserves that intent.
        setParamNormalized(
            kRoomDecay,
            legacyRoom);
    }

    return kResultOk;
}

} // namespace HighGainGuitarFinisher
