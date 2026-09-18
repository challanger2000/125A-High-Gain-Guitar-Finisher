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

    parameters.addParameter(
        STR16("Room"), STR16("%"), 0, 0.0, automate, kRoom);

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

    return kResultOk;
}

tresult PLUGIN_API Controller::setComponentState(IBStream* state) {
    if (!state)
        return kInvalidArgument;

    IBStreamer stream(state, kLittleEndian);

    int32 version = 0;
    if (!stream.readInt32(version) || version != kStateVersion)
        return kResultFalse;

    const ParamID ids[4] {kFinish, kRoom, kOutput, kBypass};

    for (const auto id : ids) {
        double value = 0.0;
        if (!stream.readDouble(value) || !std::isfinite(value))
            return kResultFalse;

        setParamNormalized(id, std::clamp(value, 0.0, 1.0));
    }

    return kResultOk;
}

} // namespace HighGainGuitarFinisher
