"""Read-only sampling of Miracast processes, CPU, memory and RTP socket drops."""
import argparse
import json
import re
import subprocess
import time

PLINK = r"D:\AI工具\putty\plink.exe"
HOSTKEY = "SHA256:I1li/9+0wRwOzVnASuXFl8VwY6uZTMTKaE13b84S/Dc"
RUNTIME = "/tmp/manual_miracast_20260920"


def ssh(host, command):
    run = subprocess.run([PLINK, "-batch", "-ssh", "-hostkey", HOSTKEY,
                          "-pw", "1", "root@" + host, command],
                         capture_output=True, timeout=25)
    if run.returncode:
        raise RuntimeError(run.stderr.decode(errors="replace").strip())
    return run.stdout.decode(errors="replace")


def parse_rtp_line(line):
    values = line.split()
    return {"inode": int(values[9]), "drops": int(values[-1]),
            "queue_accounting_bytes": int(values[4].split(":")[1], 16)}


def parse_wlan1_line(line):
    values = list(map(int, line.partition(":")[2].split()))
    return {"rx_bytes": values[0], "rx_packets": values[1], "rx_drops": values[3]}


def snapshot(host):
    ids = ssh(host, f"cat {RUNTIME}/player.pid {RUNTIME}/sink.pid").split()
    if len(ids) != 2 or not all(x.isdigit() for x in ids):
        raise RuntimeError("Missing or invalid runtime PID files")
    player, sink = ids
    command = f"test -d /proc/{player} && test -d /proc/{sink} && cat /proc/uptime /proc/stat /proc/net/udp /proc/net/dev /proc/meminfo /proc/{player}/task/*/stat /proc/{player}/task/*/status /proc/{sink}/task/*/stat /proc/{sink}/task/*/status"
    data = ssh(host, command)
    result = {"player": int(player), "sink": int(sink), "threads": {}, "memory_kib": {}}
    current = None
    for line in data.splitlines():
        if re.fullmatch(r"\d+\.\d+ \d+\.\d+", line):
            result["uptime"] = float(line.split()[0])
        elif line.startswith("cpu "):
            result["cpu"] = list(map(int, line.split()[1:9]))
        elif re.match(r"\s*\d+: [0-9A-Fa-f]{8}:0404 ", line):
            result["rtp"] = parse_rtp_line(line)
        elif re.match(r"\s*wlan1:", line):
            result["wlan1"] = parse_wlan1_line(line)
        elif re.match(r"\d+ \(.*\) ", line):
            tid = int(line.split()[0])
            fields = line.rsplit(") ", 1)[1].split()
            result["threads"][tid] = {"ticks": int(fields[11]) + int(fields[12]),
                                      "state": fields[0]}
        elif line.startswith("Pid:"):
            current = int(line.split()[1])
        elif line.startswith("voluntary_ctxt_switches:") and current in result["threads"]:
            result["threads"][current]["switches"] = int(line.split()[1])
        elif line.startswith(("MemAvailable:", "CmaFree:", "CmaTotal:", "SUnreclaim:")):
            name, value, _ = line.split()
            result["memory_kib"][name.rstrip(":")] = int(value)
    if "rtp" not in result or "cpu" not in result:
        raise RuntimeError("No live RTP socket or CPU data; inspect lifecycle before restart")
    return result


def delta(old, new):
    elapsed = new["uptime"] - old["uptime"]
    cpu = [b - a for a, b in zip(old["cpu"], new["cpu"])]
    total = sum(cpu)
    if elapsed <= 0 or total <= 0:
        raise RuntimeError("Invalid sampling interval")
    same = old["rtp"]["inode"] == new["rtp"]["inode"]
    threads = {}
    for tid, values in new["threads"].items():
        before = old["threads"].get(tid)
        if before:
            threads[tid] = {"cpu_pct": round(100 * (values["ticks"] - before["ticks"]) / total, 2),
                            "voluntary_per_s": round((values.get("switches", 0) - before.get("switches", 0)) / elapsed, 2)}
    rx_mbps = None
    if "wlan1" in old and "wlan1" in new:
        rx_bytes = new["wlan1"]["rx_bytes"] - old["wlan1"]["rx_bytes"]
        if rx_bytes >= 0:
            rx_mbps = round(rx_bytes * 8 / elapsed / 1000000, 3)
    return {"elapsed_s": round(elapsed, 2), "player": new["player"], "sink": new["sink"], "wlan1_rx_mbps": rx_mbps,
            "busy_pct": round(100 * (total - cpu[3] - cpu[4]) / total, 2),
            "rtp_drops_delta": new["rtp"]["drops"] - old["rtp"]["drops"] if same else None,
            "rtp": new["rtp"], "threads": threads, "memory_kib": new["memory_kib"]}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--host", default="10.0.0.234")
    parser.add_argument("--interval", type=float, default=10)
    parser.add_argument("--count", type=int, default=3)
    args = parser.parse_args()
    if args.interval <= 0 or args.count < 1:
        parser.error("positive interval/count required")
    before = snapshot(args.host)
    for _ in range(args.count):
        time.sleep(args.interval)
        after = snapshot(args.host)
        print(json.dumps(delta(before, after)), flush=True)
        before = after


if __name__ == "__main__":
    main()
