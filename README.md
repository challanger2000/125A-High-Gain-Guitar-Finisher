# 125A High Gain Guitar Finisher

Post-amp/post-cab processor for shaping distorted and high-gain guitars toward a tighter, cleaner and more mix-ready metal sound.

## Scope

This plugin is intentionally specialized. It is designed for distorted/high-gain electric guitar after an amp/cab stage.

It is **not** intended as an all-purpose processor for clean, acoustic, western or classical guitar.

## Core controls

- **FINISH** — exact 0-100% amount for the adaptive metal-finishing path.
- **MODE 1 / 2 / 3** — related Open/Balanced, Bite/Industrial and Smooth/Controlled optimizer characters.
- **MASS** — optional 0-100% weight/cleanup stage: low-end foundation plus nearby low-mid cleanup.
- **ROOM WET / DECAY** — short industrial guitar room, not a conventional long reverb.
- **LOW CUT** — continuously adjustable 45-120 Hz high-pass, with true Off.
- **OUTPUT** — final output trim.
- **BYPASS** — unity bypass.

## Current development status

FINISH uses five parallel adaptive zones for low/chug control, body, articulation, harshness and fizz rather than one fixed EQ recipe. A bounded internal Auto Level stage removes loudness bias without peak matching. FINISH is applied only after the complete 100% optimizer result is built, so 50% is mathematically halfway between 0% and 100%.

MASS is a separate linear character stage after FINISH and before ROOM. Its current reference-derived full curve combines a broad 140 Hz weight boost with 220 Hz cleanup and a small internal trim to minimize loudness bias. MASS 0% is sample-exact neutral, and MASS 50% is sample-exactly halfway to the full curve.

ROOM is now implemented as a deliberately short, dark and stereo-safe ambience for high-gain guitars:

- first reflections begin around 15.7 ms at 48 kHz,
- a four-line feedback delay network creates a dense short tail,
- the wet path is high-passed around 200 Hz to avoid low-end mud,
- upper frequencies are damped for a darker room character,
- stereo width is restrained for mono compatibility,
- adaptive ducking keeps the room behind strong pick and palm-mute attacks,
- WET and DECAY are independent controls.

At ROOM = 0 the room path contributes exactly zero wet signal.

## Validation

The manual **Build Windows VST3** workflow builds the plugin and runs:

- DSP smoke tests,
- adaptive multi-signature fixtures,
- Auto Level and anti-pumping tests,
- exact FINISH and MASS 0/50/100 interpolation tests,
- MASS frequency-response checks,
- dedicated ROOM impulse/decay/stereo/spectral and guitar-programme metrology,
- tonal/RMS/peak/crest/stereo measurements,
- 44.1/48/96/192 kHz stability checks,
- impulse/latency validation,
- processor/controller state migration parity,
- parameter-only VST3 flush and artifact-safe bypass transitions,
- exact offline/realtime render parity,
- per-channel silence flags including delayed ROOM-tail reactivation,
- stop/start DSP-state reset,
- 32-bit / 64-bit processor parity,
- Mono ROOM collapse parity against Stereo,
- FINISH, MASS and LOW CUT sample-rate response through 192 kHz,
- Auto Level anti-pumping parity through 192 kHz,
- ROOM timing/decay signature through 192 kHz,
- zero heap allocations in the warmed realtime DSP processing path.

The current engineering head has also completed Steinberg Validator 47/47 and the full internal suite at 14/14 tests.

All test guards are runtime-enforced in Release builds.

## Build

- Mono->Mono and Stereo->Stereo processing
- Windows x64
- Visual Studio 2022
- CMake 3.25+
- Steinberg VST3 SDK 3.8.1, pinned through CMake FetchContent

See docs/ARCHITECTURE.md, docs/DSP_PLAN.md, docs/MEASUREMENT_STRATEGY.md and docs/V2_READINESS.md for engineering details and the current release-readiness gate.
