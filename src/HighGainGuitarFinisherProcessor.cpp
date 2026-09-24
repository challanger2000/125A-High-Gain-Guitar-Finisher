#include "HighGainGuitarFinisherProcessor.h"
#include "HighGainGuitarFinisherIDs.h"
#include "dsp/LowCutMapping.h"
#include "AutomationMath.h"

#include "base/source/fstreamer.h"
#include "pluginterfaces/vst/vstspeaker.h"

#include <algorithm>
#include <cmath>
#include <limits>

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

    if (numIns != 1 ||
        numOuts != 1) {
        return kResultFalse;
    }

    const bool mono =
        inputs[0] == SpeakerArr::kMono &&
        outputs[0] == SpeakerArr::kMono;

    const bool stereo =
        inputs[0] == SpeakerArr::kStereo &&
        outputs[0] == SpeakerArr::kStereo;

    if (!mono && !stereo)
        return kResultFalse;

    return AudioEffect::setBusArrangements(
        inputs,
        numIns,
        outputs,
        numOuts);
}

tresult PLUGIN_API Processor::canProcessSampleSize(int32 symbolicSampleSize) {
    return (symbolicSampleSize == kSample32 || symbolicSampleSize == kSample64)
        ? kResultTrue
        : kResultFalse;
}

uint32 PLUGIN_API Processor::getTailSamples() {
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

    try {
        finisher_.prepare(sampleRate_);
    } catch (...) {
        return kResultFalse;
    }

    syncDSPParameters();

    const bool bypassed =
        bypass_ >= 0.5;

    bypassCrossfade_.prepare(
        sampleRate_,
        bypassed);

    lastBypassed_ = bypassed;
    bypassDSPDormant_ = bypassed;

    return AudioEffect::setupProcessing(setup);
}

tresult PLUGIN_API Processor::setActive(TBool state) {
    if (state) {
        finisher_.reset();

        const bool bypassed =
            bypass_ >= 0.5;

        bypassCrossfade_.reset(
            bypassed);

        lastBypassed_ = bypassed;
        bypassDSPDormant_ = bypassed;
    }

    return AudioEffect::setActive(state);
}

tresult PLUGIN_API Processor::setProcessing(TBool state) {
    if (state) {
        finisher_.reset();

        const bool bypassed =
            bypass_ >= 0.5;

        bypassCrossfade_.reset(
            bypassed);

        lastBypassed_ = bypassed;
        bypassDSPDormant_ = bypassed;
    }

    AudioEffect::setProcessing(state);
    return kResultTrue;
}

ParamValue Processor::currentParameterValue(
    Steinberg::Vst::ParamID id) const noexcept {

    switch (id) {
        case kFinish:    return finish_;
        case kRoom:      return room_;
        case kRoomDecay: return roomDecay_;
        case kOutput:    return output_;
        case kBypass:    return bypass_;
        case kLowCut80:  return lowCut_;
        case kMode:      return mode_;
        case kMass:      return mass_;
        default:         return 0.0;
    }
}

void Processor::applyParameterValue(
    Steinberg::Vst::ParamID id,
    ParamValue value) noexcept {

    if (!std::isfinite(value))
        return;

    value = std::clamp(value, 0.0, 1.0);

    switch (id) {
        case kFinish:    finish_ = value; break;
        case kRoom:      room_ = value; break;
        case kRoomDecay: roomDecay_ = value; break;
        case kOutput:    output_ = value; break;
        case kBypass:    bypass_ = value; break;
        case kLowCut80:  lowCut_ = value; break;
        case kMode:      mode_ = value; break;
        case kMass:      mass_ = value; break;
        default: break;
    }
}

void Processor::readLastParameterChanges(
    IParameterChanges* changes) noexcept {

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
                value) == kResultTrue) {
            applyParameterValue(
                queue->getParameterId(),
                value);
        }
    }
}

