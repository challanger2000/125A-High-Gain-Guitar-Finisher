#pragma once

#include "public.sdk/source/vst/vstaudioeffect.h"

namespace HighGainGuitarFinisher {

class Processor : public Steinberg::Vst::AudioEffect {
public:
    Processor();

    static Steinberg::FUnknown* createInstance(void*) {
        return static_cast<Steinberg::Vst::IAudioProcessor*>(new Processor());
    }

    Steinberg::tresult PLUGIN_API initialize(Steinberg::FUnknown* context) override;

    Steinberg::tresult PLUGIN_API setBusArrangements(
        Steinberg::Vst::SpeakerArrangement* inputs,
        Steinberg::int32 numIns,
        Steinberg::Vst::SpeakerArrangement* outputs,
        Steinberg::int32 numOuts) override;

    Steinberg::tresult PLUGIN_API canProcessSampleSize(
        Steinberg::int32 symbolicSampleSize) override;

    Steinberg::tresult PLUGIN_API process(
        Steinberg::Vst::ProcessData& data) override;

    Steinberg::tresult PLUGIN_API setState(
        Steinberg::IBStream* state) override;

    Steinberg::tresult PLUGIN_API getState(
        Steinberg::IBStream* state) override;

private:
    void readParameterChanges(Steinberg::Vst::IParameterChanges* changes);

    template <typename Sample>
    void processBlock(Sample** inputs,
                      Sample** outputs,
                      Steinberg::int32 numSamples,
                      Steinberg::int32 numChannels);

    double finish_ {0.0};
    double room_ {0.0};
    double output_ {0.5}; // normalized: -12 dB .. +12 dB, 0.5 = 0 dB
    double bypass_ {0.0};
};

} // namespace HighGainGuitarFinisher
