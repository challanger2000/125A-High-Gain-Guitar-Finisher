#pragma once

#include <algorithm>
#include <cmath>
#include <complex>
#include <cstddef>
#include <limits>
#include <numeric>
#include <vector>

namespace HGGFTests {

constexpr double kPi = 3.141592653589793238462643383279502884;

inline double linearToDb(double value) noexcept {
    return value > 0.0
        ? 20.0 * std::log10(value)
        : -std::numeric_limits<double>::infinity();
}

struct StereoMetrics {
    double peak {0.0};
    double rms {0.0};
    double crestDb {0.0};
    double correlation {1.0};
    double midSideDb {-1000.0};
};

inline StereoMetrics measureStereo(const std::vector<double>& left,
                                   const std::vector<double>& right) {
    StereoMetrics result;
    const std::size_t count = std::min(left.size(), right.size());
    if (count == 0)
        return result;

    long double totalSquares = 0.0L;
    long double leftSquares = 0.0L;
    long double rightSquares = 0.0L;
    long double cross = 0.0L;
    long double midSquares = 0.0L;
    long double sideSquares = 0.0L;

    for (std::size_t i = 0; i < count; ++i) {
        const double l = std::isfinite(left[i]) ? left[i] : 0.0;
        const double r = std::isfinite(right[i]) ? right[i] : 0.0;

        result.peak = std::max(result.peak, std::max(std::abs(l), std::abs(r)));
        totalSquares += static_cast<long double>(l) * l;
        totalSquares += static_cast<long double>(r) * r;
        leftSquares += static_cast<long double>(l) * l;
        rightSquares += static_cast<long double>(r) * r;
        cross += static_cast<long double>(l) * r;

        const long double mid = (static_cast<long double>(l) + r) * 0.7071067811865475L;
        const long double side = (static_cast<long double>(l) - r) * 0.7071067811865475L;
        midSquares += mid * mid;
        sideSquares += side * side;
    }

    result.rms = std::sqrt(
        static_cast<double>(totalSquares / static_cast<long double>(count * 2)));

    if (result.rms > 0.0 && result.peak > 0.0)
        result.crestDb = linearToDb(result.peak / result.rms);

    const long double denom = std::sqrt(
        std::max(leftSquares * rightSquares, 1.0e-30L));
    result.correlation = std::clamp(
        static_cast<double>(cross / denom), -1.0, 1.0);

    if (midSquares > 1.0e-30L && sideSquares > 1.0e-30L)
        result.midSideDb = 10.0 * std::log10(
            static_cast<double>(sideSquares / midSquares));
    else if (sideSquares <= 1.0e-30L)
        result.midSideDb = -1000.0;
    else
        result.midSideDb = 1000.0;

    return result;
}

inline double nullPeak(const std::vector<double>& a,
                       const std::vector<double>& b) {
    const std::size_t count = std::min(a.size(), b.size());
    double peak = 0.0;
    for (std::size_t i = 0; i < count; ++i)
        peak = std::max(peak, std::abs(a[i] - b[i]));
    return peak;
}

inline std::vector<double> hannWindow(std::size_t size) {
    std::vector<double> window(size, 1.0);
    if (size < 2)
        return window;

    for (std::size_t i = 0; i < size; ++i) {
        window[i] =
            0.5 - 0.5 * std::cos(
                2.0 * kPi * static_cast<double>(i) /
                static_cast<double>(size - 1));
    }
    return window;
}

inline void fft(std::vector<std::complex<double>>& data) {
    const std::size_t size = data.size();

    for (std::size_t i = 1, j = 0; i < size; ++i) {
        std::size_t bit = size >> 1;
        for (; j & bit; bit >>= 1)
            j ^= bit;
        j ^= bit;
        if (i < j)
            std::swap(data[i], data[j]);
    }

    for (std::size_t len = 2; len <= size; len <<= 1) {
        const double angle = -2.0 * kPi / static_cast<double>(len);
        const std::complex<double> step(std::cos(angle), std::sin(angle));

        for (std::size_t start = 0; start < size; start += len) {
            std::complex<double> phase(1.0, 0.0);
            for (std::size_t j = 0; j < len / 2; ++j) {
                const auto even = data[start + j];
                const auto odd = data[start + j + len / 2] * phase;
                data[start + j] = even + odd;
                data[start + j + len / 2] = even - odd;
                phase *= step;
            }
        }
    }
}

struct TonalBands {
    double sub20To80 {0.0};
    double low80To250 {0.0};
    double body250To500 {0.0};
    double mids500To2000 {0.0};
    double upperMids2000To5000 {0.0};
    double presence5000To8000 {0.0};
    double air8000To12000 {0.0};
    double top12000To20000 {0.0};
};

inline TonalBands measureTonalBands(const std::vector<double>& mono,
                                    double sampleRate) {
    constexpr std::size_t fftSize = 4096;
    constexpr std::size_t hop = fftSize / 2;

    TonalBands bands;
    if (mono.size() < fftSize)
        return bands;

    const auto window = hannWindow(fftSize);
    long double energies[8] {};
    std::size_t frames = 0;

    for (std::size_t start = 0; start + fftSize <= mono.size(); start += hop) {
        std::vector<std::complex<double>> spectrum(fftSize);
        for (std::size_t i = 0; i < fftSize; ++i)
            spectrum[i] = mono[start + i] * window[i];

        fft(spectrum);

        for (std::size_t bin = 1; bin <= fftSize / 2; ++bin) {
            const double frequency =
                static_cast<double>(bin) * sampleRate /
                static_cast<double>(fftSize);

            if (frequency < 20.0 || frequency > 20000.0)
                continue;

            const long double energy =
                std::norm(spectrum[bin]);

            std::size_t index = 0;
            if (frequency < 80.0) index = 0;
            else if (frequency < 250.0) index = 1;
            else if (frequency < 500.0) index = 2;
            else if (frequency < 2000.0) index = 3;
            else if (frequency < 5000.0) index = 4;
            else if (frequency < 8000.0) index = 5;
            else if (frequency < 12000.0) index = 6;
            else index = 7;

            energies[index] += energy;
        }

        ++frames;
    }

    if (frames == 0)
        return bands;

    auto toValue = [frames](long double energy) {
        return static_cast<double>(energy / static_cast<long double>(frames));
    };

    bands.sub20To80 = toValue(energies[0]);
    bands.low80To250 = toValue(energies[1]);
    bands.body250To500 = toValue(energies[2]);
    bands.mids500To2000 = toValue(energies[3]);
    bands.upperMids2000To5000 = toValue(energies[4]);
    bands.presence5000To8000 = toValue(energies[5]);
    bands.air8000To12000 = toValue(energies[6]);
    bands.top12000To20000 = toValue(energies[7]);
    return bands;
}

inline double energyDeltaDb(double inputEnergy,
                            double outputEnergy) noexcept {
    if (inputEnergy <= 0.0 || outputEnergy <= 0.0)
        return -1000.0;
    return 10.0 * std::log10(outputEnergy / inputEnergy);
}

} // namespace HGGFTests
