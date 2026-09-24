#include "support/TestSupport.h"
#include "MetalFinisherDSP.h"
#include "LowCutMapping.h"

#include <cmath>
#include <cstddef>
#include <cstdlib>
#include <new>

#if defined(_WIN32)
#include <malloc.h>
#endif

using HighGainGuitarFinisher::dsp::MetalFinisherDSP;
using HighGainGuitarFinisher::dsp::lowCutNormalizedFromFrequency;

namespace {

bool gTrackAllocations = false;
std::size_t gAllocationCount = 0;

void noteAllocation() noexcept {
    if (gTrackAllocations)
        ++gAllocationCount;
}

void* allocateRaw(std::size_t size) {
    noteAllocation();

    if (void* p = std::malloc(size))
        return p;

    throw std::bad_alloc {};
}

#if defined(__cpp_aligned_new)
void* allocateAligned(
    std::size_t size,
    std::size_t alignment) {

    noteAllocation();

#if defined(_WIN32)
    if (void* p =
            _aligned_malloc(
                size,
                alignment)) {
        return p;
    }
#else
    void* p = nullptr;

    if (posix_memalign(
            &p,
            alignment,
            size) == 0) {
        return p;
    }
#endif

    throw std::bad_alloc {};
}
#endif

} // namespace

void* operator new(std::size_t size) {
    return allocateRaw(size);
}

void* operator new[](std::size_t size) {
    return allocateRaw(size);
}

void operator delete(void* pointer) noexcept {
    std::free(pointer);
}

void operator delete[](void* pointer) noexcept {
    std::free(pointer);
}

void operator delete(
    void* pointer,
    std::size_t) noexcept {

    std::free(pointer);
}

void operator delete[](
    void* pointer,
    std::size_t) noexcept {

    std::free(pointer);
}

#if defined(__cpp_aligned_new)
void* operator new(
    std::size_t size,
    std::align_val_t alignment) {

    return allocateAligned(
        size,
        static_cast<std::size_t>(
            alignment));
}

void* operator new[](
    std::size_t size,
    std::align_val_t alignment) {

    return allocateAligned(
        size,
        static_cast<std::size_t>(
            alignment));
}

void operator delete(
    void* pointer,
    std::align_val_t) noexcept {

#if defined(_WIN32)
    _aligned_free(pointer);
#else
    std::free(pointer);
#endif
}

void operator delete[](
    void* pointer,
    std::align_val_t) noexcept {

#if defined(_WIN32)
    _aligned_free(pointer);
#else
    std::free(pointer);
#endif
}

void operator delete(
    void* pointer,
    std::size_t,
    std::align_val_t) noexcept {

#if defined(_WIN32)
    _aligned_free(pointer);
#else
    std::free(pointer);
#endif
}

void operator delete[](
    void* pointer,
    std::size_t,
    std::align_val_t) noexcept {

#if defined(_WIN32)
    _aligned_free(pointer);
#else
    std::free(pointer);
#endif
}
#endif

int main() {
    constexpr double sampleRate = 48000.0;
    constexpr double pi =
        3.141592653589793238462643383279502884;

    MetalFinisherDSP dsp;

    // Setup is explicitly outside the realtime allocation window.
    dsp.prepare(sampleRate);
    dsp.setFinish(1.0);
    dsp.setMass(1.0);
    dsp.setLowCut(
        lowCutNormalizedFromFrequency(
            92.0));
    dsp.setRoomWet(1.0);
    dsp.setRoomDecay(0.83);
    dsp.setMode(0.5);

    // Warm up all coefficient-update and state paths before measuring.
    for (int i = 0; i < 4096; ++i) {
        const double t =
            static_cast<double>(i) /
            sampleRate;

        double left =
            0.35 * std::sin(
                2.0 * pi * 113.0 * t) +
            0.21 * std::sin(
                2.0 * pi * 3900.0 * t);

        double right =
            0.33 * std::sin(
                2.0 * pi * 127.0 * t) +
            0.20 * std::sin(
                2.0 * pi * 6100.0 * t);

        dsp.processFrame(
            left,
            right);
    }

    gAllocationCount = 0;
    gTrackAllocations = true;

    double checksum = 0.0;

    for (int i = 0; i < 48000; ++i) {
        const double t =
            static_cast<double>(i) /
            sampleRate;

        const double pulse =
            std::fmod(t, 0.25) < 0.070
                ? 1.0
                : 0.32;

        double left =
            pulse * (
                0.30 * std::sin(
                    2.0 * pi * 105.0 * t) +
                0.18 * std::sin(
                    2.0 * pi * 335.0 * t)) +
            0.22 * std::sin(
                2.0 * pi * 1300.0 * t) +
            0.16 * std::sin(
                2.0 * pi * 4300.0 * t);

        double right =
            pulse * (
                0.28 * std::sin(
                    2.0 * pi * 119.0 * t) +
                0.17 * std::sin(
                    2.0 * pi * 370.0 * t)) +
            0.21 * std::sin(
                2.0 * pi * 1450.0 * t) +
            0.15 * std::sin(
                2.0 * pi * 6200.0 * t);

        dsp.processFrame(
            left,
            right);

        checksum +=
            left * 0.5 +
            right * 0.5;
    }

    gTrackAllocations = false;

    HGGF_REQUIRE(
        std::isfinite(checksum));

    HGGF_REQUIRE(
        gAllocationCount == 0);

    return 0;
}
