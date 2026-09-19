#include "HighGainGuitarFinisherController.h"
#include "HighGainGuitarFinisherIDs.h"

#include "base/source/fstreamer.h"
#include "public.sdk/source/vst/vstparameters.h"

#include <algorithm>
#include <cmath>

namespace HighGainGuitarFinisher {

using namespace Steinberg;
using namespace Steinberg::Vst;

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
        STR16("Wet / Dry"), STR16("%"), 0, 0.0, automate, kRoom);

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
        STR16("Low Cut 80 Hz"),
        STR16(""),
        1,
        0.0,
        ParameterInfo::kCanAutomate,
        kLowCut80);

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

        setParamNormalized(
            kLowCut80,
            std::clamp(lowCut, 0.0, 1.0));
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
