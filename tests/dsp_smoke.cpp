#include "MetalFinisherDSP.h"

#include <cassert>
#include <cmath>
#include <iostream>

using HighGainGuitarFinisher::dsp::MetalFinisherDSP;

namespace {
constexpr double pi = 3.141592653589793238462643383279502884;

double measureGain(double frequency, double finish) {
    constexpr double sampleRate = 48000.0;
    constexpr int warmup = 24000;
    constexpr int measured = 48000;

    MetalFinisherDSP dsp;
    dsp.prepare(sampleRate);
    dsp.setFinish(finish);

    double inPower = 0.0;
    double outPower = 0.0;

    for (int i = 0; i < warmup + measured; ++i) {
        const double x = std::sin(2.0 * pi * frequency * i / sampleRate);
        const double y = dsp.processSample(0, x);
        assert(std::isfinite(y));

        if (i >= warmup) {
            inPower += x * x;
            outPower += y * y;
        }
    }

    return std::sqrt(outPower / inPower);
}
}

int main() {
    MetalFinisherDSP dsp;
    dsp.prepare(48000.0);
    dsp.setFinish(0.0);

    for (int i = 0; i < 1000; ++i) {
        const double x = std::sin(2.0 * pi * 440.0 * i / 48000.0);
        assert(dsp.processSample(0, x) == x);
    }

    const double gain80 = measureGain(80.0, 1.0);
    const double gain300 = measureGain(300.0, 1.0);
    const double gain1000 = measureGain(1000.0, 1.0);

    assert(gain80 < 0.75);
    assert(gain300 < 0.75);
    assert(gain1000 > 0.90);
    assert(gain1000 < 1.05);

    std::cout << "DSP smoke test passed\n"
              << "80 Hz gain: " << gain80 << "\n"
              << "300 Hz gain: " << gain300 << "\n"
              << "1 kHz gain: " << gain1000 << "\n";
}