void Processor::initializeAutomationCursors(
    IParameterChanges* changes,
    std::array<AutomationCursor, kAutomatedParameterCount>& cursors) noexcept {

    for (auto& cursor : cursors)
        cursor = {};

    if (!changes)
        return;

    std::size_t cursorIndex = 0;

    for (int32 i = 0;
         i < changes->getParameterCount() &&
         cursorIndex < cursors.size();
         ++i) {

        auto* queue =
            changes->getParameterData(i);

        if (!queue ||
            queue->getPointCount() <= 0) {
            continue;
        }

        const Steinberg::Vst::ParamID id =
            queue->getParameterId();

        switch (id) {
            case kFinish:
            case kRoom:
            case kRoomDecay:
            case kOutput:
            case kBypass:
            case kLowCut80:
            case kMode:
            case kMass:
                break;
            default:
                continue;
        }

        auto& cursor =
            cursors[cursorIndex++];

        cursor.queue = queue;
        cursor.pointCount =
            queue->getPointCount();
        cursor.id = id;
        cursor.discrete =
            id == kBypass ||
            id == kMode;
        cursor.segmentStartOffset = -1;
        cursor.segmentStartValue =
            currentParameterValue(id);

        int32 offset = 0;
        ParamValue value = 0.0;

        if (queue->getPoint(
                0,
                offset,
                value) == kResultTrue) {
            cursor.nextSampleOffset = offset;
            cursor.nextValue = value;
            cursor.hasNext = true;
        }
    }
}

bool Processor::applyAutomationAtSample(
    std::array<AutomationCursor, kAutomatedParameterCount>& cursors,
    int32 sampleOffset,
    bool& outputPathChanged) noexcept {

    bool changed = false;
    outputPathChanged = false;

    const auto markChanged =
        [&](Steinberg::Vst::ParamID id) noexcept {
            changed = true;

            if (id == kOutput ||
                id == kBypass) {
                outputPathChanged = true;
            }
        };

    const auto advancePoint =
        [](AutomationCursor& cursor) noexcept {

            ++cursor.pointIndex;

            if (cursor.pointIndex >=
                cursor.pointCount) {
                cursor.hasNext = false;
                return;
            }

            int32 nextOffset = 0;
            ParamValue nextValue = 0.0;

            if (cursor.queue->getPoint(
                    cursor.pointIndex,
                    nextOffset,
                    nextValue) != kResultTrue) {
                cursor.hasNext = false;
                return;
            }

            if (nextOffset <
                cursor.nextSampleOffset) {
                cursor.hasNext = false;
                return;
            }

            cursor.nextSampleOffset =
                nextOffset;
            cursor.nextValue =
                nextValue;
        };

    for (auto& cursor : cursors) {
        if (!cursor.hasNext)
            continue;

        if (cursor.discrete) {
            while (cursor.hasNext &&
                   cursor.nextSampleOffset <=
                       sampleOffset) {

                applyParameterValue(
                    cursor.id,
                    cursor.nextValue);

                markChanged(
                    cursor.id);

                cursor.segmentStartOffset =
                    cursor.nextSampleOffset;

                cursor.segmentStartValue =
                    std::clamp(
                        std::isfinite(
                            cursor.nextValue)
                            ? cursor.nextValue
                            : cursor.segmentStartValue,
                        0.0,
                        1.0);

                advancePoint(
                    cursor);
            }

            continue;
        }

        while (cursor.hasNext &&
               cursor.nextSampleOffset <=
                   sampleOffset) {

            applyParameterValue(
                cursor.id,
                cursor.nextValue);

            markChanged(
                cursor.id);

            cursor.segmentStartOffset =
                cursor.nextSampleOffset;

            cursor.segmentStartValue =
                std::clamp(
                    std::isfinite(
                        cursor.nextValue)
                        ? cursor.nextValue
                        : cursor.segmentStartValue,
                    0.0,
                    1.0);

            advancePoint(
                cursor);
        }

        if (!cursor.hasNext)
            continue;

        const int32 span =
            cursor.nextSampleOffset -
            cursor.segmentStartOffset;

        if (span <= 0)
            continue;

        const ParamValue value =
            automation::linearValueAtSample(
                sampleOffset,
                cursor.segmentStartOffset,
                cursor.segmentStartValue,
                cursor.nextSampleOffset,
                cursor.nextValue);

        applyParameterValue(
            cursor.id,
            value);

        markChanged(
            cursor.id);
    }

    return changed;
}

