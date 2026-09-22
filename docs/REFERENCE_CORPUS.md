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


## Measured Renesans baseline

The three uploaded Renesans sessions were measured directly from the official 24-bit / 96 kHz mono guitar files:

- Labor Of Hate Take 2
- Less Than Nothing Take 2
- Split Brow Take 2

Each session contains a corresponding GTR 1 DI and GTR 1 Amp M81 track of identical duration.

Across the three GTR 1 Amp tracks, 630 active one-second windows above -45 dBFS were analysed.

### Aggregate active-window corridor

| Metric | P10 | P25 | Median | P75 | P90 |
| --- | ---: | ---: | ---: | ---: | ---: |
| RMS dBFS | -31.23 | -28.57 | -26.40 | -24.98 | -23.48 |
| Crest dB | 11.20 | 12.02 | 12.91 | 13.94 | 15.19 |
| 20-80 Hz relative energy dB | -25.82 | -20.76 | -17.15 | -13.76 | -11.64 |
| 80-200 Hz | -10.91 | -6.04 | -3.27 | -1.72 | -0.93 |
| 200-640 Hz | -13.87 | -11.25 | -8.84 | -7.07 | -5.81 |
| 640-2500 Hz | -13.03 | -10.08 | -7.28 | -5.13 | -3.58 |
| 2500-5000 Hz | -16.04 | -13.35 | -10.29 | -8.00 | -5.99 |
| 5000-10000 Hz | -29.95 | -26.82 | -22.95 | -18.22 | -14.97 |
| 10000-20000 Hz | -46.45 | -42.82 | -37.77 | -34.38 | -31.11 |

The band values are normalized to total measured 20 Hz-20 kHz energy per one-second window. They describe a real reference family, not a target EQ curve.

### Song medians

Labor Of Hate:
- RMS -26.83 dBFS
- crest 12.93 dB
- band medians: -18.39 / -4.44 / -7.67 / -7.16 / -9.08 / -22.78 / -37.55 dB

Less Than Nothing:
- RMS -27.06 dBFS
- crest 12.96 dB
- band medians: -15.53 / -4.11 / -8.55 / -6.86 / -10.04 / -21.69 / -37.27 dB

Split Brow:
- RMS -26.09 dBFS
- crest 12.82 dB
- band medians: -17.22 / -2.43 / -9.68 / -7.68 / -10.90 / -24.05 / -38.22 dB

Band order:
20-80 / 80-200 / 200-640 / 640-2500 / 2500-5000 / 5000-10000 / 10000-20000 Hz.

### DI versus amp sanity check

The DI tracks show substantially higher crest factors and far less energy above roughly 2.5 kHz than the corresponding amp captures. This confirms that the files are useful as genuine DI/amp reference pairs rather than redundant duplicates.

### Acceptance use

Renesans is now the first real-programme reference family for:
- good-source restraint,
- realistic crest-factor preservation,
- low/chug corridor checks,
- body and upper-mid corridor checks,
- DI-to-amp sanity observations.

It must not become the only target family. More metal/industrial DI/amp references should be added before production thresholds are tightened around real-programme statistics.
