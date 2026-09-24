#include "support/TestSupport.h"
#include "HighGainGuitarFinisherProcessor.h"
#include "HighGainGuitarFinisherIDs.h"

#include "public.sdk/source/vst/hosting/parameterchanges.h"
#include "pluginterfaces/vst/vstspeaker.h"

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


void verifyStopStartLifecycleReset() {
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

    ParameterChanges changes(4);
    addChange(
        changes,
        HighGainGuitarFinisher::kFinish,
        1.0);
    addChange(
        changes,
        HighGainGuitarFinisher::kRoom,
        1.0);
    addChange(
        changes,
        HighGainGuitarFinisher::kRoomDecay,
        1.0);
    addChange(
        changes,
        HighGainGuitarFinisher::kMass,
        1.0);

    AudioBlock excited;
    processAudio(
        processor,
        excited,
        &changes);

    for (int block = 0;
         block < 8;
         ++block) {

        AudioBlock history;
        for (int i = 0; i < kBlockSize; ++i) {
            const double phase =
                2.0 * 3.14159265358979323846 *
                173.0 *
                static_cast<double>(
                    block * kBlockSize + i) /
                kSampleRate;

            history.inLeft[
                static_cast<std::size_t>(i)] =
                0.35 * std::sin(phase);

            history.inRight[
                static_cast<std::size_t>(i)] =
                0.31 * std::sin(
                    phase * 1.07);
        }

        processAudio(
            processor,
            history);
    }

    HGGF_REQUIRE(
        processor.setProcessing(false) ==
        kResultOk);
    HGGF_REQUIRE(
        processor.setProcessing(true) ==
        kResultOk);

    AudioBlock afterRestart;
    afterRestart.inLeft.fill(0.0);
    afterRestart.inRight.fill(0.0);

    processAudio(
        processor,
        afterRestart);

    HGGF_REQUIRE(
        afterRestart.outputBus.silenceFlags ==
        uint64 {3});

    for (int i = 0; i < kBlockSize; ++i) {
        HGGF_REQUIRE(
            afterRestart.outLeft[
                static_cast<std::size_t>(i)] ==
            0.0);

        HGGF_REQUIRE(
            afterRestart.outRight[
                static_cast<std::size_t>(i)] ==
            0.0);
    }

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

void verifyFloatDoubleParity() {
    Processor floatProcessor;
    Processor doubleProcessor;

    HGGF_REQUIRE(
        floatProcessor.initialize(nullptr) ==
        kResultOk);
    HGGF_REQUIRE(
        doubleProcessor.initialize(nullptr) ==
        kResultOk);

    ProcessSetup floatSetup {};
    floatSetup.processMode = kRealtime;
    floatSetup.symbolicSampleSize = kSample32;
    floatSetup.maxSamplesPerBlock = kBlockSize;
    floatSetup.sampleRate = kSampleRate;

    ProcessSetup doubleSetup =
        floatSetup;

    doubleSetup.symbolicSampleSize =
        kSample64;

    HGGF_REQUIRE(
        floatProcessor.setupProcessing(
            floatSetup) ==
        kResultOk);

    HGGF_REQUIRE(
        doubleProcessor.setupProcessing(
            doubleSetup) ==
        kResultOk);

    HGGF_REQUIRE(
        floatProcessor.setActive(true) ==
        kResultOk);
    HGGF_REQUIRE(
        doubleProcessor.setActive(true) ==
        kResultOk);
    HGGF_REQUIRE(
        floatProcessor.setProcessing(true) ==
        kResultOk);
    HGGF_REQUIRE(
        doubleProcessor.setProcessing(true) ==
        kResultOk);

    ParameterChanges floatChanges(7);
    ParameterChanges doubleChanges(7);

    const auto configure =
        [](ParameterChanges& changes) {
            addChange(
                changes,
                HighGainGuitarFinisher::kFinish,
                0.87);
            addChange(
                changes,
                HighGainGuitarFinisher::kRoom,
                0.37);
            addChange(
                changes,
                HighGainGuitarFinisher::kRoomDecay,
                0.68);
            addChange(
                changes,
                HighGainGuitarFinisher::kLowCut80,
                0.44);
            addChange(
                changes,
                HighGainGuitarFinisher::kMode,
                0.5);
            addChange(
                changes,
                HighGainGuitarFinisher::kMass,
                0.58);
            addChange(
                changes,
                HighGainGuitarFinisher::kOutput,
                0.57);
        };

    configure(floatChanges);
    configure(doubleChanges);

    std::array<float, kBlockSize> inLeft32 {};
    std::array<float, kBlockSize> inRight32 {};
    std::array<float, kBlockSize> outLeft32 {};
    std::array<float, kBlockSize> outRight32 {};
    std::array<float*, 2> in32 {
        inLeft32.data(),
        inRight32.data()
    };
    std::array<float*, 2> out32 {
        outLeft32.data(),
        outRight32.data()
    };

    std::array<double, kBlockSize> inLeft64 {};
    std::array<double, kBlockSize> inRight64 {};
    std::array<double, kBlockSize> outLeft64 {};
    std::array<double, kBlockSize> outRight64 {};
    std::array<double*, 2> in64 {
        inLeft64.data(),
        inRight64.data()
    };
    std::array<double*, 2> out64 {
        outLeft64.data(),
        outRight64.data()
    };

    AudioBusBuffers inBus32 {};
    AudioBusBuffers outBus32 {};
    AudioBusBuffers inBus64 {};
    AudioBusBuffers outBus64 {};

    inBus32.numChannels = 2;
    outBus32.numChannels = 2;
    inBus32.channelBuffers32 =
        in32.data();
    outBus32.channelBuffers32 =
        out32.data();

    inBus64.numChannels = 2;
    outBus64.numChannels = 2;
    inBus64.channelBuffers64 =
        in64.data();
    outBus64.channelBuffers64 =
        out64.data();

    ProcessData data32 {};
    data32.processMode = kRealtime;
    data32.symbolicSampleSize = kSample32;
    data32.numSamples = kBlockSize;
    data32.numInputs = 1;
    data32.numOutputs = 1;
    data32.inputs = &inBus32;
    data32.outputs = &outBus32;

    ProcessData data64 {};
    data64.processMode = kRealtime;
    data64.symbolicSampleSize = kSample64;
    data64.numSamples = kBlockSize;
    data64.numInputs = 1;
    data64.numOutputs = 1;
    data64.inputs = &inBus64;
    data64.outputs = &outBus64;

    constexpr double pi =
        3.141592653589793238462643383279502884;

    double maxDelta = 0.0;

    for (int block = 0;
         block < 16;
         ++block) {

        for (int i = 0;
             i < kBlockSize;
             ++i) {

            const double sample =
                static_cast<double>(
                    block * kBlockSize + i);

            const double t =
                sample /
                kSampleRate;

            const double left =
                0.31 * std::sin(
                    2.0 * pi * 113.0 * t) +
                0.18 * std::sin(
                    2.0 * pi * 423.0 * t) +
                0.16 * std::sin(
                    2.0 * pi * 3770.0 * t);

            const double right =
                0.29 * std::sin(
                    2.0 * pi * 127.0 * t) +
                0.17 * std::sin(
                    2.0 * pi * 463.0 * t) +
                0.15 * std::sin(
                    2.0 * pi * 6290.0 * t);

            inLeft64[
                static_cast<std::size_t>(i)] =
                left;

            inRight64[
                static_cast<std::size_t>(i)] =
                right;

            inLeft32[
                static_cast<std::size_t>(i)] =
                static_cast<float>(left);

            inRight32[
                static_cast<std::size_t>(i)] =
                static_cast<float>(right);
        }

        data32.inputParameterChanges =
            block == 0
                ? &floatChanges
                : nullptr;

        data64.inputParameterChanges =
            block == 0
                ? &doubleChanges
                : nullptr;

        HGGF_REQUIRE(
            floatProcessor.process(data32) ==
            kResultOk);

        HGGF_REQUIRE(
            doubleProcessor.process(data64) ==
            kResultOk);

        for (int i = 0;
             i < kBlockSize;
             ++i) {

            const auto index =
                static_cast<std::size_t>(i);

            HGGF_REQUIRE(
                std::isfinite(
                    outLeft32[index]));
            HGGF_REQUIRE(
                std::isfinite(
                    outRight32[index]));
            HGGF_REQUIRE(
                std::isfinite(
                    outLeft64[index]));
            HGGF_REQUIRE(
                std::isfinite(
                    outRight64[index]));

            maxDelta =
                std::max(
                    maxDelta,
                    std::abs(
                        static_cast<double>(
                            outLeft32[index]) -
                        outLeft64[index]));

            maxDelta =
                std::max(
                    maxDelta,
                    std::abs(
                        static_cast<double>(
                            outRight32[index]) -
                        outRight64[index]));
        }
    }

    HGGF_REQUIRE(
        maxDelta < 2.0e-6);

    HGGF_REQUIRE(
        floatProcessor.setProcessing(false) ==
        kResultOk);
    HGGF_REQUIRE(
        doubleProcessor.setProcessing(false) ==
        kResultOk);
    HGGF_REQUIRE(
        floatProcessor.setActive(false) ==
        kResultOk);
    HGGF_REQUIRE(
        doubleProcessor.setActive(false) ==
        kResultOk);
    HGGF_REQUIRE(
        floatProcessor.terminate() ==
        kResultOk);
    HGGF_REQUIRE(
        doubleProcessor.terminate() ==
        kResultOk);
}


void verifyMonoRoomMatchesStereoCollapse() {
    Processor mono;
    Processor stereo;

    HGGF_REQUIRE(
        mono.initialize(nullptr) ==
        kResultOk);

    HGGF_REQUIRE(
        stereo.initialize(nullptr) ==
        kResultOk);

    SpeakerArrangement monoIn =
        SpeakerArr::kMono;

    SpeakerArrangement monoOut =
        SpeakerArr::kMono;

    SpeakerArrangement stereoIn =
        SpeakerArr::kStereo;

    SpeakerArrangement stereoOut =
        SpeakerArr::kStereo;

    HGGF_REQUIRE(
        mono.setBusArrangements(
            &monoIn,
            1,
            &monoOut,
            1) ==
        kResultOk);

    HGGF_REQUIRE(
        stereo.setBusArrangements(
            &stereoIn,
            1,
            &stereoOut,
            1) ==
        kResultOk);

    ProcessSetup setup {};
    setup.processMode = kRealtime;
    setup.symbolicSampleSize = kSample64;
    setup.maxSamplesPerBlock = kBlockSize;
    setup.sampleRate = kSampleRate;

    HGGF_REQUIRE(
        mono.setupProcessing(setup) ==
        kResultOk);

    HGGF_REQUIRE(
        stereo.setupProcessing(setup) ==
        kResultOk);

    HGGF_REQUIRE(
        mono.setActive(true) ==
        kResultOk);

    HGGF_REQUIRE(
        stereo.setActive(true) ==
        kResultOk);

    HGGF_REQUIRE(
        mono.setProcessing(true) ==
        kResultOk);

    HGGF_REQUIRE(
        stereo.setProcessing(true) ==
        kResultOk);

    ParameterChanges monoChanges(6);
    ParameterChanges stereoChanges(6);

    const auto configure =
        [](ParameterChanges& changes) {
            addChange(
                changes,
                HighGainGuitarFinisher::kFinish,
                0.84);
            addChange(
                changes,
                HighGainGuitarFinisher::kRoom,
                0.73);
            addChange(
                changes,
                HighGainGuitarFinisher::kRoomDecay,
                0.82);
            addChange(
                changes,
                HighGainGuitarFinisher::kLowCut80,
                0.46);
            addChange(
                changes,
                HighGainGuitarFinisher::kMass,
                0.55);
            addChange(
                changes,
                HighGainGuitarFinisher::kOutput,
                0.53);
        };

    configure(monoChanges);
    configure(stereoChanges);

    std::array<double, kBlockSize> monoInput {};
    std::array<double, kBlockSize> monoOutput {};
    std::array<double*, 1> monoInputPointers {
        monoInput.data()
    };
    std::array<double*, 1> monoOutputPointers {
        monoOutput.data()
    };

    AudioBusBuffers monoInputBus {};
    AudioBusBuffers monoOutputBus {};

    monoInputBus.numChannels = 1;
    monoInputBus.channelBuffers64 =
        monoInputPointers.data();

    monoOutputBus.numChannels = 1;
    monoOutputBus.channelBuffers64 =
        monoOutputPointers.data();

    ProcessData monoData {};
    monoData.processMode = kRealtime;
    monoData.symbolicSampleSize = kSample64;
    monoData.numSamples = kBlockSize;
    monoData.numInputs = 1;
    monoData.numOutputs = 1;
    monoData.inputs = &monoInputBus;
    monoData.outputs = &monoOutputBus;

    constexpr double pi =
        3.141592653589793238462643383279502884;

    bool monoTailObserved = false;

    for (int block = 0;
         block < 40;
         ++block) {

        AudioBlock stereoBlock;

        const bool sourceActive =
            block < 10;

        for (int i = 0;
             i < kBlockSize;
             ++i) {

            const double absoluteSample =
                static_cast<double>(
                    block * kBlockSize + i);

            const double t =
                absoluteSample /
                kSampleRate;

            const double sample =
                sourceActive
                    ? (
                        0.32 * std::sin(
                            2.0 * pi *
                            109.0 * t) +
                        0.19 * std::sin(
                            2.0 * pi *
                            347.0 * t) +
                        0.14 * std::sin(
                            2.0 * pi *
                            3900.0 * t))
                    : 0.0;

            monoInput[
                static_cast<std::size_t>(i)] =
                sample;

            stereoBlock.inLeft[
                static_cast<std::size_t>(i)] =
                sample;

            stereoBlock.inRight[
                static_cast<std::size_t>(i)] =
                sample;
        }

        monoData.inputParameterChanges =
            block == 0
                ? &monoChanges
                : nullptr;

        HGGF_REQUIRE(
            mono.process(monoData) ==
            kResultOk);

        processAudio(
            stereo,
            stereoBlock,
            block == 0
                ? &stereoChanges
                : nullptr);

        for (int i = 0;
             i < kBlockSize;
             ++i) {

            const auto index =
                static_cast<std::size_t>(i);

            const double expectedMono =
                0.5 * (
                    stereoBlock.outLeft[index] +
                    stereoBlock.outRight[index]);

            HGGF_REQUIRE(
                monoOutput[index] ==
                expectedMono);
        }

        const bool stereoSilent =
            stereoBlock.outputBus.silenceFlags ==
            uint64 {3};

        const bool monoSilent =
            monoOutputBus.silenceFlags ==
            uint64 {1};

        HGGF_REQUIRE(
            monoSilent ==
            stereoSilent);

        if (!sourceActive &&
            !monoSilent) {
            monoTailObserved = true;
        }
    }

    HGGF_REQUIRE(
        monoTailObserved);

    HGGF_REQUIRE(
        mono.setProcessing(false) ==
        kResultOk);

    HGGF_REQUIRE(
        stereo.setProcessing(false) ==
        kResultOk);

    HGGF_REQUIRE(
        mono.setActive(false) ==
        kResultOk);

    HGGF_REQUIRE(
        stereo.setActive(false) ==
        kResultOk);

    HGGF_REQUIRE(
        mono.terminate() ==
        kResultOk);

    HGGF_REQUIRE(
        stereo.terminate() ==
        kResultOk);
}

void verifyVariableBlockAndRateLifecycle() {
    Processor processor;

    HGGF_REQUIRE(
        processor.initialize(nullptr) ==
        kResultOk);

    constexpr int32 maxBlock = 1024;

    std::array<double, maxBlock> inLeft {};
    std::array<double, maxBlock> inRight {};
    std::array<double, maxBlock> outLeft {};
    std::array<double, maxBlock> outRight {};

    std::array<double*, 2> inputPointers {
        inLeft.data(),
        inRight.data()
    };

    std::array<double*, 2> outputPointers {
        outLeft.data(),
        outRight.data()
    };

    AudioBusBuffers inputBus {};
    AudioBusBuffers outputBus {};

    inputBus.numChannels = 2;
    inputBus.channelBuffers64 =
        inputPointers.data();

    outputBus.numChannels = 2;
    outputBus.channelBuffers64 =
        outputPointers.data();

    ProcessData data {};
    data.processMode = kRealtime;
    data.symbolicSampleSize = kSample64;
    data.numInputs = 1;
    data.numOutputs = 1;
    data.inputs = &inputBus;
    data.outputs = &outputBus;

    const int32 blockSizes[] {
        1,
        2,
        7,
        31,
        32,
        63,
        64,
        127,
        128,
        255,
        256,
        511,
        1024
    };

    constexpr double pi =
        3.141592653589793238462643383279502884;

    for (const double sampleRate :
         {44100.0,
          48000.0,
          96000.0,
          192000.0}) {

        ProcessSetup setup {};
        setup.processMode = kRealtime;
        setup.symbolicSampleSize = kSample64;
        setup.maxSamplesPerBlock = maxBlock;
        setup.sampleRate = sampleRate;

        HGGF_REQUIRE(
            processor.setupProcessing(
                setup) ==
            kResultOk);

        HGGF_REQUIRE(
            processor.getTailSamples() ==
            static_cast<uint32>(
                std::llround(
                    sampleRate * 6.0)));

        HGGF_REQUIRE(
            processor.setActive(true) ==
            kResultOk);

        HGGF_REQUIRE(
            processor.setProcessing(true) ==
            kResultOk);

        ParameterChanges initialChanges(6);

        addChange(
            initialChanges,
            HighGainGuitarFinisher::kFinish,
            0.82);

        addChange(
            initialChanges,
            HighGainGuitarFinisher::kMass,
            0.61);

        addChange(
            initialChanges,
            HighGainGuitarFinisher::kRoom,
            0.34);

        addChange(
            initialChanges,
            HighGainGuitarFinisher::kRoomDecay,
            0.72);

        addChange(
            initialChanges,
            HighGainGuitarFinisher::kLowCut80,
            0.47);

        addChange(
            initialChanges,
            HighGainGuitarFinisher::kMode,
            0.5);

        bool firstBlock = true;
        int64 absoluteSample = 0;

        for (const int32 blockSize :
             blockSizes) {

            for (int32 i = 0;
                 i < blockSize;
                 ++i) {

                const double time =
                    static_cast<double>(
                        absoluteSample + i) /
                    sampleRate;

                inLeft[
                    static_cast<std::size_t>(i)] =
                    0.31 * std::sin(
                        2.0 * pi * 117.0 * time) +
                    0.19 * std::sin(
                        2.0 * pi * 3900.0 * time);

                inRight[
                    static_cast<std::size_t>(i)] =
                    0.29 * std::sin(
                        2.0 * pi * 139.0 * time) +
                    0.17 * std::sin(
                        2.0 * pi * 6100.0 * time);
            }

            inputBus.silenceFlags = 0;
            outputBus.silenceFlags = 0;

            data.numSamples = blockSize;
            data.inputParameterChanges =
                firstBlock
                    ? &initialChanges
                    : nullptr;

            HGGF_REQUIRE(
                processor.process(data) ==
                kResultOk);

            for (int32 i = 0;
                 i < blockSize;
                 ++i) {

                HGGF_REQUIRE(
                    std::isfinite(
                        outLeft[
                            static_cast<std::size_t>(i)]));

                HGGF_REQUIRE(
                    std::isfinite(
                        outRight[
                            static_cast<std::size_t>(i)]));
            }

            firstBlock = false;
            absoluteSample += blockSize;
        }

        HGGF_REQUIRE(
            processor.setProcessing(false) ==
            kResultOk);

        HGGF_REQUIRE(
            processor.setActive(false) ==
            kResultOk);
    }

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
    verifyStopStartLifecycleReset();
    verifyFloatDoubleParity();
    verifyMonoRoomMatchesStereoCollapse();
    verifyVariableBlockAndRateLifecycle();

    return 0;
}
