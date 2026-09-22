# MASS design

MASS is an optional guitar-character stage for adding perceived weight without turning the plugin into a bass enhancer.

It is deliberately independent from FINISH.

## Signal concept

The full 100% MASS curve is:

- broad boost: 140 Hz, +10.50 dB, Q 1.20
- broad cleanup: 220 Hz, -10.50 dB, Q 1.00
- fixed full-stage trim: -0.60 dB

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

The first conservative range was rejected in listening because even 100% remained too subtle. The design goal was therefore changed deliberately: the user must be able to choose subtle processing at low settings and intentionally overdo the effect at high settings.

On the user's real guitar render, the expanded linear range measures approximately:

- 25%: +0.66 dB in 80-200 Hz / -1.06 dB in 200-640 Hz
- 50%: +1.42 dB / -2.23 dB
- 75%: +2.22 dB / -3.51 dB
- 100%: +3.02 dB / -4.87 dB

The intended control philosophy is therefore:

- low settings = subtle weight/cleanup
- middle settings = clearly audible shaping
- high settings = deliberately strong and optionally exaggerated
- 0% remains exact bypass of the MASS stage
- the control remains sample-linear between dry and the full 100% curve

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

The range is intentionally not capped at a permanently conservative sound. The user chooses the amount. Safety requirements apply to numerical stability, peak growth and reproducibility, not to preventing strong tonal choices.


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
