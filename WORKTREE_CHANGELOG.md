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
