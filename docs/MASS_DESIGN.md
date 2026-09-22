# MASS design

MASS is an optional guitar-character stage for adding perceived weight without turning the plugin into a bass enhancer.

It is deliberately independent from FINISH.

## Signal concept

The full 100% MASS curve is:

- broad boost: 140 Hz, +1.75 dB, Q 1.20
- broad cleanup: 220 Hz, -1.75 dB, Q 1.00
- fixed full-stage trim: -0.10 dB

The stage is inserted after the FINISH optimizer and before ROOM.

The complete 100% MASS path is always calculated internally and the user control linearly interpolates between dry and full MASS:

processed = dry + MASS * (fullMass - dry)

Therefore:

- MASS 0% is sample-exact neutral
- MASS 50% is sample-exactly halfway between 0% and 100%
- MASS 100% is the full designed curve

This avoids a nonlinear amount law caused by scaling biquad gain parameters directly.

## Why not copy a Pultec EQ

MASS is inspired by the general psychoacoustic idea of combining low-frequency weight with nearby low-mid cleanup.

It is not a Pultec emulation and does not use Pultec naming, frequencies, gain laws, transformer/tube behaviour or filter topology.

The design is specific to post-amp/post-cab high-gain guitar.

## Reference-derived choice

The candidate family was evaluated against two real multitrack reference families:

- Renesans — real Guitar DI / Amp captures
- Cnoc An Tursa — Bannockburn — four Guitar DI performances with two cabinet/mic captures each

The chosen conservative curve was preferred over more aggressive candidates because it remains useful across both families without relying strongly on loudness bias.

The intended 100% spectral direction is:

- modest 80-200 Hz foundation increase
- moderate 200-640 Hz cleanup
- very small change above the low-mid region
- negligible overall loudness bias

## Production guardrails

MASS must keep the following properties:

- no latency
- no nonlinear processing
- no oversampling requirement
- no detector/pumping behaviour
- exact neutrality at 0%
- exact linear interpolation at 50%
- stable operation from 44.1 to 192 kHz
- ROOM receives the already MASS-shaped guitar signal

A stronger MASS curve should only replace this design if real-programme measurements show a clear improvement without increased loudness bias or excessive sub energy.
