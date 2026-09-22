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


### Renesans representative one-second windows

The following windows were selected from sufficiently active amp material (roughly above -32 dBFS) as targeted regression candidates. They are descriptive stress cases, not claims that the musical sections are defective.

- Typical corridor example: Less Than Nothing, 152-153 s
  - RMS -26.79 dBFS
  - crest 13.12 dB
- Strong 80-200 Hz dominance: Labor Of Hate, 224-225 s
  - RMS -17.76 dBFS
  - crest 9.70 dB
- Strong 200-640 Hz body: Less Than Nothing, 160-161 s
  - RMS -28.55 dBFS
  - crest 15.47 dB
- Strong 640-2500 Hz mid content: Less Than Nothing, 174-175 s
  - RMS -31.15 dBFS
  - crest 9.98 dB
- Strong 2500-5000 Hz upper-mid content: Labor Of Hate, 129-130 s
  - RMS -30.82 dBFS
  - crest 15.99 dB
- Strong 5000-10000 Hz content: Labor Of Hate, 214-215 s
  - RMS -28.80 dBFS
  - crest 16.56 dB
- High-crest active example: Split Brow, 155-156 s
  - RMS -26.92 dBFS
  - crest 17.93 dB
- Dense / low-crest active example: Split Brow, 228-229 s
  - RMS -29.64 dBFS
  - crest 9.41 dB

These timestamps can be used for short, repeatable render comparisons before longer full-song validation.


### Expanded Renesans guitar family

After adding GTR 2 Amp and the Pre Rec L/R guitar tracks from all three songs, the Renesans family contains 12 non-DI guitar tracks and 2,508 active one-second windows above -45 dBFS.

Aggregate P10 / P25 / median / P75 / P90:

| Metric | P10 | P25 | Median | P75 | P90 |
| --- | ---: | ---: | ---: | ---: | ---: |
| RMS dBFS | -35.20 | -33.61 | -31.34 | -27.36 | -25.19 |
| Crest dB | 9.14 | 11.16 | 12.30 | 13.60 | 15.18 |
| 20-80 Hz | -39.14 | -34.41 | -24.96 | -20.46 | -14.98 |
| 80-200 Hz | -10.32 | -6.70 | -4.19 | -2.16 | -1.06 |
| 200-640 Hz | -14.65 | -12.13 | -9.75 | -7.86 | -6.18 |
| 640-2500 Hz | -12.16 | -8.65 | -5.52 | -3.49 | -2.27 |
| 2500-5000 Hz | -15.63 | -12.09 | -9.89 | -8.17 | -6.95 |
| 5000-10000 Hz | -27.75 | -23.91 | -19.85 | -17.64 | -15.25 |
| 10000-20000 Hz | -54.69 | -49.63 | -42.54 | -35.30 | -28.69 |

Role medians:

- GTR 1 Amp M81: RMS -26.40 dBFS, crest 12.91 dB
- GTR 2 Amp M81: RMS -28.15 dBFS, crest 13.60 dB
- GTR Pre Rec L: RMS -33.30 dBFS, crest 11.45 dB
- GTR Pre Rec R: RMS -33.44 dBFS, crest 11.54 dB

The pre-recorded guitar pair is significantly more mid-forward and has much less 20-80 Hz energy than the live amp captures. It is therefore useful as a second tonal family inside the same session, but must not be treated as equivalent to the DI/Amp pair.


## Next-priority external reference

### Cnoc An Tursa — Bannockburn

Cambridge MT classifies the project as Death Metal.

The project notes state that all six guitar parts were recorded through dual-miked cabinets and also captured as DI feeds at the same time. This makes the session especially valuable for comparing multiple real high-gain amp/cab captures against their corresponding DI performances.

Cambridge lists:
- Edited Excerpt: 35 tracks, approximately 107 MB
- Full Multitrack: 43 tracks, approximately 999 MB

Priority: high. Prefer the edited excerpt first if it preserves the guitar DI/cab pairs; use the full multitrack only if required.
