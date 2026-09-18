# Architecture

## Current signal path

Stereo In -> optional 80 Hz Low Cut -> adaptive FINISH -> bounded Auto Level -> ROOM -> OUTPUT -> Stereo Out

BYPASS skips intentional processing and output trim so bypass remains unity.

## Adaptive FINISH

FINISH uses three stereo-linked adaptive search banks:

- low/palm-mute region: approximately 85, 110, 145 and 180 Hz,
- body resonance region: approximately 220, 300, 390 and 500 Hz,
- harshness region: approximately 3.2, 4.2, 5.4 and 6.8 kHz.

The search frequencies are analysis anchors, not a universal target EQ.

## Auto Level

Auto Level compares broadband energy immediately before and after FINISH.

Safeguards:

- slow programme-energy tracking,
- slow makeup rise,
- faster return toward unity,
- maximum makeup +1.5 dB,
- no negative makeup,
- silence reset,
- exact unity reset at FINISH = 0.

The optional 80 Hz low cut is outside the compensation comparison and is therefore never undone.

## ROOM architecture

ROOM is intentionally a short industrial guitar ambience rather than a general-purpose reverb.

### Early reflections

Four asymmetric stereo reflection taps start around 11-13 ms and extend to roughly 41 ms.

Cross-channel taps and alternating polarity increase density without making the first reflections sound like a simple slap delay.

### Late field

A four-line feedback delay network uses approximately:

- 47.9 ms,
- 59.3 ms,
- 71.1 ms,
- 83.7 ms.

A normalized Hadamard-style feedback matrix diffuses energy between the four lines.

ROOM changes the feedback from roughly 0.48 toward 0.66 as the macro increases. This keeps low ROOM values tight and lets the maximum setting bloom without becoming a long ambient reverb.

### Wet-path tone

The room input is high-passed around 180 Hz so palm-mute and bass energy do not accumulate in the tail.

The feedback network is damped around 5.2 kHz and the final wet signal is low-passed around 6.2 kHz.

### Stereo and mono behaviour

The wet signal is converted to mid/side internally and the side component is limited to 72% of its raw value.

This preserves decorrelation while retaining useful mono energy.

### Ducking

A fast-attack, slower-release envelope follows the processed guitar.

Strong guitar events can reduce the wet path by up to roughly 42%. The reverb therefore stays behind the pick attack and blooms into the spaces between notes.

### ROOM macro

ROOM simultaneously controls wet level and tail density.

Maximum nominal wet gain is 0.22 before ducking. ROOM = 0 returns exactly zero wet signal and eventually clears the tail state.

ROOM is placed after FINISH and Auto Level, so the FINISH loudness compensator does not attempt to cancel the intended ambience.

## State and lifecycle

ROOM already existed as a serialized public parameter, so implementing its DSP requires no state-version change.

DSP state is reset on host activation, processing restart, state load and bypass transitions.

## Real-time rules

The audio callback performs no file access, logging or locking.

ROOM delay buffers are allocated during prepare, never during sample processing.

## Validation

Release-build tests now use explicit runtime checks rather than C assert, so guard failures remain active even when NDEBUG is defined.
