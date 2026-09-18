# Architecture

## Design goals

The project is intentionally split into a thin VST3 integration layer and reusable DSP code.

- src/HighGainGuitarFinisherProcessor.* owns VST3 audio I/O, automation/state handoff and lifecycle.
- src/HighGainGuitarFinisherController.* owns public parameters and controller state.
- src/dsp/ contains audio algorithms with no dependency on the VST3 SDK.
- tests/ validates DSP behaviour independently from a DAW.
- docs/ records design intent and engineering decisions.

## Current signal path

Stereo In -> optional 80 Hz Low Cut -> adaptive FINISH analysis/processing -> bounded Auto Level -> ROOM placeholder -> OUTPUT -> Stereo Out

BYPASS skips intentional processing and output trim so bypass remains unity.

## Adaptive FINISH architecture

The former fixed 85 Hz high-pass, fixed 300 Hz cut and fixed 4.8 kHz harshness centre have been removed from the default FINISH path.

FINISH now uses three adaptive search banks.

### 1. Low-end / palm-mute search

Candidate centres cover approximately 85, 110, 145 and 180 Hz.

The controller compares fast band energy against a slower learned baseline. It reacts to low-frequency rises characteristic of palm-mute thump rather than applying one permanent low-frequency cut.

### 2. Body resonance search

Candidate centres cover approximately 220, 300, 390 and 500 Hz.

A slowly learned spectral profile identifies a locally dominant body/low-mid resonance. Broad, balanced body energy is intentionally left alone.

### 3. Harshness search

Candidate centres cover approximately 3.2, 4.2, 5.4 and 6.8 kHz.

The controller can react to both short upper-mid bursts and persistent local resonances. It does not use a permanent low-pass filter.

## Auto-level compensation

Auto Level is internal rather than a user-facing loudness effect.

It compares broadband stereo energy immediately before FINISH against the post-FINISH result.

Current safeguards:

- approximately 500 ms programme-energy tracking,
- approximately 750 ms makeup rise,
- faster return toward unity when less compensation is needed,
- +1.5 dB hard maximum,
- no negative makeup: it does not turn a louder result down,
- independent activity detector and silence reset,
- exact unity reset at FINISH = 0.

The reference point is after LOW CUT 80 Hz. Therefore the optional fixed high-pass remains intentional and is never compensated away.

The purpose is comparison fairness, not dynamics processing. Individual transients and peaks are deliberately not matched.

## Search grids are not target EQ curves

The candidate frequencies are measurement probes and processing anchors, not universal correction values.

Each bank measures its candidates continuously, selects the region that best matches the current source and crossfades the selection over time. Selection hysteresis prevents neighbouring bands from chattering.

The fixed numeric values that remain are safety boundaries: search ranges, time constants and maximum internal reduction.

## Optional 80 Hz low cut

LOW CUT 80 Hz is a separate user choice and is off by default.

It uses a second-order high-pass at 80 Hz and is smoothly crossfaded when switched. It is independent from FINISH.

## State compatibility

State version 2 appends the Low Cut parameter after the original four serialized values.

Version 1 states remain readable. When an old state is loaded, LOW CUT 80 Hz defaults to Off.

Auto Level adds no serialized parameter and therefore requires no new state version.

## Real-time rules

The audio callback must not allocate memory, lock a mutex, access files, log, or perform GUI work.

Adaptive filter banks, envelopes and Auto Level use preallocated fixed-size state only.

## Validation

The automated suite checks:

- exact transparency at FINISH = 0 with LOW CUT off,
- optional 80 Hz low-cut response,
- finite output from 44.1 to 192 kHz,
- adaptive frequency movement between substantially different synthetic guitar signatures,
- bounded Auto Level over several sample rates,
- Auto Level silence/reset behaviour,
- RMS/crest/spectral guardrails,
- stereo-correlation stability,
- impulse response and zero-lookahead latency.

DAW/host validation remains a separate layer and must still be performed on built VST3 bundles.
