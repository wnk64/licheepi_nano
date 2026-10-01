"""Operate only the Miracast settings UI, preserving the user's media task."""
import argparse
import json
import re
import time
import xml.etree.ElementTree as ET
from adb_cast_control import shell


def connected(serial):
    return b"mRemoteDisplayConnected=true" in shell(serial, "dumpsys display")


def tap_text(serial, text, timeout=45):
    deadline = time.monotonic() + timeout
    while time.monotonic() < deadline:
        data = shell(serial, "uiautomator dump /data/local/tmp/codex_cast_ui.xml >/dev/null 2>&1; cat /data/local/tmp/codex_cast_ui.xml; rm /data/local/tmp/codex_cast_ui.xml")
        try:
            root = ET.fromstring(data)
        except ET.ParseError:
            time.sleep(1)
            continue
        for node in root.iter("node"):
            if node.get("text") == text and node.get("enabled") == "true":
                bounds = re.fullmatch(r"\[(\d+),(\d+)\]\[(\d+),(\d+)\]", node.get("bounds", ""))
                if bounds:
                    x1, y1, x2, y2 = map(int, bounds.groups())
                    shell(serial, f"input tap {(x1+x2)//2} {(y1+y2)//2}")
                    return
        time.sleep(1)
    raise RuntimeError("No enabled matching cast UI item before deadline")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("action", choices=("connect", "disconnect", "status"))
    parser.add_argument("--serial", default="dda57287")
    parser.add_argument("--target", default="F1C200S-AIC")
    args = parser.parse_args()
    state = connected(args.serial)
    expected = args.action == "connect"
    if args.action != "status" and state != expected:
        shell(args.serial, "am start -a android.settings.CAST_SETTINGS")
        tap_text(args.serial, args.target)
        if args.action == "disconnect":
            tap_text(args.serial, "\u65ad\u5f00\u8fde\u63a5", timeout=10)
        deadline = time.monotonic() + 60
        while time.monotonic() < deadline:
            state = connected(args.serial)
            if state == expected:
                break
            time.sleep(1)
        else:
            raise RuntimeError("Cast connection state did not reach requested state")
        if expected:
            shell(args.serial, "input keyevent 4")
    print(json.dumps({"action": args.action, "connected": state, "target": args.target}))


if __name__ == "__main__":
    main()
