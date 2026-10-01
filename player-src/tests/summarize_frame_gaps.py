"""Summarize measured successful video ioctl intervals, not visual scanout."""
import argparse
import json
import pathlib
import re


def summarize(text):
    rows = []
    started = False
    for line in text.splitlines():
        if "video commits:" in line:
            row = dict((k, int(v)) for k, v in re.findall(r"(\w+)=(\d+)", line))
            started = started or row.get("total", 0) > 0 or row.get("window", 0) > 0
            if started:
                rows.append(row)
    if not rows:
        return {"active_windows": 0}
    span = sum(r["span_us"] for r in rows) / 1000000
    frames = sum(r["window"] for r in rows)
    return {"active_windows": sum(r["window"] > 0 for r in rows),
            "stalled_windows": sum(r["window"] == 0 for r in rows),
            "span_s": span, "commits": frames,
            "commits_per_s": frames / span,
            "gaps_over250ms": sum(r["over250"] for r in rows),
            "max_completed_gap_us": max(r["max_gap_us"] for r in rows),
            "max_sampled_idle_us": max(r["idle_us"] for r in rows),
            "errors": rows[-1]["errors"]}


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("logs", nargs="+")
    args = parser.parse_args()
    for name in args.logs:
        print(json.dumps({"log": name, "stats": summarize(
            pathlib.Path(name).read_text(errors="replace"))}))
