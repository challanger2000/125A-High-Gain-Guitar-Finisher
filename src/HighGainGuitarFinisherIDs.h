#pragma once

#include "pluginterfaces/base/funknown.h"
#include "pluginterfaces/vst/vsttypes.h"

namespace HighGainGuitarFinisher {

static const Steinberg::FUID kProcessorUID(
    0xB486847A, 0x10D64CA2, 0x8F2BD60B, 0x71534ABC);

static const Steinberg::FUID kControllerUID(
    0x07A0454C, 0x4D634C3A, 0xAC40022F, 0xC6C482B0);

enum ParamID : Steinberg::Vst::ParamID {
    kFinish = 100,
    kRoom,
    kOutput,
    kBypass,
    kLowCut80,
    kRoomDecay,
    kMode,
    kMass
};

constexpr Steinberg::int32 kStateVersion = 6;
constexpr Steinberg::int32 kFirstSupportedStateVersion = 1;

} // namespace HighGainGuitarFinisher
