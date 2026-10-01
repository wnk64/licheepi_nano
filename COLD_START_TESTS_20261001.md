# Current Miracast cold-start tests, 2026-10-01

Original tree branch candidate/cold-start-workflow-20261001;
plan9de1e3e/sourcefed75fe/host acknowledgement timestampse2af014.
Launcher c312e492ada61bbcd2e916592ee498fe, checked on board. Templates copied
from current working runtime, not archived startup scripts, no home Wi-Fi creds.
Artifact preflight passed; cold-on-active-runtime correctly refused before
mutation. /tmp is tmpfs so cold runtime will be fresh.
Hot session mode starts player11613/watch11614/sink11663, ADB connected,
PCM73MB and advancing video, peak gap169792us at last check. Two samples
CPU91.31/86.04%,6.03/5.579Mbps, localdrops0/0, availableRAM~17.7MiB.
Prior stream saved2083.766915s(~34.7min):90095commits,2gaps>250ms,
max251974us/errors0. Do not claim absolutely no stalls or optical proof.

Selected sink42d0cfa8/playera683cf70, native800x48060/rotate90/pool9/defaultBT;
all3optimizations1. Keep same kernel/DTB/drivers/firmware/audio route/BS SSH.
Each test: phone disconnect, wait processes terminal, sync, verified COM3
OFF/ON5s interval, COM4 boot capture, different boot ID, BS SSH/release/#250/
native framebuffer, existing exact AIC registration/GO5745/WFD/PBC/DHCP,
pairedBluetooth default route, phone connect, actual frame/PCM progression,
RTP/localdrops/CPU/RAM/ION/CMA/error observation. No driver substitutions.
The hub tool uses the provided skill script with only current port override.

## Cycle1 / planned
No cold cycle accepted yet. Physical serial/power and all functional results
must be recorded before any count; visual/long-duration acceptance remains
separate from commit counters or process liveness.
