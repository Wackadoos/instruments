"""Decode, plot and CSV-export binary data logs from the instruments firmware.

Run: uv run main.py [--file PATH] [--no-show] [--out csv_path]

The .bin format is fixed (see src/modules/sd.h):

  12-byte header: b"INSTRLOG" + u16 format version + u16 reserved
  frames of FRAME_BYTES each:
    u16 MAGIC (frame delimiter / corruption check)
    u32 epoch (RTC seconds since 1970 UTC)
    u32 millis (ms since boot)
    128-byte LogPayload (packed, little-endian, matching the C struct)

Frames are fixed size, so decoding is robust to power loss mid-write: the
decoder stops at the first frame whose MAGIC is invalid (the preallocated /
truncated tail), and can resync past mid-file corruption.
"""

from __future__ import annotations

import argparse
import csv
import os
import struct
import sys
from datetime import datetime, timezone

import numpy as np

HEADER_MAGIC = b"INSTRLOG"
HEADER_SIZE = 12
FRAME_MAGIC = 0x4C47
PAYLOAD_BYTES = 128
FRAME_HEADER_BYTES = 10  # magic u16 + epoch u32 + millis u32
FRAME_BYTES = FRAME_HEADER_BYTES + PAYLOAD_BYTES

# Field layout of LogPayload in src/modules/sd.h. Keep in the same order.
_PAYLOAD_SPEC = [
    ("mode", "u1"),
    ("temp_motor_1", "f4"),
    ("temp_motor_2", "f4"),
    ("imu_accel_x", "f4"),
    ("imu_accel_y", "f4"),
    ("imu_accel_z", "f4"),
    ("imu_die_temp", "f4"),
    ("max_1s_acceleration", "f4"),
    ("max_1s_accel_x", "f4"),
    ("max_1s_accel_y", "f4"),
    ("max_1s_accel_z", "f4"),
    ("uncalibrated_altitude", "f4"),
    ("ambient_temperature", "f4"),
    ("relative_humidity", "f4"),
    ("ambient_temperature_2", "f4"),
    ("kilometers_per_hour", "f4"),
    ("motor_current", "f4"),
    ("battery_current", "f4"),
    ("battery_power", "f4"),
    ("duty_cycle", "f4"),
    ("battery_voltage", "f4"),
    ("watts_used", "f4"),
    ("watts_charged", "f4"),
    ("esc_temp", "f4"),
    ("gps_speed", "f4"),
    ("heading", "f4"),
    ("battery_soc_compensated", "f4"),
    ("battery_time_remaining_mins", "f4"),
    ("latitude", "i4"),
    ("longitude", "i4"),
    ("altitude", "i4"),
    ("visible_satellites", "u1"),
    ("fix_satellites", "u1"),
    ("ram_free_bytes", "u2"),
    ("ram_free_bytes_minimum", "u2"),
    ("reserved", "u1"),
]
PAYLOAD_DTYPE = np.dtype(_PAYLOAD_SPEC)
assert PAYLOAD_DTYPE.itemsize == PAYLOAD_BYTES, "payload dtype size mismatch"


def _resync(data: bytes, start: int) -> int | None:
    """Scan forward for the next plausible frame start (MAGIC at p and p+FRAME_BYTES).

    A lone MAGIC is only accepted for the single last slot that ends exactly at
    EOF, avoiding false positives in a garbage/zero preallocated tail.
    """
    n = len(data)
    needle = struct.pack("<H", FRAME_MAGIC)
    p = start
    while p + 2 <= n:
        p = data.find(needle, p)
        if p < 0 or p + FRAME_BYTES > n:
            return None
        if p + 2 * FRAME_BYTES <= n:
            if struct.unpack_from("<H", data, p + FRAME_BYTES)[0] == FRAME_MAGIC:
                return p
        elif p == n - FRAME_BYTES:
            return p  # last full frame possible, ends exactly at EOF
        p += 2
    return None


