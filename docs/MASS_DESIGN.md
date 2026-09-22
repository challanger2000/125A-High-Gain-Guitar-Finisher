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


## Full FINISH + MASS real-audio validation

MASS was also evaluated after the complete FINISH path, matching its actual production position before ROOM.

### Bannockburn medians at MASS 100%

Mode 1:
- broad bands: +0.91 / +0.36 / -1.06 / +0.20 / -1.19 / -1.05 / +0.10 dB
- RMS delta: about -0.23 dB
- sample-peak delta: about -0.23 dB

Mode 2:
- broad bands: +0.60 / -0.44 / -1.43 / +0.37 / -0.04 / -1.93 / -0.15 dB
- RMS delta: about -0.20 dB
- sample-peak delta: about -0.04 dB

Mode 3:
- broad bands: +0.84 / +0.76 / -0.42 / +0.06 / -2.60 / -3.13 / -0.10 dB
- RMS delta: about -0.22 dB
- sample-peak delta: about -0.48 dB

### Renesans representative-window medians at MASS 100%

Mode 1:
- broad bands: +0.56 / -1.04 / +0.51 / +2.61 / -0.93 / -0.84 / +0.15 dB
- RMS delta: about -0.36 dB
- sample-peak delta: about +0.03 dB

Mode 2:
- broad bands: +0.45 / -1.33 / +0.36 / +2.74 / +0.98 / -0.94 / -0.05 dB
- RMS delta: about -0.32 dB
- sample-peak delta: about +0.29 dB

Mode 3:
- broad bands: +0.61 / -0.15 / +0.36 / +1.64 / -2.48 / -2.60 / -0.15 dB
- RMS delta: about -0.22 dB
- sample-peak delta: about 0.00 dB

Band order:
20-80 / 80-200 / 200-640 / 640-2500 / 2500-5000 / 5000-10000 / 10000-20000 Hz.

The combined results support keeping the conservative MASS curve. It increases perceived foundation relative to the already-finished signal while adding low-mid cleanup, without creating large peak growth or broad loudness inflation.