void Processor::syncDSPParameters() noexcept {
    finisher_.setFinish(finish_);
    finisher_.setLowCut(lowCut_);
    finisher_.setRoomWet(room_);
    finisher_.setRoomDecay(roomDecay_);
    finisher_.setMode(mode_);
    finisher_.setMass(mass_);
}

template <typename Sample>
uint64 Processor::processBlock(
    Sample** inputs,
    Sample** outputs,
    int32 numSamples,
    int32 numChannels,
    IParameterChanges* parameterChanges) {

    std::array<
        AutomationCursor,
        kAutomatedParameterCount> cursors {};

    initializeAutomationCursors(
        parameterChanges,
        cursors);

    bool bypassed =
        bypass_ >= 0.5;

    if (bypassed != lastBypassed_) {
        if (!bypassed) {
            finisher_.reset();
            bypassDSPDormant_ = false;
        }

        bypassCrossfade_.setBypassed(
            bypassed);

        lastBypassed_ = bypassed;
    }

    syncDSPParameters();

    double activeOutputGain =
        std::pow(
            10.0,
            ((output_ * 24.0) - 12.0) /
                20.0);

    const Sample* inputLeft =
        inputs ? inputs[0] : nullptr;

    const Sample* inputRight =
        inputs
            ? (numChannels > 1
                ? inputs[1]
                : inputs[0])
            : nullptr;

    Sample* outputLeft =
        outputs ? outputs[0] : nullptr;

    Sample* outputRight =
        outputs && numChannels > 1
            ? outputs[1]
            : nullptr;

    bool leftSilent = true;
    bool rightSilent = true;

    for (int32 sample = 0;
         sample < numSamples;
         ++sample) {

        bool outputPathChanged = false;

        if (applyAutomationAtSample(
                cursors,
                sample,
                outputPathChanged)) {

            syncDSPParameters();

            if (outputPathChanged) {
                const bool nextBypassed =
                    bypass_ >= 0.5;

                if (nextBypassed !=
                    lastBypassed_) {

                    if (!nextBypassed) {
                        finisher_.reset();
                        bypassDSPDormant_ =
                            false;
                    }

                    bypassCrossfade_.
                        setBypassed(
                            nextBypassed);

                    lastBypassed_ =
                        nextBypassed;
                }

                bypassed =
                    nextBypassed;

                activeOutputGain =
                    std::pow(
                        10.0,
                        ((output_ * 24.0) -
                         12.0) /
                            20.0);
            }
        }

        if (!outputLeft)
            continue;

        double left =
            inputLeft
                ? static_cast<double>(
                    inputLeft[sample])
                : 0.0;

        double right =
            inputRight
                ? static_cast<double>(
                    inputRight[sample])
                : left;

        if (!std::isfinite(left))
            left = 0.0;
        if (!std::isfinite(right))
            right = 0.0;

        const double dryLeft = left;
        const double dryRight = right;

        const bool processingNeeded =
            !bypassDSPDormant_ ||
            !bypassed ||
            !bypassCrossfade_.
                fullyBypassed();

        if (processingNeeded) {
            finisher_.processFrame(
                left,
                right);
        }

        left *= activeOutputGain;
        right *= activeOutputGain;

        const double bypassMix =
            bypassCrossfade_.advance();

        if (bypassMix >= 1.0) {
            left = dryLeft;
            right = dryRight;
        } else if (bypassMix > 0.0) {
            left +=
                (dryLeft - left) *
                bypassMix;

            right +=
                (dryRight - right) *
                bypassMix;
        }

        if (bypassed &&
            bypassCrossfade_.
                fullyBypassed() &&
            !bypassDSPDormant_) {

            finisher_.reset();
            bypassDSPDormant_ = true;
        }

        if (!outputRight) {
            // The DSP deliberately runs its spatial room as a stereo field.
            // For Mono->Mono, collapse that complete field instead of
            // discarding the right-side reflections.
            left =
                0.5 * (
                    left +
                    right);
        }

        if (!std::isfinite(left))
            left = 0.0;
        if (!std::isfinite(right))
            right = 0.0;

        outputLeft[sample] =
            static_cast<Sample>(left);

        if (outputLeft[sample] !=
            static_cast<Sample>(0)) {
            leftSilent = false;
        }

        if (outputRight) {
            outputRight[sample] =
                static_cast<Sample>(right);

            if (outputRight[sample] !=
                static_cast<Sample>(0)) {
                rightSilent = false;
            }
        }
    }

    uint64 silenceFlags = 0;

    if (leftSilent)
        silenceFlags |= uint64 {1};

    if (numChannels > 1 &&
        rightSilent) {
        silenceFlags |= uint64 {2};
    }

    return silenceFlags;
}

