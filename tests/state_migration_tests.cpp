#include "support/TestSupport.h"
#include "HighGainGuitarFinisherIDs.h"
#include "HighGainGuitarFinisherProcessor.h"
#include "dsp/LowCutMapping.h"

#include "base/source/fstreamer.h"
#include "public.sdk/source/common/memorystream.h"

#include <cmath>
#include <limits>

using HighGainGuitarFinisher::Processor;
using namespace HighGainGuitarFinisher;
using namespace Steinberg;

namespace {

void requireNear(
    double actual,
    double expected,
    double tolerance = 1.0e-12) {

    HGGF_REQUIRE(
        std::abs(actual - expected) <=
        tolerance);
}

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

    HGGF_REQUIRE(
        writer.writeInt32(version));

    HGGF_REQUIRE(
        writer.writeDouble(finish));
    HGGF_REQUIRE(
        writer.writeDouble(room));
    HGGF_REQUIRE(
        writer.writeDouble(output));
    HGGF_REQUIRE(
        writer.writeDouble(bypass));

    if (version >= 2)
        HGGF_REQUIRE(
            writer.writeDouble(lowCut));

    if (version >= 3)
        HGGF_REQUIRE(
            writer.writeDouble(decay));

    if (version >= 5)
        HGGF_REQUIRE(
            writer.writeDouble(mode));

    if (version >= 6)
        HGGF_REQUIRE(
            writer.writeDouble(mass));

    rewindStream(stream);
    return stream;
}

struct CurrentState {
    int32 version {0};
    double finish {0.0};
    double room {0.0};
    double output {0.0};
    double bypass {0.0};
    double lowCut {0.0};
    double decay {0.0};
    double mode {0.0};
    double mass {0.0};
};

CurrentState readCurrentState(
    Processor& processor) {

    MemoryStream stream;

    HGGF_REQUIRE(
        processor.getState(
            &stream) == kResultOk);

    rewindStream(stream);

    IBStreamer reader(
        &stream,
        kLittleEndian);

    CurrentState state;

    HGGF_REQUIRE(
        reader.readInt32(
            state.version));

    HGGF_REQUIRE(
        reader.readDouble(
            state.finish));
    HGGF_REQUIRE(
        reader.readDouble(
            state.room));
    HGGF_REQUIRE(
        reader.readDouble(
            state.output));
    HGGF_REQUIRE(
        reader.readDouble(
            state.bypass));
    HGGF_REQUIRE(
        reader.readDouble(
            state.lowCut));
    HGGF_REQUIRE(
        reader.readDouble(
            state.decay));
    HGGF_REQUIRE(
        reader.readDouble(
            state.mode));
    HGGF_REQUIRE(
        reader.readDouble(
            state.mass));

    return state;
}

void verifyVersion1Migration() {
    Processor processor;

    auto legacy =
        makeState(
            1,
            0.73,
            0.42,
            0.61,
            1.0);

    HGGF_REQUIRE(
        processor.setState(
            &legacy) == kResultOk);

    const auto current =
        readCurrentState(
            processor);

    HGGF_REQUIRE(
        current.version ==
        kStateVersion);

    requireNear(
        current.finish,
        0.73);
    requireNear(
        current.room,
        0.42);
    requireNear(
        current.output,
        0.61);
    requireNear(
        current.bypass,
        1.0);

    // v1 had no LOW CUT.
    requireNear(
        current.lowCut,
        0.0);

    // v1 ROOM was a combined macro. DECAY inherits it.
    requireNear(
        current.decay,
        0.42);

    // MODE and MASS did not exist.
    requireNear(
        current.mode,
        0.0);
    requireNear(
        current.mass,
        0.0);
}

void verifyLegacyBooleanLowCutMigration() {
    for (const double legacyLowCut :
         {0.0, 1.0}) {

        Processor processor;

        auto legacy =
            makeState(
                2,
                0.25,
                0.33,
                0.50,
                0.0,
                legacyLowCut);

        HGGF_REQUIRE(
            processor.setState(
                &legacy) == kResultOk);

        const auto current =
            readCurrentState(
                processor);

        const double expectedLowCut =
            legacyLowCut >= 0.5
                ? dsp::
                    lowCutNormalizedFromFrequency(
                        80.0)
                : 0.0;

        requireNear(
            current.lowCut,
            expectedLowCut);

        // v2 still had no independent decay.
        requireNear(
            current.decay,
            0.33);

        requireNear(
            current.mode,
            0.0);

        requireNear(
            current.mass,
            0.0);
    }
}

