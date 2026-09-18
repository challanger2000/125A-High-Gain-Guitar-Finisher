# Architecture

## Design goals

The project is intentionally split into a thin VST3 integration layer and reusable DSP code.

- `src/HighGainGuitarFinisherProcessor.*` owns VST3 audio I/O, automation/state handoff and lifecycle.
- `src/HighGainGuitarFinisherController.*` owns public parameters and controller state.
- `src/dsp/` contains audio algorithms with no dependency on the VST3 SDK.
- `tests/` validates DSP behaviour independently from a DAW.
- `docs/` records design intent and engineering decisions.

## Current signal path

`Stereo In -> FINISH static cleanup -> FINISH dynamic low-end control -> ROOM (neutral placeholder) -> OUTPUT -> Stereo Out`

BYPASS skips intentional processing and output trim so bypass remains unity.

## FINISH Stage 1 — static cleanup

The static stage uses one fully designed wet branch:

1. second-order high-pass at 85 Hz,
2. broad peaking cut centred at 300 Hz at -4 dB,
3. dry/wet interpolation controlled by FINISH.

Keeping the filter coefficients fixed avoids coefficient jumps during FINISH automation. `FINISH = 0` is exactly transparent.

## FINISH Stage 2 — dynamic low-end / palm-mute control

The dynamic stage is intentionally level-independent:

1. a stereo detector isolates the low band below roughly 180 Hz,
2. low-band and broadband envelopes are compared,
3. low-frequency dominance drives a bounded reduction,
4. one shared reduction value is applied to both channels,
5. attack/release smoothing prevents abrupt gain changes.

The processor subtracts only part of the detected low-band component rather than turning the whole signal down. This is intended to restrain palm-mute thump without collapsing the useful guitar midrange or destabilizing stereo balance.

## Real-time rules

The audio callback must not allocate memory, lock a mutex, access files, log, or perform GUI work.

DSP objects are prepared/reset from the VST3 lifecycle. Runtime processing uses preallocated state only.

## State compatibility

The processor state begins with `kStateVersion`. Public test builds must not silently reorder or reinterpret existing serialized values. Any incompatible state change requires an explicit version migration.

## Validation

The standalone DSP smoke test checks:

- exact transparency at `FINISH = 0`,
- finite stereo output,
- expected attenuation around 80 Hz,
- expected attenuation around 300 Hz,
- preservation of the useful midrange around 1 kHz,
- activation of dynamic low-end control on sustained low-frequency material,
- release/recovery when the input moves back into the guitar midrange.

DAW/host validation remains a separate layer and must be performed on built VST3 bundles.
