"""Validate standard legacy U-Boot script header, CRCs and source equality."""
import argparse
import hashlib
import json
import pathlib
import struct
import zlib


def inspect(image, source=None):
    if len(image) < 72:
        raise ValueError("truncated image")
    values = struct.unpack(">7I4B32s", image[:64])
    magic, hcrc, timestamp, size, load, entry, dcrc, os_id, arch, kind, comp, name = values
    if magic != 0x27051956 or (os_id, arch, kind, comp) != (5, 2, 6, 0):
        raise ValueError("not an uncompressed ARM Linux legacy script")
    header = bytearray(image[:64])
    header[4:8] = b"\0" * 4
    if zlib.crc32(header) & 0xffffffff != hcrc:
        raise ValueError("header CRC mismatch")
    if len(image) != size + 64 or zlib.crc32(image[64:]) & 0xffffffff != dcrc:
        raise ValueError("data size/CRC mismatch")
    script_size, terminator = struct.unpack(">2I", image[64:72])
    if terminator or script_size != len(image) - 72:
        raise ValueError("not exactly one script component")
    script = image[72:]
    if source is not None and script != source:
        raise ValueError("source differs from actual image payload")
    return {"md5": hashlib.md5(image).hexdigest(), "timestamp": timestamp,
            "load": load, "entry": entry, "name": name.split(b"\0", 1)[0].decode("ascii"),
            "script_size": script_size, "script": script.decode("ascii")}


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("image")
    parser.add_argument("--source")
    parser.add_argument("--timestamp-only", action="store_true")
    args = parser.parse_args()
    result = inspect(pathlib.Path(args.image).read_bytes(),
                     pathlib.Path(args.source).read_bytes() if args.source else None)
    print(result["timestamp"] if args.timestamp_only else json.dumps(result))
