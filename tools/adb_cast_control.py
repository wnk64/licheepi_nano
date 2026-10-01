"""Reuse an already-authorized ADB server for Miracast UI diagnostics."""
import argparse
import json
import socket
import sys
import xml.etree.ElementTree as ET


def exact(sock, size):
    data = bytearray()
    while len(data) < size:
        chunk = sock.recv(size - len(data))
        if not chunk:
            raise RuntimeError("ADB service closed unexpectedly")
        data.extend(chunk)
    return bytes(data)


def request(sock, command):
    payload = command.encode()
    sock.sendall(f"{len(payload):04x}".encode() + payload)
    if exact(sock, 4) != b"OKAY":
        length = int(exact(sock, 4), 16)
        raise RuntimeError(exact(sock, length).decode(errors="replace"))


def shell(serial, command):
    with socket.create_connection(("127.0.0.1", 5037), 5) as sock:
        sock.settimeout(30)
        request(sock, "host:transport:" + serial)
        request(sock, "shell:" + command)
        result = bytearray()
        while True:
            data = sock.recv(65536)
            if not data:
                return bytes(result)
            result.extend(data)
            if len(result) > 16 * 1024 * 1024:
                raise RuntimeError("ADB diagnostic output exceeds limit")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--serial", default="dda57287")
    parser.add_argument("command", help="shell command, --ui or --display")
    args = parser.parse_args()
    command = args.command
    if command == "--ui":
        command = "uiautomator dump /data/local/tmp/codex_cast_ui.xml >/dev/null 2>&1; cat /data/local/tmp/codex_cast_ui.xml; rm /data/local/tmp/codex_cast_ui.xml"
    elif command == "--display":
        command = "dumpsys display"
    result = shell(args.serial, command)
    if args.command == "--ui":
        root = ET.fromstring(result)
        for node in root.iter("node"):
            if node.get("text") or node.get("content-desc"):
                print(json.dumps({key: node.get(key) for key in
                                  ("text", "content-desc", "enabled", "bounds")}, ensure_ascii=True))
    elif args.command == "--display":
        for line in result.decode(errors="replace").splitlines():
            if any(word in line for word in ("mActiveDisplayState", "mActiveDisplay=", "mRemoteDisplayConnected")):
                print(line.encode("ascii", errors="backslashreplace").decode())
    else:
        sys.stdout.buffer.write(result)


if __name__ == "__main__":
    main()
