# Worktree Change Log

Worktree: /home/wnk/LicheePi_Nano/third_party/lazycast_host_20260721
Component: Miracast sink
Base: 4f4a2cc plus previously untracked miracast_sink_dump.c
Purpose: preserve exact 640-stage source before 800x480 negotiation candidate

## 2026-10-01 / baseline established / reproducible lowest sink
Existing source MD5f1a08e9f4fc8e645842bd78f400ec127; standalone libc-only
program, original directory retained. Buildroot2018.02.11 GCC6.4.0:
arm-buildroot-linux-gnueabi-gcc -Os -fno-builtin -march=armv5te
-o miracast_sink_dump.lowest.brarm miracast_sink_dump.c.
Exact resulting MD5ceeeb9958f8f84f2a0b7c6c3df63db0e matches active board.
Initial default-architecture builds differed; do not infer source identity
from adjacent files. With explicit armv5te it is byte-reproducible.
Track original source and Makefile.sink before edit; preserve unrelated files.
Archive full Git bundle, original source/build recipe/exact binary separately.
640 stage uses player e1d6fed2, #250/fbdev100/CMA24MiB, current AIC rxguard,
5745MHz GO and FIFO. User requests stage backup, not stable certification.
Board snapshot MD59a797dc49737a464135f25f07122bf83 in Windows and VM verified.
No video capture or private management Wi-Fi credentials archived.
No live process interrupted yet; no new worktree or GitHub push.

## 2026-10-01 / planned / prefer handheld800x480p60
Base05324af/tagmiracast-sink-640-stage-20261001, exact binaryceeeb995.
Original worktree branchcandidate/miracast-hh800-20261001, no new worktree.
Only M3 wfd_video_formats native00->0a (HH1), HH bitmap0->2 (800x480p60).
Keep mandatory CEA0 fallback and all codec/RTSP/RTP/PES fields unchanged.
Mode mapping verified in existing miraclecast src/ctl/wfd.c HH index1.
Phone M4 and decoded picture must actually select800x480, never count640
fallback as800 or scale it. Isolated runtime watcher only changes SINK path.
Use existing e1d6fed2 player raw80048060/rotate90/native480x800, pool9MiB
to accommodate larger frames; CMA24MiB/kernel#250/DTB/AIC/BS/BT unchanged.
Record pool peak/fallback and ordinary RAM after phone starts; 9MiB is a
candidate budget, not a guarantee. Restore immutable640 snapshot on failure.
Tests: byte diff, deterministic ARM builds, mock M1-M7 negotiation, realphone.
Decision: pending, no stable certification or GitHub push.
