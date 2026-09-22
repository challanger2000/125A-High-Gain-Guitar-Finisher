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


## Measured Cnoc An Tursa baseline

Uploaded source:
- Cnoc An Tursa — Bannockburn edited excerpt
- 24-bit / 44.1 kHz mono
- 155 BPM
- 28.401 seconds
- educational-use multitrack excerpt

Retained tracks:
- Bass DI + Bass Amp
- four Electric Guitar DI tracks
- four corresponding Guitar Mic 1 tracks
- four corresponding Guitar Mic 2 tracks

### DI / cab sanity

Across the four guitars:
- DI crest factors are approximately 17.36-19.11 dB
- mic/cab crest factors are approximately 11.83-12.54 dB

Mic 1 and Mic 2 are aligned to about 2 samples (0.045 ms) and correlate around 0.95 after that offset, confirming that they are closely related alternate cabinet/microphone captures of the same performances.

Mic 1 consistently contains more 5-10 kHz energy than Mic 2, making the pair useful as a real brighter/aggressive versus darker/controlled tonal family.

### Active-window corridor

Eight guitar mic tracks produced 224 active one-second windows above -35 dBFS.

Aggregate P10 / P25 / median / P75 / P90:

| Metric | P10 | P25 | Median | P75 | P90 |
| --- | ---: | ---: | ---: | ---: | ---: |
| RMS dBFS | -16.21 | -15.94 | -15.52 | -14.95 | -14.64 |
| Crest dB | 10.64 | 10.90 | 11.23 | 11.55 | 11.76 |
| 20-80 Hz | -29.76 | -28.70 | -27.66 | -26.49 | -25.44 |
| 80-200 Hz | -8.83 | -8.37 | -7.84 | -7.33 | -6.83 |
| 200-640 Hz | -8.08 | -7.29 | -6.36 | -5.56 | -4.95 |
| 640-2500 Hz | -4.34 | -3.91 | -3.51 | -3.16 | -2.90 |
| 2500-5000 Hz | -9.46 | -9.01 | -8.56 | -7.99 | -7.68 |
| 5000-10000 Hz | -25.26 | -24.93 | -22.67 | -20.38 | -19.98 |
| 10000-20000 Hz | -46.92 | -46.29 | -43.67 | -40.91 | -40.09 |

Band values are normalized to measured 20 Hz-20 kHz energy per one-second window.

### Mic-family medians

Mic 1:
- RMS -15.33 dBFS
- crest 11.20 dB
- bands: -28.43 / -8.14 / -6.59 / -3.31 / -8.53 / -20.38 / -40.89 dB

Mic 2:
- RMS -15.63 dBFS
- crest 11.26 dB
- bands: -26.88 / -7.58 / -6.06 / -3.74 / -8.57 / -24.93 / -46.29 dB

The largest stable family difference is above 5 kHz: Mic 1 is the brighter capture, while Mic 2 is darker and smoother.

### Detector-bank validation on real programme

Dominant current detector centers across the 224 active windows:

- Low: 110 Hz in 158 windows, 145 Hz in 63, 180 Hz in 3
- Body: 390 Hz in 142, 500 Hz in 51, 220 Hz in 24, 300 Hz in 7
- Articulation: 2400 Hz in 158, 800 Hz in 62, 1750 Hz in 4
- Harshness: 2800 Hz in 115, 3600 Hz in 109
- Fizz: 6000 Hz in all 224 windows

This strongly supports the current five detector-bank center sets on real death-metal cabinet material. The highest Fizz centers (9/11 kHz) remain useful candidates for other amp/cab families but are not dominant in this source.

### Acceptance use

Cnoc An Tursa is the second real-programme family and is especially valuable for:
- DI-to-real-cab comparisons
- bright versus dark real mic captures
- low/chug and body detector validation
- 2.8/3.6 kHz harshness validation
- 6 kHz fizz validation
- mode-character comparisons
- crest-factor preservation on dense high-gain material

No production threshold should be derived from this family alone.
