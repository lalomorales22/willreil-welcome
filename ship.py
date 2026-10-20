#!/usr/bin/env python3
"""
ship.py — flash the welcome firmware onto a board and print its card label.

    python ship.py                       # flash the first ESP32 found, then print the label
    python ship.py --port COM7           # pick the port yourself
    python ship.py --no-flash            # board already flashed? just read the label
    python ship.py --bin path/to/x.bin   # merged binary to flash (default: newest build/*/*.merged.bin)

Needs:   pip install esptool pyserial
The .bin: Arduino IDE -> Sketch -> Export Compiled Binary -> use the *.merged.bin
          (merged = bootloader + partitions + app in one file, flashed at 0x0).

Every label is also appended to labels.csv next to this script, so there's a
record of which unit (MAC) got which password.
"""

import argparse
import csv
import datetime
import pathlib
import subprocess
import sys
import time

try:
    import serial
    import serial.tools.list_ports
except ImportError:
    sys.exit("missing pyserial:  pip install esptool pyserial")

ESPRESSIF_VID = 0x303A          # the C6's native USB shows up with this vendor id
HERE = pathlib.Path(__file__).resolve().parent
def default_bin():
    """Arduino IDE 2 exports to willreil_welcome/build/<fqbn>/willreil_welcome.ino.merged.bin — pick the newest one anywhere under this repo."""
    hits = sorted(HERE.rglob("*.merged.bin"), key=lambda f: f.stat().st_mtime)
    return hits[-1] if hits else HERE / "willreil_welcome.ino.merged.bin"

DEFAULT_BIN = default_bin()
LOG = HERE / "labels.csv"


def find_port():
    ports = list(serial.tools.list_ports.comports())
    esp = [p for p in ports if p.vid == ESPRESSIF_VID]
    pick = esp or [p for p in ports if "usb" in (p.device + " " + (p.description or "")).lower()]
    if not pick:
        sys.exit("no board found. plug it in with a DATA usb-c cable (some are charge-only).")
    if len(pick) > 1:
        print("several boards plugged in, using", pick[0].device, "— pass --port to choose")
    return pick[0].device


def flash(port, binpath):
    if not binpath.exists():
        sys.exit(f"can't find {binpath}\nexport it: Arduino IDE -> Sketch -> Export Compiled Binary")
    cmd = [sys.executable, "-m", "esptool", "--chip", "esp32c6", "--port", port, "--baud", "921600",
           "write_flash", "0x0", str(binpath)]
    print("flashing ...")
    if subprocess.run(cmd).returncode != 0:
        sys.exit("flash failed. hold BOOT, tap RESET, let go of BOOT, and run again.")


def read_label(port, timeout=25):
    """Open the serial port, poke the firmware, wait for its LABEL|ssid|pass|mac line."""
    deadline = time.time() + timeout
    while time.time() < deadline:
        try:
            with serial.Serial(port, 115200, timeout=1) as s:
                time.sleep(0.6)
                s.reset_input_buffer()
                s.write(b"\n")                      # any keypress reprints the banner
                end = time.time() + 8
                while time.time() < end:
                    line = s.readline().decode("utf-8", "ignore").strip()
                    if line.startswith("LABEL|"):
                        parts = line.split("|")
                        if len(parts) >= 4:
                            return parts[1], parts[2], parts[3]
        except serial.SerialException:
            time.sleep(1)                           # usb port re-enumerates after the post-flash reset
    sys.exit("never saw the label line. press RESET on the board and run again with --no-flash")


def print_label(ssid, pw, mac, ip="4.3.2.1"):
    w = 34
    rows = ["WILL REIL INDUSTRIES", "", f"wifi: {ssid}", f"pass: {pw}", "", f"then open http://{ip}", f"unit {mac}"]
    print("\n+" + "-" * w + "+")
    for r in rows:
        print("|  " + r.ljust(w - 2) + "|")
    print("+" + "-" * w + "+\n")


def log(ssid, pw, mac):
    new = not LOG.exists()
    with LOG.open("a", newline="") as f:
        wr = csv.writer(f)
        if new:
            wr.writerow(["when", "mac", "ssid", "password"])
        wr.writerow([datetime.datetime.now().isoformat(timespec="seconds"), mac, ssid, pw])


def main():
    ap = argparse.ArgumentParser(description="flash + label a Will Reil ESP32-C6")
    ap.add_argument("--port", help="serial port (auto-detected if omitted)")
    ap.add_argument("--bin", type=pathlib.Path, default=DEFAULT_BIN, help="merged .bin to flash")
    ap.add_argument("--no-flash", action="store_true", help="skip flashing, just read the label")
    a = ap.parse_args()

    port = a.port or find_port()
    print("board on", port)
    if not a.no_flash:
        flash(port, a.bin)
        time.sleep(2)
    ssid, pw, mac = read_label(port)
    print_label(ssid, pw, mac)
    log(ssid, pw, mac)
    print(f"logged to {LOG.name}. next board.")


if __name__ == "__main__":
    main()
