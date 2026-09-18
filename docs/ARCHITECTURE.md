# Architecture

## Design goals

The project is intentionally split into a thin VST3 integration layer and reusable DSP code.

- `src/HighGainGuitarFinisherProcessor.*` owns VST3 audio I/O, automation/state handoff and lifecycle.
- `src/HighGainGuitarFinisherController.*` owns public parameters and controller state.
- `src/dsp/` contains audio algorithms with no dependency on the VST3 SDK.
- `tests/` validates DSP behaviour independently from a DAW.
- `docs/` records design intent and engineering decisions.

## Current signal path

`Stereo In -> static cleanup -> dynamic low-end control -> dynamic harshness control -> ROOM (neutral placeholder) -> OUTPUT -> Stereo Out`

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

The processor subtracts only part of the detected low-band component rather than turning the whole signal down.

## FINISH Stage 3 — dynamic harshness control

The harshness stage follows the same conservative architecture:

1. a broad band-pass is centred around 4.8 kHz,
2. band energy is compared with broadband energy,
3. only excessive upper-mid dominance activates reduction,
4. maximum internal band subtraction is bounded at 25%,
5. the gain attack is deliberately slower than the detector attack to preserve pick definition,
6. one shared stereo reduction value keeps double-track balance stable.

The stage is intended to reduce amp-sim harshness/fizz without applying a permanent low-pass filter or removing the useful guitar midrange.

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
- controlled attenuation around 4.8 kHz,
- preservation of upper treble around 8 kHz,
- activation and release of both dynamic controllers.

DAW/host validation remains a separate layer and must be performed on built VST3 bundles.
