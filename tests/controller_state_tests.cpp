#include "support/TestSupport.h"
#include "HighGainGuitarFinisherController.h"
#include "HighGainGuitarFinisherIDs.h"
#include "dsp/LowCutMapping.h"

#include "base/source/fstreamer.h"
#include "public.sdk/source/common/memorystream.h"

#include <algorithm>
#include <cmath>

using HighGainGuitarFinisher::Controller;
using namespace HighGainGuitarFinisher;
using namespace Steinberg;
using namespace Steinberg::Vst;

#if defined(_WIN32)
void* moduleHandle = nullptr;
#endif

namespace {

void rewindStream(MemoryStream& stream) {
    int64 position = 0;
    HGGF_REQUIRE(
        stream.seek(
            0,
            IBStream::kIBSeekSet,
            &position) == kResultOk);
    HGGF_REQUIRE(position == 0);
}

MemoryStream makeState(
    int32 version,
    double finish,
    double room,
    double output,
    double bypass,
    double lowCut = 0.0,
    double decay = 0.5,
    double mode = 0.0,
    double mass = 0.0) {

    MemoryStream stream;
    IBStreamer writer(
        &stream,
        kLittleEndian);

    HGGF_REQUIRE(writer.writeInt32(version));
    HGGF_REQUIRE(writer.writeDouble(finish));
    HGGF_REQUIRE(writer.writeDouble(room));
    HGGF_REQUIRE(writer.writeDouble(output));
    HGGF_REQUIRE(writer.writeDouble(bypass));

    if (version >= 2)
        HGGF_REQUIRE(writer.writeDouble(lowCut));

    if (version >= 3)
        HGGF_REQUIRE(writer.writeDouble(decay));

    if (version >= 5)
        HGGF_REQUIRE(writer.writeDouble(mode));

    if (version >= 6)
        HGGF_REQUIRE(writer.writeDouble(mass));

    rewindStream(stream);
    return stream;
}

void requireNear(
    double actual,
    double expected,
    double tolerance = 1.0e-12) {

    HGGF_REQUIRE(
        std::abs(actual - expected) <=
        tolerance);
}

void verifyVersion(
    int32 version,
    double finish,
    double room,
    double output,
    double bypass,
    double lowCut,
    double decay,
    double mode,
    double mass,
    double expectedLowCut,
    double expectedDecay,
    double expectedMode,
    double expectedMass) {

    Controller controller;

    HGGF_REQUIRE(
        controller.initialize(nullptr) ==
        kResultOk);

    auto state =
        makeState(
            version,
            finish,
            room,
            output,
            bypass,
            lowCut,
            decay,
            mode,
            mass);

    HGGF_REQUIRE(
        controller.setComponentState(
            &state) == kResultOk);

    requireNear(
        controller.getParamNormalized(kFinish),
        std::clamp(finish, 0.0, 1.0));

    requireNear(
        controller.getParamNormalized(kRoom),
        std::clamp(room, 0.0, 1.0));

    requireNear(
        controller.getParamNormalized(kOutput),
        std::clamp(output, 0.0, 1.0));

    requireNear(
        controller.getParamNormalized(kBypass),
        std::clamp(bypass, 0.0, 1.0));

    requireNear(
        controller.getParamNormalized(kLowCut80),
        expectedLowCut);

    requireNear(
        controller.getParamNormalized(kRoomDecay),
        expectedDecay);

    requireNear(
        controller.getParamNormalized(kMode),
        expectedMode);

    requireNear(
        controller.getParamNormalized(kMass),
        expectedMass);

    HGGF_REQUIRE(
        controller.terminate() ==
        kResultOk);
}

} // namespace

int main() {
    verifyVersion(
        1,
        0.73, 0.42, 0.61, 1.0,
        0.0, 0.5, 0.0, 0.0,
        0.0,
        0.42,
        0.0,
        0.0);

    verifyVersion(
        2,
        0.25, 0.33, 0.50, 0.0,
        1.0, 0.5, 0.0, 0.0,
        dsp::lowCutNormalizedFromFrequency(80.0),
        0.33,
        0.0,
        0.0);

    verifyVersion(
        3,
        0.55, 0.45, 0.50, 0.0,
        1.0, 0.81, 0.0, 0.0,
        dsp::lowCutNormalizedFromFrequency(80.0),
        0.81,
        0.0,
        0.0);

    verifyVersion(
        4,
        0.55, 0.45, 0.50, 0.0,
        0.37, 0.81, 0.0, 0.0,
        0.37,
        0.81,
        0.0,
        0.0);

    verifyVersion(
        5,
        0.91, 0.18, 0.52, 0.0,
        0.64, 0.72, 0.50, 0.0,
        0.64,
        0.72,
        0.50,
        0.0);

    verifyVersion(
        6,
        0.83, 0.27, 0.58, 0.0,
        0.43, 0.76, 1.0, 0.69,
        0.43,
        0.76,
        1.0,
        0.69);

    return 0;
}
