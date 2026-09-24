# Architecture

## Current signal path

Stereo In -> optional continuous Low Cut -> adaptive FINISH -> bounded Auto Level / edge protection -> MASS -> ROOM -> OUTPUT -> Stereo Out

BYPASS skips intentional processing and output trim so bypass remains unity.

## Adaptive FINISH

FINISH uses five stereo-linked adaptive analysis/correction zones:

- low / palm-mute control: approximately 85, 110, 145 and 180 Hz;
- body resonance: approximately 220, 300, 390 and 500 Hz;
- articulation support: approximately 800, 1200, 1750 and 2400 Hz;
- harshness: approximately 2.8, 3.6, 4.5 and 5.6 kHz;
- fizz: approximately 6.0, 7.5, 9.0 and 11.0 kHz.

The frequencies are analysis anchors, not a universal target EQ. The adaptive controllers are stereo linked so image movement is not created by independent left/right decisions.

The complete 100% FINISH result is built first. The FINISH control then interpolates between the Low-Cut output and that complete result. This keeps 0% exact and makes 50% mathematically halfway to the full processing result.

## Modes

MODE changes the relative authority of the five adaptive zones while preserving the same underlying architecture:

1. Open / Balanced
2. Bite / Industrial
3. Smooth / Controlled

Mode weights are smoothed to avoid abrupt DSP jumps.

## Auto Level

Auto Level compares broadband programme energy immediately before and after the full adaptive FINISH correction.

Current safeguards:

- slow programme-energy tracking;
- quicker phrase bootstrap after silence;
- slower gain rise and faster return;
- compensation bounded to +/-3 dB;
- bidirectional compensation so louder processing is not rewarded in A/B;
- silence gate and programme reset;
- exact unity reset when FINISH returns to 0%.

Low Cut is outside the compensation comparison and is therefore never undone.

## MASS

MASS is a separate zero-latency linear character stage after FINISH and before ROOM.

The current full MASS path combines:

- a broad 140 Hz weight boost;
- nearby 220 Hz low-mid cleanup;
- a small fixed full-stage trim.

The full curve is calculated first and the public MASS control linearly interpolates between dry and full processing. Therefore MASS 0% is exact neutral and 50% is sample-exactly halfway to 100%.

See MASS_DESIGN.md for the measured reference rationale.

## LOW CUT

LOW CUT is continuously adjustable from 45 to 120 Hz with a true Off state. Frequency and engage/disengage transitions are smoothed. Filter coefficients are updated at bounded intervals in the audio path; no allocation is performed.

## ROOM architecture

ROOM is a dedicated high-gain-guitar ambience rather than a general-purpose reverb.

### Early reflections

Six asymmetric reflection taps per side begin at approximately 15.7 ms on the left and 18.1 ms on the right and extend to approximately 79-85 ms.

Cross-channel taps and alternating polarity increase density without reducing the effect to a single slap delay.

### Late field

A four-line feedback delay network uses approximately:

- 71.3 ms;
- 89.9 ms;
- 113.7 ms;
- 139.3 ms.

A normalized Hadamard-style feedback matrix diffuses energy between the four lines.

DECAY is independent of wet level and maps the feedback approximately from 0.50 to 0.88 using a shaped control law.

### Wet-path tone

The room input is high-passed around 200 Hz. The feedback network is damped around 5.2 kHz, and the final wet output is low-passed around 6 kHz. A restrained 2.1 kHz metallic band component increases with DECAY.

### Stereo and mono behaviour

The wet field is converted to mid/side internally and side is constrained rather than maximized. Dedicated metrology guards stereo correlation and mono energy.

### Ducking

A fast-attack, slower-release envelope follows the processed guitar. Strong events can reduce room gain so the ambience remains behind pick and palm-mute attacks and recovers into gaps.

### WET and DECAY

WET controls wet amount and DECAY independently controls tail behaviour. WET = 0 converges to exact zero wet output and clears tail state.

ROOM follows FINISH, Auto Level and MASS, so the FINISH loudness compensator does not attempt to cancel intended ambience.

## VST3 automation

Public parameter IDs remain stable.

The processor consumes every valid VST3 parameter-queue point at its supplied sample offset inside the current process block. It does not collapse automation to the last point of the block.

No heap allocation is introduced by automation processing; a fixed-size cursor array is used for the known public parameters.

## State and lifecycle

Current serialized processor state version: 6.

Older supported states are migrated deliberately:

- legacy Low Cut Boolean states map to the compatible 80 Hz position;
- older ROOM states seed DECAY where required;
- MODE and MASS default safely when absent.

DSP state is reset on activation, processing restart, state load and bypass transitions.

## Numerical and realtime safety

The audio path performs no file/network I/O, logging or blocking locks and allocates no delay memory during process().

Numerical safeguards include:

- non-finite input sanitization;
- emergency bounding of absurd finite input magnitudes far above the intended audio range;
- bounded detector-energy accumulation;
- non-finite biquad-state recovery;
- explicit collapse of numerically irrelevant biquad states below the subnormal-risk region.

These guards are failure containment, not normal-range tone shaping.

## Validation

Release-build tests use explicit runtime checks rather than C assert, so measurement failures remain active with NDEBUG.

Current deterministic QA includes transparency, FINISH/MASS amount-law checks, adaptive fixtures, Auto Level behaviour, ROOM metrology, sample-rate coverage, impulse/latency checks and pathological-input recovery.