def parse_log(path: str) -> dict:
    """Parse a .bin log. Truncated/corrupt tails are dropped, not fatal."""
    with open(path, "rb") as fh:
        data = fh.read()

    if len(data) < HEADER_SIZE or data[:8] != HEADER_MAGIC:
        raise ValueError(f"{path}: not an instruments log (bad header)")
    version = struct.unpack_from("<H", data, 8)[0]

    epochs: list[int] = []
    millis: list[int] = []
    payloads: list[np.void] = []
    pos = HEADER_SIZE
    n = len(data)
    while pos + FRAME_BYTES <= n:
        if struct.unpack_from("<H", data, pos)[0] != FRAME_MAGIC:
            new_pos = _resync(data, pos)
            if new_pos is None:
                break  # corrupt or truncated tail
            pos = new_pos
            continue
        epochs.append(struct.unpack_from("<I", data, pos + 2)[0])
        millis.append(struct.unpack_from("<I", data, pos + 6)[0])
        payloads.append(
            np.frombuffer(data, dtype=PAYLOAD_DTYPE, count=1, offset=pos + FRAME_HEADER_BYTES)[0].copy()
        )
        pos += FRAME_BYTES

    if not payloads:
        raise ValueError(f"{path}: no complete frames found")

    return {
        "version": version,
        "epoch": np.asarray(epochs, dtype=np.uint32),
        "millis": np.asarray(millis, dtype=np.uint32),
        "payload": np.asarray(payloads, dtype=PAYLOAD_DTYPE),
        "truncated": pos < n,  # stopped early: partial/corrupt data or preallocated tail
        "stopped_at": pos,
    }


def _format_value(value: np.generic) -> str:
    if value.dtype.kind == "f":
        return repr(float(value))
    return str(int(value))


def write_csv(path: str, parsed: dict) -> int:
    """Write every frame as a row. Returns the number of data rows written."""
    payload = parsed["payload"]
    fields = list(PAYLOAD_DTYPE.names)
    epoch = parsed["epoch"]
    millis = parsed["millis"]

    with open(path, "w", newline="") as fh:
        writer = csv.writer(fh)
        writer.writerow(["datetime_utc", "epoch_seconds", "millis_since_boot"] + fields)
        for i in range(len(payload)):
            dt = datetime.fromtimestamp(epoch[i] + millis[i] / 1000.0, tz=timezone.utc)
            writer.writerow(
                [dt.isoformat(), str(epoch[i]), str(millis[i])]
                + [_format_value(payload[field][i]) for field in fields]
            )
    return len(payload)


_PANELS = [
    (
        "Speed (km/h)",
        [
            ("kilometers_per_hour", "Wheel"),
            ("gps_speed", "GPS"),
        ],
    ),
    (
        "Battery",
        [
            ("battery_voltage", "Voltage (V)"),
            ("battery_current", "Current (A)"),
            ("battery_power", "Power (W)"),
            ("duty_cycle", "Duty (%)"),
            ("battery_soc_compensated", "SOC (%)"),
            ("battery_time_remaining_mins", "Time left (min)"),
        ],
    ),
    (
        "Temperatures (degC)",
        [
            ("temp_motor_1", "Motor 1"),
            ("temp_motor_2", "Motor 2"),
            ("esc_temp", "ESC"),
            ("ambient_temperature", "Ambient BMP"),
            ("ambient_temperature_2", "Ambient ATH"),
            ("imu_die_temp", "IMU die"),
        ],
    ),
    (
        "Acceleration (m/s^2)",
        [
            ("imu_accel_x", "X"),
            ("imu_accel_y", "Y"),
            ("imu_accel_z", "Z"),
            ("max_1s_acceleration", "1s max"),
            ("max_1s_accel_x", "1s max X"),
            ("max_1s_accel_y", "1s max Y"),
            ("max_1s_accel_z", "1s max Z"),
        ],
    ),
    (
        "GPS & Environment",
        [
            ("latitude", "Latitude (deg)", lambda v: v / 1e7),
            ("longitude", "Longitude (deg)", lambda v: v / 1e7),
            ("altitude", "GPS alt (m)", lambda v: v / 100.0),
            ("heading", "Heading (deg)"),
            ("visible_satellites", "Visible sats"),
            ("fix_satellites", "Fix sats"),
            ("relative_humidity", "Humidity (%)"),
            ("uncalibrated_altitude", "Baro alt (m)"),
        ],
    ),
    (
        "Energy & Memory",
        [
            ("watts_used", "Wh used"),
            ("watts_charged", "Wh charged"),
            ("ram_free_bytes", "Free RAM (B)"),
            ("ram_free_bytes_minimum", "Min free RAM (B)"),
        ],
    ),
]


