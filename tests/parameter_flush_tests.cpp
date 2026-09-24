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
constexpr double kInputSample = 0.1;

void addChange(
    ParameterChanges& changes,
    Steinberg::Vst::ParamID id,
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
        inLeft.fill(kInputSample);
        inRight.fill(kInputSample);

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
    Steinberg::Vst::ParamID id,
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


void verifyOfflineRealtimeParity() {
    Processor realtime;
    Processor offline;

    HGGF_REQUIRE(
        realtime.initialize(nullptr) ==
        kResultOk);

    HGGF_REQUIRE(
        offline.initialize(nullptr) ==
        kResultOk);

    ProcessSetup realtimeSetup {};
    realtimeSetup.processMode = kRealtime;
    realtimeSetup.symbolicSampleSize = kSample64;
    realtimeSetup.maxSamplesPerBlock = kBlockSize;
    realtimeSetup.sampleRate = kSampleRate;

    ProcessSetup offlineSetup =
        realtimeSetup;

    offlineSetup.processMode =
        kOffline;

    HGGF_REQUIRE(
        realtime.setupProcessing(
            realtimeSetup) ==
        kResultOk);

    HGGF_REQUIRE(
        offline.setupProcessing(
            offlineSetup) ==
        kResultOk);

    HGGF_REQUIRE(
        realtime.setActive(true) ==
        kResultOk);

    HGGF_REQUIRE(
        offline.setActive(true) ==
        kResultOk);

    HGGF_REQUIRE(
        realtime.setProcessing(true) ==
        kResultOk);

    HGGF_REQUIRE(
        offline.setProcessing(true) ==
        kResultOk);

    ParameterChanges realtimeChanges(7);
    ParameterChanges offlineChanges(7);

    const auto configure =
        [](ParameterChanges& changes) {
            addChange(
                changes,
                HighGainGuitarFinisher::kFinish,
                1.0);
            addChange(
                changes,
                HighGainGuitarFinisher::kRoom,
                0.42);
            addChange(
                changes,
                HighGainGuitarFinisher::kRoomDecay,
                0.71);
            addChange(
                changes,
                HighGainGuitarFinisher::kLowCut80,
                0.58);
            addChange(
                changes,
                HighGainGuitarFinisher::kMode,
                0.5);
            addChange(
                changes,
                HighGainGuitarFinisher::kMass,
                0.63);
            addChange(
                changes,
                HighGainGuitarFinisher::kOutput,
                0.56);
        };

    configure(realtimeChanges);
    configure(offlineChanges);

    constexpr double pi =
        3.141592653589793238462643383279502884;

    for (int blockIndex = 0;
         blockIndex < 12;
         ++blockIndex) {

        AudioBlock realtimeBlock;
        AudioBlock offlineBlock;

        realtimeBlock.data.processMode =
            kRealtime;

        offlineBlock.data.processMode =
            kOffline;

        for (int i = 0;
             i < kBlockSize;
             ++i) {

            const double sample =
                static_cast<double>(
                    blockIndex *
                        kBlockSize +
                    i);

            const double time =
                sample /
                kSampleRate;

            const double gate =
                (blockIndex & 1) == 0
                    ? 1.0
                    : 0.31;

            const double left =
                gate * (
                    0.28 *
                    std::sin(
                        2.0 * pi *
                        107.0 * time) +
                    0.18 *
                    std::sin(
                        2.0 * pi *
                        337.0 * time)) +
                0.17 *
                std::sin(
                    2.0 * pi *
                    1750.0 * time) +
                0.13 *
                std::sin(
                    2.0 * pi *
                    4300.0 * time);

            const double right =
                gate * (
                    0.27 *
                    std::sin(
                        2.0 * pi *
                        119.0 * time) +
                    0.17 *
                    std::sin(
                        2.0 * pi *
                        371.0 * time)) +
                0.16 *
                std::sin(
                    2.0 * pi *
                    1870.0 * time) +
                0.12 *
                std::sin(
                    2.0 * pi *
                    6100.0 * time);

            realtimeBlock.inLeft[
                static_cast<std::size_t>(i)] =
                left;

            realtimeBlock.inRight[
                static_cast<std::size_t>(i)] =
                right;

            offlineBlock.inLeft[
                static_cast<std::size_t>(i)] =
                left;

            offlineBlock.inRight[
                static_cast<std::size_t>(i)] =
                right;
        }

        processAudio(
            realtime,
            realtimeBlock,
            blockIndex == 0
                ? &realtimeChanges
                : nullptr);

        processAudio(
            offline,
            offlineBlock,
            blockIndex == 0
                ? &offlineChanges
                : nullptr);

        for (int i = 0;
             i < kBlockSize;
             ++i) {

            const auto index =
                static_cast<std::size_t>(i);

            HGGF_REQUIRE(
                realtimeBlock.outLeft[index] ==
                offlineBlock.outLeft[index]);

            HGGF_REQUIRE(
                realtimeBlock.outRight[index] ==
                offlineBlock.outRight[index]);
        }
    }

    HGGF_REQUIRE(
        realtime.setProcessing(false) ==
        kResultOk);

    HGGF_REQUIRE(
        offline.setProcessing(false) ==
        kResultOk);

    HGGF_REQUIRE(
        realtime.setActive(false) ==
        kResultOk);

    HGGF_REQUIRE(
        offline.setActive(false) ==
        kResultOk);

    HGGF_REQUIRE(
        realtime.terminate() ==
        kResultOk);

    HGGF_REQUIRE(
        offline.terminate() ==
        kResultOk);
}


void verifySilenceFlagsAndRoomTail() {
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

    // Exact digital silence with ROOM off must mark both stereo
    // output channels silent.
    AudioBlock silentBlock;
    silentBlock.inLeft.fill(0.0);
    silentBlock.inRight.fill(0.0);

    processAudio(
        processor,
        silentBlock);

    HGGF_REQUIRE(
        silentBlock.outputBus.silenceFlags ==
        uint64 {3});

    // Channel flags are independent: left-only dry audio keeps only
    // the right channel marked silent.
    AudioBlock leftOnlyBlock;
    leftOnlyBlock.inLeft.fill(0.125);
    leftOnlyBlock.inRight.fill(0.0);

    processAudio(
        processor,
        leftOnlyBlock);

    HGGF_REQUIRE(
        leftOnlyBlock.outputBus.silenceFlags ==
        uint64 {2});

    ParameterChanges roomChanges(2);
    addChange(
        roomChanges,
        HighGainGuitarFinisher::kRoom,
        1.0);
    addChange(
        roomChanges,
        HighGainGuitarFinisher::kRoomDecay,
        1.0);

    AudioBlock impulseBlock;
    impulseBlock.inLeft.fill(0.0);
    impulseBlock.inRight.fill(0.0);
    impulseBlock.inLeft[0] = 0.5;
    impulseBlock.inRight[0] = 0.5;

    processAudio(
        processor,
        impulseBlock,
        &roomChanges);

    HGGF_REQUIRE(
        impulseBlock.outputBus.silenceFlags ==
        uint64 {0});

    bool tailObserved = false;

    for (int block = 0;
         block < 24;
         ++block) {

        AudioBlock tailBlock;
        tailBlock.inLeft.fill(0.0);
        tailBlock.inRight.fill(0.0);

        processAudio(
            processor,
            tailBlock);

        if (tailBlock.outputBus.silenceFlags !=
            uint64 {3}) {
            tailObserved = true;
            break;
        }
    }

    // The delayed ROOM field starts after the dry impulse. Silence flags
    // must therefore become non-silent again when the reported tail arrives.
    HGGF_REQUIRE(tailObserved);

    HGGF_REQUIRE(
        processor.setProcessing(false) ==
        kResultOk);

    HGGF_REQUIRE(
        processor.setActive(false) ==
        kResultOk);

    HGGF_REQUIRE(
        processor.terminate() ==
        kResultOk);
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
        HighGainGuitarFinisher::kOutput,
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
        HighGainGuitarFinisher::kBypass,
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
            kInputSample) <
        1.0e-12);

    processFlush(
        processor,
        HighGainGuitarFinisher::kBypass,
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

    verifyOfflineRealtimeParity();
    verifySilenceFlagsAndRoomTail();

    return 0;
}
