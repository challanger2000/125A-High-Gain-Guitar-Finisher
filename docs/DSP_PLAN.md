# DSP plan

## Product target

The 125A High Gain Guitar Finisher is a post-amp/post-cab processor for distorted and high-gain electric guitar.

Its job is to move an already usable high-gain tone toward a controlled, dense, modern, mix-ready metal-guitar sound with minimal user interaction.

## User-facing controls

- **FINISH** — adaptive main macro.
- **ROOM** — short industrial guitar ambience.
- **LOW CUT 80 Hz** — optional conventional high-pass, off by default.
- **OUTPUT** — final level trim.
- **BYPASS** — unity processing bypass.

## Implemented FINISH foundation

FINISH currently provides:

- adaptive low-end/palm-mute control,
- adaptive body-resonance stabilization,
- adaptive harshness/fizz control,
- bounded automatic loudness compensation.

The current real-guitar 0%/100% FINISH comparison is essentially loudness matched while retaining lower processed peaks and intact transient shape.

## Implemented ROOM

ROOM is now a purpose-built high-gain guitar ambience.

Design goals:

- audible space without washing out double-tracked rhythm guitars,
- very early room cues rather than a long obvious reverb,
- dark wet spectrum,
- no low-frequency buildup,
- controlled stereo decorrelation,
- good mono retention,
- automatic ducking behind attacks.

Implementation:

- asymmetric early reflections from about 11 to 41 ms,
- four-line FDN tail around 48-84 ms,
- feedback increases moderately with ROOM,
- approximately 180 Hz wet high-pass,
- approximately 5.2 kHz feedback damping,
- approximately 6.2 kHz final wet low-pass,
- limited wet side component,
- up to about 42% wet ducking on strong attacks.

ROOM = 0 is exactly dry with respect to the room path.

## Remaining candidate stages

Do not add these unless measurements and listening tests demonstrate a clear benefit:

1. subtle harmonic cohesion,
2. dedicated peak control,
3. additional attack/presence shaping.

Any nonlinear stage must be measured for harmonic structure and aliasing before oversampling is considered.

## Engineering rules

- Stereo in / stereo out.
- 32-bit and 64-bit sample processing.
- No hidden global SDK dependency.
- Backward-compatible serialized state.
- Avoid loudness bias when evaluating FINISH.
- No oversampling without measured need.
- Measure latency if any future stage introduces it.
- Keep bypass unity.
- Keep DSP independent from the VST3 SDK wherever practical.
