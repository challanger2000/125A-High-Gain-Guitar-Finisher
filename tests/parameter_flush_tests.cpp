#include "support/TestSupport.h"
#include "HighGainGuitarFinisherProcessor.h"
#include "HighGainGuitarFinisherIDs.h"

#include "public.sdk/source/vst/hosting/parameterchanges.h"

#include <array>
#include <cmath>

using HighGainGuitarFinisher::Processor;
using namespace HighGainGuitarFinisher;
using namespace Steinberg;
using namespace Steinberg::Vst;

namespace {

constexpr double kSampleRate = 48000.0;
constexpr int32 kBlockSize = 256;
constexpr double kInput = 0.1;

void addChange(
    ParameterChanges& changes,
    ParamID id,
    ParamValue value) {

    int32 queueIndex = 0;

    auto* queue =
        changes.addParameterData(
            id,
            queueIndex);

    HGGF_REQUIRE(queue != nullptr);

    int32 pointIndex = 0;

    HGGF_REQUIRE(
        queue->addPoint(
            0,
            value,
            pointIndex) ==
        kResultTrue);
}

struct AudioBlock {
    std::array<double, kBlockSize> inLeft {};
    std::array<double, kBlockSize> inRight {};
    std::array<double, kBlockSize> outLeft {};
    std::array<double, kBlockSize> outRight {};

    std::array<double*, 2> inputPointers {};
    std::array<double*, 2> outputPointers {};

    AudioBusBuffers inputBus {};
    AudioBusBuffers outputBus {};
    ProcessData data {};

    AudioBlock() {
        inLeft.fill(kInput);
        inRight.fill(kInput);

        inputPointers = {
            inLeft.data(),
            inRight.data()
        };

        outputPointers = {
            outLeft.data(),
            outRight.data()
        };

        inputBus.numChannels = 2;
        inputBus.silenceFlags = 0;
        inputBus.channelBuffers64 =
            inputPointers.data();

        outputBus.numChannels = 2;
        outputBus.silenceFlags = 0;
        outputBus.channelBuffers64 =
            outputPointers.data();

        data.processMode = kRealtime;
        data.symbolicSampleSize = kSample64;
        data.numSamples = kBlockSize;
        data.numInputs = 1;
        data.numOutputs = 1;
        data.inputs = &inputBus;
        data.outputs = &outputBus;
    }
};

void processFlush(
    Processor& processor,
    ParamID id,
    ParamValue value) {

    ParameterChanges changes(1);

    addChange(
        changes,
        id,
        value);

    ProcessData flush {};
    flush.processMode = kRealtime;
    flush.symbolicSampleSize = kSample64;
    flush.numSamples = 0;
    flush.numInputs = 0;
    flush.numOutputs = 0;
    flush.inputParameterChanges =
        &changes;

    HGGF_REQUIRE(
        processor.process(flush) ==
        kResultOk);
}

void processAudio(
    Processor& processor,
    AudioBlock& block,
    ParameterChanges* changes = nullptr) {

    block.data.inputParameterChanges =
        changes;

    HGGF_REQUIRE(
        processor.process(block.data) ==
        kResultOk);

    for (int i = 0; i < kBlockSize; ++i) {
        HGGF_REQUIRE(
            std::isfinite(
                block.outLeft[
                    static_cast<std::size_t>(i)]));
        HGGF_REQUIRE(
            std::isfinite(
                block.outRight[
                    static_cast<std::size_t>(i)]));
    }
}

} // namespace

int main() {
    Processor processor;

    HGGF_REQUIRE(
        processor.initialize(nullptr) ==
        kResultOk);

    ProcessSetup setup {};
    setup.processMode = kRealtime;
    setup.symbolicSampleSize = kSample64;
    setup.maxSamplesPerBlock = kBlockSize;
    setup.sampleRate = kSampleRate;

    HGGF_REQUIRE(
        processor.setupProcessing(setup) ==
        kResultOk);

    HGGF_REQUIRE(
        processor.setActive(true) ==
        kResultOk);

    HGGF_REQUIRE(
        processor.setProcessing(true) ==
        kResultOk);

    ParameterChanges outputChange(1);
    addChange(
        outputChange,
        kOutput,
        1.0);

    AudioBlock active;
    processAudio(
        processor,
        active,
        &outputChange);

    // +12 dB output trim should make the active path clearly distinct
    // from exact-unity bypass.
    HGGF_REQUIRE(
        active.outLeft[32] > 0.35);

    processFlush(
        processor,
        kBypass,
        1.0);

    AudioBlock enteringBypass;
    processAudio(
        processor,
        enteringBypass);

    // A parameter-only flush must not jump the audio lifecycle directly
    // to full bypass. The first sample remains near the processed path,
    // then the 5 ms crossfade reaches exact unity dry.
    HGGF_REQUIRE(
        enteringBypass.outLeft[0] > 0.30);

    HGGF_REQUIRE(
        std::abs(
            enteringBypass.outLeft.back() -
            kInput) <
        1.0e-12);

    processFlush(
        processor,
        kBypass,
        0.0);

    AudioBlock leavingBypass;
    processAudio(
        processor,
        leavingBypass);

    // Re-enable follows the inverse bounded transition.
    HGGF_REQUIRE(
        leavingBypass.outLeft[0] < 0.20);

    HGGF_REQUIRE(
        leavingBypass.outLeft.back() >
        0.35);

    HGGF_REQUIRE(
        processor.setProcessing(false) ==
        kResultOk);

    HGGF_REQUIRE(
        processor.setActive(false) ==
        kResultOk);

    HGGF_REQUIRE(
        processor.terminate() ==
        kResultOk);

    return 0;
}
