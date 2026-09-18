#include "MetalFinisherDSP.h"

#include <cassert>
#include <cmath>
#include <iostream>

using HighGainGuitarFinisher::dsp::MetalFinisherDSP;

namespace {

constexpr double pi = 3.141592653589793238462643383279502884;
constexpr double sampleRate = 48000.0;

double measureGain(double frequency, double finish) {
    constexpr int warmup = 24000;
    constexpr int measured = 48000;

    MetalFinisherDSP dsp;
    dsp.prepare(sampleRate);
    dsp.setFinish(finish);

    double inputPower = 0.0;
    double outputPower = 0.0;

    for (int i = 0; i < warmup + measured; ++i) {
        const double x =
            std::sin(2.0 * pi * frequency * i / sampleRate);

        double left = x;
        double right = x;
        dsp.processFrame(left, right);

        assert(std::isfinite(left));
        assert(std::isfinite(right));

        if (i >= warmup) {
            inputPower += x * x;
            outputPower += left * left;
        }
    }

    return std::sqrt(outputPower / inputPower);
}

void verifyExactTransparency() {
    MetalFinisherDSP dsp;
    dsp.prepare(sampleRate);
    dsp.setFinish(0.0);

    for (int i = 0; i < 1000; ++i) {
        const double x =
            std::sin(2.0 * pi * 440.0 * i / sampleRate);

        double left = x;
        double right = -x;
        const double originalLeft = left;
        const double originalRight = right;

        dsp.processFrame(left, right);

        assert(left == originalLeft);
        assert(right == originalRight);
    }
}

void verifyDynamicLowEndControl() {
    MetalFinisherDSP dsp;
    dsp.prepare(sampleRate);
    dsp.setFinish(1.0);

    for (int i = 0; i < static_cast<int>(sampleRate); ++i) {
        const double x =
            0.8 * std::sin(2.0 * pi * 100.0 * i / sampleRate);

        double left = x;
        double right = x;
        dsp.processFrame(left, right);
    }

    const double activeReduction =
        dsp.currentDynamicLowEndReduction();

    assert(activeReduction > 0.30);
    assert(activeReduction <= 0.451);

    for (int i = 0; i < static_cast<int>(sampleRate); ++i) {
        const double x =
            0.5 * std::sin(2.0 * pi * 2000.0 * i / sampleRate);

        double left = x;
        double right = x;
        dsp.processFrame(left, right);
    }

    const double recoveredReduction =
        dsp.currentDynamicLowEndReduction();

    assert(recoveredReduction < 0.01);
}

void verifyDynamicHarshnessControl() {
    MetalFinisherDSP dsp;
    dsp.prepare(sampleRate);
    dsp.setFinish(1.0);

    for (int i = 0; i < static_cast<int>(sampleRate); ++i) {
        const double x =
            0.6 * std::sin(2.0 * pi * 4800.0 * i / sampleRate);

        double left = x;
        double right = x;
        dsp.processFrame(left, right);
    }

    const double activeReduction =
        dsp.currentHarshnessReduction();

    assert(activeReduction > 0.20);
    assert(activeReduction <= 0.251);

    for (int i = 0; i < static_cast<int>(sampleRate); ++i) {
        const double x =
            0.5 * std::sin(2.0 * pi * 1000.0 * i / sampleRate);

        double left = x;
        double right = x;
        dsp.processFrame(left, right);
    }

    const double recoveredReduction =
        dsp.currentHarshnessReduction();

    assert(recoveredReduction < 0.01);
}

} // namespace

int main() {
    verifyExactTransparency();
    verifyDynamicLowEndControl();
    verifyDynamicHarshnessControl();

    const double gain80 = measureGain(80.0, 1.0);
    const double gain300 = measureGain(300.0, 1.0);
    const double gain1000 = measureGain(1000.0, 1.0);
    const double gain4800 = measureGain(4800.0, 1.0);
    const double gain8000 = measureGain(8000.0, 1.0);

    assert(gain80 < 0.55);
    assert(gain300 < 0.75);
    assert(gain1000 > 0.90);
    assert(gain1000 < 1.05);
    assert(gain4800 < 0.80);
    assert(gain8000 > 0.85);

    std::cout
        << "DSP smoke test passed\n"
        << "80 Hz gain: " << gain80 << "\n"
        << "300 Hz gain: " << gain300 << "\n"
        << "1 kHz gain: " << gain1000 << "\n"
        << "4.8 kHz gain: " << gain4800 << "\n"
        << "8 kHz gain: " << gain8000 << "\n";

    return 0;
}
