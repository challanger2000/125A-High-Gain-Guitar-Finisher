#!/usr/bin/env python3
"""
Offline reference-audio analyser for the 125A High Gain Guitar Finisher.

Purpose:
- analyse user-supplied / legally obtained WAV references locally
- keep copyrighted reference audio out of the repository
- produce repeatable programme statistics for reference-corridor work

Supported input:
- PCM WAV, mono or stereo
- 16-bit or 24-bit integer PCM

No third-party Python packages are required.
"""

from __future__ import annotations

import argparse
import json
import math
import struct
import wave
from pathlib import Path
from typing import Iterable


BANDS = (
    (20.0, 80.0),
    (80.0, 200.0),
    (200.0, 640.0),
    (640.0, 2500.0),
    (2500.0, 5000.0),
    (5000.0, 10000.0),
    (10000.0, 20000.0),
)


def db20(x: float) -> float:
    return 20.0 * math.log10(max(x, 1.0e-20))


def db10(x: float) -> float:
    return 10.0 * math.log10(max(x, 1.0e-30))


def percentile(values: list[float], p: float) -> float:
    if not values:
        return float("nan")
    data = sorted(values)
    pos = (len(data) - 1) * p
    lo = int(math.floor(pos))
    hi = int(math.ceil(pos))
    if lo == hi:
        return data[lo]
    frac = pos - lo
    return data[lo] * (1.0 - frac) + data[hi] * frac


def read_pcm_wav(path: Path) -> tuple[int, list[float]]:
    with wave.open(str(path), "rb") as w:
        channels = w.getnchannels()
        width = w.getsampwidth()
        rate = w.getframerate()
        count = w.getnframes()
        comp = w.getcomptype()
        if comp != "NONE":
            raise ValueError(f"{path}: compressed WAV is not supported")
        if channels not in (1, 2):
            raise ValueError(f"{path}: only mono/stereo WAV is supported")
        if width not in (2, 3):
            raise ValueError(f"{path}: only 16/24-bit PCM WAV is supported")
        raw = w.readframes(count)

    samples: list[float] = []
    frame_bytes = channels * width

    for off in range(0, len(raw), frame_bytes):
        vals: list[float] = []
        for ch in range(channels):
            p = off + ch * width
            if width == 2:
                value = struct.unpack_from("<h", raw, p)[0] / 32768.0
            else:
                b0, b1, b2 = raw[p], raw[p + 1], raw[p + 2]
                value = b0 | (b1 << 8) | (b2 << 16)
                if value & 0x800000:
                    value -= 1 << 24
                value /= 8388608.0
            vals.append(value)
        samples.append(sum(vals) / len(vals))

    return rate, samples


def goertzel_energy(samples: list[float], rate: int, frequency: float) -> float:
    n = len(samples)
    if n == 0 or frequency <= 0.0 or frequency >= rate * 0.5:
        return 0.0

    omega = 2.0 * math.pi * frequency / rate
    coeff = 2.0 * math.cos(omega)
    s0 = s1 = s2 = 0.0

    # Hann window reduces leakage enough for broad-band integration.
    for i, x in enumerate(samples):
        if n > 1:
            win = 0.5 - 0.5 * math.cos(2.0 * math.pi * i / (n - 1))
        else:
            win = 1.0
        s0 = x * win + coeff * s1 - s2
        s2 = s1
        s1 = s0

    power = s1 * s1 + s2 * s2 - coeff * s1 * s2
    return max(power, 0.0)


def broad_band_energy(samples: list[float], rate: int) -> list[float]:
    # Log-spaced probes; broad corridor statistics matter more than FFT-bin
    # exactness here. Keeping this pure Python avoids external dependencies.
    result: list[float] = []
    for lo, hi in BANDS:
        effective_hi = min(hi, rate * 0.49)
        if effective_hi <= lo:
            result.append(0.0)
            continue

        probes = 24
        ratio = (effective_hi / lo) ** (1.0 / max(probes - 1, 1))
        freq = lo
        energy = 0.0
        for _ in range(probes):
            energy += goertzel_energy(samples, rate, freq)
            freq *= ratio
        result.append(energy)
    return result


def analyse_window(samples: list[float], rate: int) -> dict:
    if not samples:
        return {}

    sq = sum(x * x for x in samples)
    rms = math.sqrt(sq / len(samples))
    peak = max(abs(x) for x in samples)
    crest = db20(peak / max(rms, 1.0e-20))

    bands = broad_band_energy(samples, rate)
    total = sum(bands)
    band_db = [
        db10(e / total) if total > 0.0 and e > 0.0 else -300.0
        for e in bands
    ]

    return {
        "rms_dbfs": db20(rms),
        "peak_dbfs": db20(peak),
        "crest_db": crest,
        "bands_db": band_db,
    }


def analyse_file(path: Path, threshold_dbfs: float, window_seconds: float) -> dict:
    rate, mono = read_pcm_wav(path)
    window = max(1, int(round(rate * window_seconds)))

    whole = analyse_window(mono, rate)
    active: list[dict] = []

    for start in range(0, len(mono) - window + 1, window):
        result = analyse_window(mono[start:start + window], rate)
        if result and result["rms_dbfs"] >= threshold_dbfs:
            result["start_seconds"] = start / rate
            active.append(result)

    return {
        "file": str(path),
        "sample_rate": rate,
        "duration_seconds": len(mono) / rate,
        "whole_file": whole,
        "active_windows": active,
    }


def corpus_summary(results: Iterable[dict]) -> dict:
    windows = [
        w
        for result in results
        for w in result.get("active_windows", [])
    ]

    if not windows:
        return {"active_window_count": 0}

    percentiles = (0.10, 0.25, 0.50, 0.75, 0.90)

    def stats(values: list[float]) -> dict:
        return {
            f"p{int(p * 100):02d}": percentile(values, p)
            for p in percentiles
        }

    summary = {
        "active_window_count": len(windows),
        "rms_dbfs": stats([w["rms_dbfs"] for w in windows]),
        "crest_db": stats([w["crest_db"] for w in windows]),
        "bands_db": [],
    }

    for band_index, (lo, hi) in enumerate(BANDS):
        summary["bands_db"].append({
            "range_hz": [lo, hi],
            **stats([w["bands_db"][band_index] for w in windows]),
        })

    return summary


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("wav", nargs="+", type=Path)
    parser.add_argument("--threshold", type=float, default=-45.0,
                        help="active-window RMS threshold in dBFS")
    parser.add_argument("--window", type=float, default=1.0,
                        help="analysis-window duration in seconds")
    parser.add_argument("--json", type=Path,
                        help="optional JSON output path")
    args = parser.parse_args()

    results = [
        analyse_file(path, args.threshold, args.window)
        for path in args.wav
    ]

    output = {
        "analysis_version": 1,
        "window_seconds": args.window,
        "threshold_dbfs": args.threshold,
        "bands_hz": [list(b) for b in BANDS],
        "files": results,
        "corpus": corpus_summary(results),
    }

    encoded = json.dumps(output, indent=2)
    print(encoded)

    if args.json:
        args.json.write_text(encoded + "\n", encoding="utf-8")

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