def plot(parsed: dict, source_path: str, png_path: str | None = None, show: bool = True) -> None:
    import matplotlib

    if not show:
        matplotlib.use("Agg")
    import matplotlib.pyplot as plt

    payload = parsed["payload"]
    t = parsed["epoch"].astype(np.float64) + parsed["millis"].astype(np.float64) / 1000.0
    t_min = (t - t[0]) / 60.0

    rows = len(_PANELS)
    fig, axes = plt.subplots(rows, 1, figsize=(12, 2.3 * rows), sharex=True, squeeze=False)
    for ax, (title, series) in zip(axes[:, 0], _PANELS):
        for entry in series:
            field, label = entry[0], entry[1]
            transform = entry[2] if len(entry) > 2 else None
            values = payload[field].astype(np.float64)
            if transform:
                values = transform(values)
            ax.plot(t_min, values, label=label, linewidth=0.8)
        ax.set_ylabel(title)
        ax.grid(True, alpha=0.3)
        ax.legend(fontsize=8, loc="upper right", ncol=3)
    axes[-1, 0].set_xlabel("Minutes since log start")

    duration_s = t[-1] - t[0]
    fig.suptitle(
        f"{os.path.basename(source_path)}  -  {len(payload)} frames, "
        f"{duration_s / 60.0:.1f} min ({t[0]:.0f} epoch start)",
        fontsize=11,
    )
    fig.tight_layout(rect=(0, 0, 1, 0.98))
    if png_path:
        fig.savefig(png_path, dpi=150)
    if show:
        plt.show()


def pick_file() -> str | None:
    try:
        import tkinter as tk
        from tkinter import filedialog

        root = tk.Tk()
        root.withdraw()
        path = filedialog.askopenfilename(
            title="Select instruments binary log",
            filetypes=[("Binary log", "*.bin"), ("All files", "*.*")],
        )
        root.destroy()
        return path
    except Exception as exc:  # no display / headless environment
        print(f"File dialog unavailable: {exc}", file=sys.stderr)
        return None


def main() -> None:
    parser = argparse.ArgumentParser(description="Decode, plot and export an instruments .bin log.")
    parser.add_argument("--file", help="Path to the .bin log (prompts via tkinter if omitted)")
    parser.add_argument("--no-show", action="store_true", help="Don't open the plot window (PNG + CSV still written)")
    parser.add_argument("--out", help="CSV output path (default: <bin>.csv)")
    args = parser.parse_args()

    path = args.file or pick_file()
    if not path:
        sys.exit("No file selected")

    parsed = parse_log(path)
    base, _ = os.path.splitext(path)
    csv_path = args.out or base + ".csv"
    png_path = base + ".png"

    n = write_csv(csv_path, parsed)
    plot(parsed, path, png_path, show=not args.no_show)

    tail = "  [TRUNCATED - stopped at byte %d]" % parsed["stopped_at"] if parsed["truncated"] else ""
    print(
        f"Decoded {n} frames from {path} (format v{parsed['version']})\n"
        f"  CSV: {csv_path}\n  PNG: {png_path}{tail}"
    )


if __name__ == "__main__":
    main()