void verifyVersion3And4Migration() {
    {
        Processor processor;

        auto legacy =
            makeState(
                3,
                0.55,
                0.45,
                0.50,
                0.0,
                1.0,
                0.81);

        HGGF_REQUIRE(
            processor.setState(
                &legacy) == kResultOk);

        const auto current =
            readCurrentState(
                processor);

        requireNear(
            current.lowCut,
            dsp::
                lowCutNormalizedFromFrequency(
                    80.0));

        requireNear(
            current.decay,
            0.81);

        requireNear(
            current.mode,
            0.0);

        requireNear(
            current.mass,
            0.0);
    }

    {
        Processor processor;

        constexpr double continuousLowCut =
            0.37;

        auto legacy =
            makeState(
                4,
                0.55,
                0.45,
                0.50,
                0.0,
                continuousLowCut,
                0.81);

        HGGF_REQUIRE(
            processor.setState(
                &legacy) == kResultOk);

        const auto current =
            readCurrentState(
                processor);

        requireNear(
            current.lowCut,
            continuousLowCut);

        requireNear(
            current.decay,
            0.81);

        requireNear(
            current.mode,
            0.0);

        requireNear(
            current.mass,
            0.0);
    }
}

void verifyVersion5Migration() {
    Processor processor;

    auto legacy =
        makeState(
            5,
            0.91,
            0.18,
            0.52,
            0.0,
            0.64,
            0.72,
            0.50);

    HGGF_REQUIRE(
        processor.setState(
            &legacy) == kResultOk);

    const auto current =
        readCurrentState(
            processor);

    requireNear(
        current.finish,
        0.91);

    requireNear(
        current.lowCut,
        0.64);

    requireNear(
        current.decay,
        0.72);

    requireNear(
        current.mode,
        0.50);

    // MASS was introduced in v6.
    requireNear(
        current.mass,
        0.0);
}

void verifyVersion6RoundTrip() {
    Processor processor;

    auto currentInput =
        makeState(
            6,
            0.83,
            0.27,
            0.58,
            0.0,
            0.43,
            0.76,
            1.0,
            0.69);

    HGGF_REQUIRE(
        processor.setState(
            &currentInput) == kResultOk);

    const auto current =
        readCurrentState(
            processor);

    HGGF_REQUIRE(
        current.version ==
        kStateVersion);

    requireNear(
        current.finish,
        0.83);

    requireNear(
        current.room,
        0.27);

    requireNear(
        current.output,
        0.58);

    requireNear(
        current.bypass,
        0.0);

    requireNear(
        current.lowCut,
        0.43);

    requireNear(
        current.decay,
        0.76);

    requireNear(
        current.mode,
        1.0);

    requireNear(
        current.mass,
        0.69);
}

void verifyStateClampingAndRejection() {
    {
        Processor processor;

        auto state =
            makeState(
                6,
                -2.0,
                4.0,
                2.0,
                -1.0,
                9.0,
                -3.0,
                8.0,
                3.0);

        HGGF_REQUIRE(
            processor.setState(
                &state) == kResultOk);

        const auto current =
            readCurrentState(
                processor);

        requireNear(
            current.finish,
            0.0);
        requireNear(
            current.room,
            1.0);
        requireNear(
            current.output,
            1.0);
        requireNear(
            current.bypass,
            0.0);
        requireNear(
            current.lowCut,
            1.0);
        requireNear(
            current.decay,
            0.0);
        requireNear(
            current.mode,
            1.0);
        requireNear(
            current.mass,
            1.0);
    }

    {
        Processor processor;

        auto invalidVersion =
            makeState(
                kStateVersion + 1,
                0.5,
                0.5,
                0.5,
                0.0,
                0.5,
                0.5,
                0.5,
                0.5);

        HGGF_REQUIRE(
            processor.setState(
                &invalidVersion) ==
            kResultFalse);
    }

    {
        MemoryStream stream;
        IBStreamer writer(
            &stream,
            kLittleEndian);

        HGGF_REQUIRE(
            writer.writeInt32(6));

        HGGF_REQUIRE(
            writer.writeDouble(
                std::numeric_limits<
                    double>::quiet_NaN()));

        rewindStream(stream);

        Processor processor;

        HGGF_REQUIRE(
            processor.setState(
                &stream) ==
            kResultFalse);
    }
}

} // namespace

int main() {
    verifyVersion1Migration();
    verifyLegacyBooleanLowCutMigration();
    verifyVersion3And4Migration();
    verifyVersion5Migration();
    verifyVersion6RoundTrip();
    verifyStateClampingAndRejection();

    return 0;
}
