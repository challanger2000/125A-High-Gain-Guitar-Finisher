#include "HighGainGuitarFinisherProcessor.h"
#include "HighGainGuitarFinisherIDs.h"
#include "dsp/LowCutMapping.h"

#include "base/source/fstreamer.h"
#include "pluginterfaces/vst/ivstparameterchanges.h"
#include "pluginterfaces/vst/vstspeaker.h"

#include <algorithm>
#include <cmath>

namespace HighGainGuitarFinisher {

using namespace Steinberg;
using namespace Steinberg::Vst;

Processor::Processor() {
    setControllerClass(kControllerUID);
}

tresult PLUGIN_API Processor::initialize(FUnknown* context) {
    const auto result = AudioEffect::initialize(context);
    if (result != kResultOk)
        return result;

    addAudioInput(STR16("Stereo In"), SpeakerArr::kStereo);
    addAudioOutput(STR16("Stereo Out"), SpeakerArr::kStereo);
    return kResultOk;
}

tresult PLUGIN_API Processor::setBusArrangements(
    SpeakerArrangement* inputs,
    int32 numIns,
    SpeakerArrangement* outputs,
    int32 numOuts) {

    if (numIns == 1 && numOuts == 1 &&
        inputs[0] == SpeakerArr::kStereo &&
        outputs[0] == SpeakerArr::kStereo) {
        return AudioEffect::setBusArrangements(inputs, numIns, outputs, numOuts);
    }

    return kResultFalse;
}

tresult PLUGIN_API Processor::canProcessSampleSize(int32 symbolicSampleSize) {
    return (symbolicSampleSize == kSample32 || symbolicSampleSize == kSample64)
        ? kResultTrue
        : kResultFalse;
}

uint32 PLUGIN_API Processor::getTailSamples() {
    // Report the maximum ROOM decay to the host even when ROOM is currently
    // at zero. Hosts may cache this value, and a later ROOM automation change
    // must not allow the reverb tail to be suspended or truncated.
    constexpr double kMaximumTailSeconds = 6.0;

    return static_cast<uint32>(
        std::max(
            1.0,
            std::round(
                sampleRate_ *
                kMaximumTailSeconds)));
}

tresult PLUGIN_API Processor::setupProcessing(ProcessSetup& setup) {
    sampleRate_ = (std::isfinite(setup.sampleRate) && setup.sampleRate > 1000.0)
        ? setup.sampleRate
        : 44100.0;

    // ROOM allocates delay storage during prepare(). Keep allocation out of
    // process(), but never let an allocation failure escape across the VST3 ABI.
    try {
        finisher_.prepare(sampleRate_);
    } catch (...) {
        return kResultFalse;
    }

    finisher_.setFinish(finish_);
    finisher_.setLowCut(lowCut_);
    finisher_.setRoomWet(room_);
    finisher_.setRoomDecay(roomDecay_);
    finisher_.setMode(mode_);
    lastBypassed_ = bypass_ >= 0.5;

    return AudioEffect::setupProcessing(setup);
}

tresult PLUGIN_API Processor::setActive(TBool state) {
    if (state)
        finisher_.reset();

    return AudioEffect::setActive(state);
}

tresult PLUGIN_API Processor::setProcessing(TBool state) {
    if (state)
        finisher_.reset();

    // Steinberg AudioEffect::setProcessing() deliberately returns
    // kNotImplemented. Hosts and lifecycle tests expect our concrete
    // processor to acknowledge the transition explicitly.
    AudioEffect::setProcessing(state);
    return kResultTrue;
}

void Processor::readParameterChanges(IParameterChanges* changes) {
    if (!changes)
        return;

    for (int32 i = 0; i < changes->getParameterCount(); ++i) {
        auto* queue = changes->getParameterData(i);
        if (!queue || queue->getPointCount() <= 0)
            continue;

        int32 sampleOffset = 0;
        ParamValue value = 0.0;

        if (queue->getPoint(
                queue->getPointCount() - 1,
                sampleOffset,
                value) != kResultTrue) {
            continue;
        }

        if (!std::isfinite(value))
            continue;

        value = std::clamp(value, 0.0, 1.0);

        switch (queue->getParameterId()) {
            case kFinish:   finish_ = value; break;
            case kRoom:     room_ = value; break;
            case kRoomDecay: roomDecay_ = value; break;
            case kOutput:   output_ = value; break;
            case kBypass:   bypass_ = value; break;
            case kLowCut80: lowCut_ = value; break;
            case kMode:     mode_ = value; break;
            default: break;
        }
    }
}

template <typename Sample>
void Processor::processBlock(
    Sample** inputs,
    Sample** outputs,
    int32 numSamples,
    int32 numChannels) {

    const bool bypassed = bypass_ >= 0.5;

    if (bypassed != lastBypassed_) {
        finisher_.reset();
        lastBypassed_ = bypassed;
    }

    const double outputDb = (output_ * 24.0) - 12.0;

    const double outputGain =
        bypassed
            ? 1.0
            : std::pow(10.0, outputDb / 20.0);

    finisher_.setFinish(finish_);
    finisher_.setLowCut(lowCut_);
    finisher_.setRoomWet(room_);
    finisher_.setRoomDecay(roomDecay_);
    finisher_.setMode(mode_);

    for (int32 sample = 0; sample < numSamples; ++sample) {
        const Sample* inputLeft = inputs[0];
        const Sample* inputRight =
            numChannels > 1 ? inputs[1] : inputs[0];

        Sample* outputLeft = outputs[0];
        Sample* outputRight =
            numChannels > 1 ? outputs[1] : nullptr;

        if (!outputLeft)
            continue;

        double left =
            inputLeft
                ? static_cast<double>(inputLeft[sample])
                : 0.0;

        double right =
            inputRight
                ? static_cast<double>(inputRight[sample])
                : left;

        if (!std::isfinite(left))
            left = 0.0;
        if (!std::isfinite(right))
            right = 0.0;

        if (!bypassed)
            finisher_.processFrame(left, right);

        outputLeft[sample] =
            static_cast<Sample>(left * outputGain);

        if (outputRight) {
            outputRight[sample] =
                static_cast<Sample>(right * outputGain);
        }
    }
}

tresult PLUGIN_API Processor::process(ProcessData& data) {
    readParameterChanges(data.inputParameterChanges);

    if (data.numInputs == 0 ||
        data.numOutputs == 0 ||
        data.numSamples <= 0) {
        return kResultOk;
    }

    const int32 numChannels = std::min<int32>(
        2,
        std::min(
            data.inputs[0].numChannels,
            data.outputs[0].numChannels));

    if (numChannels <= 0)
        return kResultOk;

    if (data.symbolicSampleSize == kSample32) {
        processBlock(
            data.inputs[0].channelBuffers32,
            data.outputs[0].channelBuffers32,
            data.numSamples,
            numChannels);
    } else if (data.symbolicSampleSize == kSample64) {
        processBlock(
            data.inputs[0].channelBuffers64,
            data.outputs[0].channelBuffers64,
            data.numSamples,
            numChannels);
    } else {
        return kResultFalse;
    }

    bool silent = true;

    if (data.symbolicSampleSize == kSample32) {
        for (int32 channel = 0;
             channel < numChannels && silent;
             ++channel) {

            const auto* output =
                data.outputs[0].channelBuffers32[channel];

            if (!output)
                continue;

            for (int32 sample = 0;
                 sample < data.numSamples;
                 ++sample) {

                if (output[sample] != 0.0f) {
                    silent = false;
                    break;
                }
            }
        }
    } else {
        for (int32 channel = 0;
             channel < numChannels && silent;
             ++channel) {

            const auto* output =
                data.outputs[0].channelBuffers64[channel];

            if (!output)
                continue;

            for (int32 sample = 0;
                 sample < data.numSamples;
                 ++sample) {

                if (output[sample] != 0.0) {
                    silent = false;
                    break;
                }
            }
        }
    }

    data.outputs[0].silenceFlags = silent
        ? ((Steinberg::uint64 {1} << numChannels) - 1)
        : 0;

    return kResultOk;
}

tresult PLUGIN_API Processor::setState(IBStream* state) {
    if (!state)
        return kInvalidArgument;

    IBStreamer stream(state, kLittleEndian);

    int32 version = 0;
    if (!stream.readInt32(version) ||
        version < kFirstSupportedStateVersion ||
        version > kStateVersion) {
        return kResultFalse;
    }

    double legacyValues[4] {};

    for (double& value : legacyValues) {
        if (!stream.readDouble(value) || !std::isfinite(value))
            return kResultFalse;

        value = std::clamp(value, 0.0, 1.0);
    }

    finish_ = legacyValues[0];
    room_ = legacyValues[1];
    output_ = legacyValues[2];
    bypass_ = legacyValues[3];

    if (version >= 2) {
        double savedLowCut = 0.0;

        if (!stream.readDouble(savedLowCut) ||
            !std::isfinite(savedLowCut)) {
            return kResultFalse;
        }

        lowCut_ =
            version >= 4
                ? std::clamp(savedLowCut, 0.0, 1.0)
                : (savedLowCut >= 0.5
                    ? dsp::lowCutNormalizedFromFrequency(80.0)
                    : 0.0);
    } else {
        lowCut_ = 0.0;
    }

    if (version >= 3) {
        if (!stream.readDouble(roomDecay_) ||
            !std::isfinite(roomDecay_)) {
            return kResultFalse;
        }

        roomDecay_ =
            std::clamp(roomDecay_, 0.0, 1.0);
    } else {
        roomDecay_ = room_;
    }

    if (version >= 5) {
        if (!stream.readDouble(mode_) ||
            !std::isfinite(mode_)) {
            return kResultFalse;
        }

        mode_ = std::clamp(mode_, 0.0, 1.0);
    } else {
        mode_ = 0.0;
    }

    finisher_.setFinish(finish_);
    finisher_.setLowCut(lowCut_);
    finisher_.setRoomWet(room_);
    finisher_.setRoomDecay(roomDecay_);
    finisher_.setMode(mode_);
    finisher_.reset();
    lastBypassed_ = bypass_ >= 0.5;

    return kResultOk;
}

tresult PLUGIN_API Processor::getState(IBStream* state) {
    if (!state)
        return kInvalidArgument;

    IBStreamer stream(state, kLittleEndian);

    if (!stream.writeInt32(kStateVersion))
        return kResultFalse;

    const double values[7] {
        finish_,
        room_,
        output_,
        bypass_,
        lowCut_,
        roomDecay_,
        mode_
    };

    for (const double value : values) {
        if (!stream.writeDouble(value))
            return kResultFalse;
    }

    return kResultOk;
}

} // namespace HighGainGuitarFinisher
