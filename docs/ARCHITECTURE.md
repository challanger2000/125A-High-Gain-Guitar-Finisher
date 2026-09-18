# Architecture

## Design goals

The project is intentionally split into a thin VST3 integration layer and reusable DSP code.

- `src/HighGainGuitarFinisherProcessor.*` owns VST3 audio I/O, automation/state handoff and lifecycle.
- `src/HighGainGuitarFinisherController.*` owns public parameters and controller state.
- `src/dsp/` contains audio algorithms with no dependency on the VST3 SDK.
- `tests/` validates DSP behaviour independently from a DAW.
- `docs/` records design intent and engineering decisions.

## Current signal path

`Stereo In -> FINISH Stage 1 -> ROOM (neutral placeholder) -> OUTPUT -> Stereo Out`

BYPASS skips intentional processing and output trim so bypass remains unity.

## FINISH Stage 1

The first measured stage is deliberately conservative:

1. a second-order high-pass filter whose cutoff moves from 55 Hz toward 85 Hz,
2. a broad peaking cut centred at 300 Hz, reaching -4 dB at maximum FINISH,
3. dry/processed interpolation tied to FINISH so `FINISH = 0` is exactly transparent.

This stage targets excess sub/low-end energy and low-mid congestion commonly found after high-gain amp/cab processing. It is not considered the complete FINISH algorithm.

## Real-time rules

The audio callback must not allocate memory, lock a mutex, access files, log, or perform GUI work.

DSP objects are prepared/reset from the VST3 lifecycle. Coefficients are updated only when a parameter value changes.

## State compatibility

The processor state begins with `kStateVersion`. Public test builds must not silently reorder or reinterpret existing serialized values. Any incompatible state change requires an explicit version migration.

## Validation

The standalone DSP smoke test checks:

- exact transparency at `FINISH = 0`,
- finite output,
- expected attenuation around 80 Hz,
- expected attenuation around 300 Hz,
- preservation of the useful midrange around 1 kHz.

DAW/host validation remains a separate layer and must be performed on built VST3 bundles.