tresult PLUGIN_API Processor::process(ProcessData& data) {
    if (data.numInputs == 0 ||
        data.numOutputs == 0 ||
        data.numSamples <= 0) {

        readLastParameterChanges(
            data.inputParameterChanges);

        syncDSPParameters();

        const bool bypassed =
            bypass_ >= 0.5;

        bypassCrossfade_.reset(
            bypassed);

        lastBypassed_ = bypassed;
        bypassDSPDormant_ = bypassed;

        return kResultOk;
    }

    const int32 numChannels = std::min<int32>(
        2,
        std::min(
            data.inputs[0].numChannels,
            data.outputs[0].numChannels));

    if (numChannels <= 0) {
        readLastParameterChanges(
            data.inputParameterChanges);
        syncDSPParameters();

        const bool bypassed =
            bypass_ >= 0.5;

        bypassCrossfade_.reset(
            bypassed);

        lastBypassed_ = bypassed;
        bypassDSPDormant_ = bypassed;

        return kResultOk;
    }

    uint64 silenceFlags = 0;

    if (data.symbolicSampleSize == kSample32) {
        silenceFlags =
            processBlock(
                data.inputs[0].channelBuffers32,
                data.outputs[0].channelBuffers32,
                data.numSamples,
                numChannels,
                data.inputParameterChanges);
    } else if (data.symbolicSampleSize == kSample64) {
        silenceFlags =
            processBlock(
                data.inputs[0].channelBuffers64,
                data.outputs[0].channelBuffers64,
                data.numSamples,
                numChannels,
                data.inputParameterChanges);
    } else {
        return kResultFalse;
    }

    data.outputs[0].silenceFlags =
        silenceFlags;

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
        if (!stream.readDouble(value) ||
            !std::isfinite(value))
            return kResultFalse;

        value =
            std::clamp(
                value,
                0.0,
                1.0);
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
                ? std::clamp(
                    savedLowCut,
                    0.0,
                    1.0)
                : (savedLowCut >= 0.5
                    ? dsp::lowCutNormalizedFromFrequency(
                        80.0)
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
            std::clamp(
                roomDecay_,
                0.0,
                1.0);
    } else {
        roomDecay_ = room_;
    }

    if (version >= 5) {
        if (!stream.readDouble(mode_) ||
            !std::isfinite(mode_)) {
            return kResultFalse;
        }

        mode_ =
            std::clamp(
                mode_,
                0.0,
                1.0);
    } else {
        mode_ = 0.0;
    }

    if (version >= 6) {
        if (!stream.readDouble(mass_) ||
            !std::isfinite(mass_)) {
            return kResultFalse;
        }

        mass_ =
            std::clamp(
                mass_,
                0.0,
                1.0);
    } else {
        mass_ = 0.0;
    }

    syncDSPParameters();
    finisher_.reset();

    const bool bypassed =
        bypass_ >= 0.5;

    bypassCrossfade_.prepare(
        sampleRate_,
        bypassed);

    lastBypassed_ = bypassed;
    bypassDSPDormant_ = bypassed;

    return kResultOk;
}

tresult PLUGIN_API Processor::getState(IBStream* state) {
    if (!state)
        return kInvalidArgument;

    IBStreamer stream(state, kLittleEndian);

    if (!stream.writeInt32(kStateVersion))
        return kResultFalse;

    const double values[8] {
        finish_,
        room_,
        output_,
        bypass_,
        lowCut_,
        roomDecay_,
        mode_,
        mass_
    };

    for (const double value : values) {
        if (!stream.writeDouble(value))
            return kResultFalse;
    }

    return kResultOk;
}

} // namespace HighGainGuitarFinisher
