# Reference corpus strategy

The finisher must not be tuned against one guitar file or one target curve.

The reference corpus is intentionally split into several real-world families so the optimizer learns a useful corridor instead of flattening every source toward one tone.

## Source policy

Only material that is legitimately available for educational/home-studio analysis is used.

### Cambridge MT

The Mixing Secrets Free Multitrack Download Library includes multiple metal-family projects, including:

- Death Of A Romantic — Metalcore
- Decypher — Melodic Death Metal
- Dark Illusion — Metalcore
- Tholas P. — Industrial Metal
- Timboz — Screamo Metal
- Turbosauro — Progressive Metal

Some Cambridge projects explicitly provide rhythm guitars as DI tracks, making them suitable for controlled re-amping and post-amp processing comparisons.

Use: non-commercial measurement and regression reference only.

Reference:
https://www.cambridge-mt.com/ms3/mtk/

### TELEFUNKEN Live From The Lab

TELEFUNKEN provides downloadable multitracks for educational and home-studio use. Several sessions contain both guitar DI and guitar amp recordings.

Useful sessions include:

- Renesans — Guitars DI + Guitars Amp
- Doom Flamingo — Guitar DI + two Guitar Amp captures
- Briana Maia — Guitar DI + two Guitar Amp captures

Use: non-commercial measurement and regression reference only.

Reference:
https://www.telefunken-elektroakustik.com/livefromthelab/

## Corpus classes

The corpus should contain examples of:

1. already-balanced high-gain tone
2. excessive low/chug energy
3. excessive low-mid/body buildup
4. thin / articulation-deficient tone
5. harsh upper-mid tone
6. fizzy high-frequency tone
7. dense industrial-style rhythm guitar
8. darker compact tone
9. brighter bite-focused tone
10. palm-mute-heavy programme
11. sustained power chords
12. hard stops with ROOM enabled

No single source is treated as ground truth.

## Measurement goals

For each usable source, compare:

- broad-band energy distribution
- selected adaptive frequencies
- correction amount per adaptive zone
- RMS delta
- sample-peak delta
- crest-factor delta
- short-window level movement
- stereo correlation where applicable
- sub and air preservation
- behaviour at FINISH 0 / 50 / 100
- mode separation
- ROOM tail and ducking on hard stops

## Good-source restraint

A source already inside the acceptable corridor should receive bounded intervention.

The optimizer is not intended to make all references converge to the same spectrum.

## External EQ tools

Tools such as Sonible smart:EQ may be used as secondary observations only.

A fixed category target must not become the finisher's absolute target curve.

## Nonlinear / peak stages

A limiter, clipper or peak catcher is only added if the real corpus shows a repeatable need.

Current synthetic regression material does not justify a limiter by itself.
