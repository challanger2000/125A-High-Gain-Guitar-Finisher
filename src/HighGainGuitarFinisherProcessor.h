#pragma once

#include "public.sdk/source/vst/vstaudioeffect.h"
#include "dsp/MetalFinisherDSP.h"

namespace HighGainGuitarFinisher {

class Processor : public Steinberg::Vst::AudioEffect {
public:
    Processor();

    static Steinberg::FUnknown* createInstance(void*) {
        return static_cast<Steinberg::Vst::IAudioProcessor*>(new Processor());
    }

    Steinberg::tresult PLUGIN_API initialize(
        Steinberg::FUnknown* context) override;

    Steinberg::tresult PLUGIN_API setBusArrangements(
        Steinberg::Vst::SpeakerArrangement* inputs,
        Steinberg::int32 numIns,
        Steinberg::Vst::SpeakerArrangement* outputs,
        Steinberg::int32 numOuts) override;

    Steinberg::tresult PLUGIN_API canProcessSampleSize(
        Steinberg::int32 symbolicSampleSize) override;

    Steinberg::tresult PLUGIN_API setupProcessing(
        Steinberg::Vst::ProcessSetup& setup) override;

    Steinberg::tresult PLUGIN_API setActive(
        Steinberg::TBool state) override;

    Steinberg::tresult PLUGIN_API setProcessing(
        Steinberg::TBool state) override;

    Steinberg::tresult PLUGIN_API process(
        Steinberg::Vst::ProcessData& data) override;

    Steinberg::tresult PLUGIN_API setState(
        Steinberg::IBStream* state) override;

    Steinberg::tresult PLUGIN_API getState(
        Steinberg::IBStream* state) override;

private:
    void readParameterChanges(
        Steinberg::Vst::IParameterChanges* changes);

    template <typename Sample>
    void processBlock(Sample** inputs,
                      Sample** outputs,
                      Steinberg::int32 numSamples,
                      Steinberg::int32 numChannels);

    dsp::MetalFinisherDSP finisher_ {};

    double sampleRate_ {44100.0};
    double finish_ {0.0};
    double room_ {0.0};
    double output_ {0.5};
    double bypass_ {0.0};
    double lowCut80_ {0.0};

    bool lastBypassed_ {false};
};

} // namespace HighGainGuitarFinisher
