"""Delegate verified power commands to the debug-hub skill, capture boot UART."""
import argparse
import importlib.util
import json
import pathlib
import time
import serial

HUB_SCRIPT = pathlib.Path(r"C:\Users\26301\.codex\skills\f1c200s-debug-hub\scripts\hub_power.py")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--power-port", default="COM3")
    parser.add_argument("--console-port", default="COM4")
    parser.add_argument("--off-seconds", type=float, default=5)
    parser.add_argument("--boot-seconds", type=float, default=35)
    parser.add_argument("--log", required=True)
    args = parser.parse_args()
    if args.power_port == args.console_port or args.off_seconds < 3 or not 1 <= args.boot_seconds <= 60:
        parser.error("distinct ports, off>=3s and boot capture1..60s required")
    spec = importlib.util.spec_from_file_location("debug_hub_dependency", HUB_SCRIPT)
    hub = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(hub)
    hub.PORT = args.power_port
    data = bytearray()
    with pathlib.Path(args.log).open("xb") as log:
        with serial.Serial(args.console_port, 115200, timeout=0.2) as console:
            console.reset_input_buffer()
            hub.send_command("OFF", 3)
            time.sleep(args.off_seconds)
            hub.send_command("ON", 3)
            deadline = time.monotonic() + args.boot_seconds
            while time.monotonic() < deadline:
                chunk = console.read(4096)
                if chunk:
                    log.write(chunk)
                    log.flush()
                    data.extend(chunk)
                    if len(data) > 1024 * 1024:
                        raise RuntimeError("boot console exceeds1MiB guard")
    marker = b"U-Boot" in data or b"Linux version" in data
    print(json.dumps({"power_port": args.power_port, "console_port": args.console_port,
                      "off_ack": True, "on_ack": True, "off_seconds": args.off_seconds,
                      "console_bytes": len(data), "boot_marker": marker, "log": args.log}))
    if not marker:
        raise RuntimeError("power acknowledgements verified but no boot marker captured")


if __name__ == "__main__":
    main()
