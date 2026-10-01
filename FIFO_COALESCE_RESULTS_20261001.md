# FIFO coalescing candidate results, 2026-10-01

Original lazycast tree; base681cddb and c4cfdcdd exact reproduced before edit.
Tag sink-before-fifo-coalesce-20261001, branch candidate/fifo-coalesce-20261001.
Plan6d8a1fb, production source df407c6, additional test7ba416f, recordb305f04.
Build: make -f Makefile.sink TARGET=/tmp/f1-coalesce-arm -B and second target.
Same GCC6.4 Buildroot ARMv5 toolchain. Both MD5
42d0cfa89120992a427abfde403770c9. Production changes only wfd_video.c.
No new worktree, kernel/driver/player change or stable GitHub push.
Target /root/aic_miracast/candidates/miracast_fifo_coalesce_20261001/sink-coalesce.
Rollback c4cfdcdd at miracast_recvmmsg_20261001/sink-recvmmsg unchanged.

WFD_VIDEO_COALESCE=1:4096byte threshold/fixed3ms monotonic intentional wait.
No deadline restart on spurious signals; scheduling latency not guaranteed3ms.
Empty->data/threshold/error/stop notifications only. Queue128KiB/scratch4KiB/
stack64KiB unchanged, no H264 files or random byte dropping.
20lifecycle/full/EPIPE tests off/on ASan/UBSan pass; new exact bytes/tail flush/
50stop/4KiB-no-wait/accounting tests ASan/UBSan pass (additional test3runs).
Mock800 M1-M7 and recovery-IDR pass with both batch and coalescing enabled.

Same a683cf70 player, native800x48060/90degree480x800/pool9/defaultBluetooth.
WFD_RTP_BATCH=1/WFD_LOSS_IDR=1 retained. ADB only operates connection;
phone foreground Bilibili and MediaSession state3/playing confirmed read-only.
Same boot ea1b461e-c82b-4fec-9d32-5256689ece80. No HCI reset needed.

| Test | Span(s) | Commit/s | Gaps >250ms | Max completed gap(us) |
| --- | ---: | ---: | ---: | ---: |
| Prior batch-only | 851.78 | 48.62 | 1 | 1083776 |
| Coalesce on1 | 240.45 | 43.00 | 0 | 156842 |
| Coalesce on2 after off control | 210.32 | 51.16 | 0 | 120992 |

These are successful video ioctls, not optical scanout. Variable source/content
phases and unequal durations prohibit exact performance-ratio or forever-
smooth claims. Same-binary off control full player-log transfer failed with
PSCP assertion1525 and zero-length destination; no full off-gap summary.
Off read-only live records show max733317us and CPU92.96..95.29%, FIFO worker
9.40..9.95%,738..820voluntary/s, UDPdrops0/11/108/0 per12.85..13.73s.
Off-sink.log retained; empty off-player.log removed, not fabricated data.
Later snapshot transfers use immutable small diagnostic log copies, removed
after successful SSH transfer; no video recording or system backups involved.

On1 six samples CPU82.94..89.31%,FIFO worker6.51..7.00%,304..341switches/s,
5.701..6.063Mbps, all6socket-drop deltas0. 50940video submissions become
18705actual successful FIFO writes; pending736bytes are queued, not discarded.
On2 six samples CPU81.20..87.08%,FIFO worker5.23..6.68%,276..339switches/s,
5.065..5.991Mbps, all6socket-drop deltas0. LPCM38502400->39844480bytes,
queue full0, errors0, availableRAM~17.7MiB/CMA total24MiB unchanged.

Current player8743/watch8744/sink8790 keeps playing, same GO/WPA/DHCP/PBC/BT.
Keep all3options enabled. Runtime diagnostics under manual_miracast_20260920.
Decision: promising repeatable runtime improvement, not accepted stable;
no long-duration/cold3boots/visual proof yet. Next preserve current path,
run longer and prepare exact reproducible cold-boot/startup regression.
