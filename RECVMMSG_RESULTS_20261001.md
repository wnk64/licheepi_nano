# Bounded UDP receive results, 2026-10-01

Base source21afbcd, tag sink-before-recvmmsg-20261001; plan1441615,
source5dc6425, build record02c4fef. Same original lazycast source tree.
Build: make -f Makefile.sink TARGET=/tmp/f1-rx-sink-arm -B (and second
target), Buildroot GCC6.4 ARMv5 toolchain. Both MD5
c4cfdcddbedbb1851e65fbe0b2cb3e52. Runtime WFD_RTP_BATCH=1/WFD_LOSS_IDR=1.
Versioned target /root/aic_miracast/candidates/miracast_recvmmsg_20261001/sink-recvmmsg.
Rollback original273b1d0a at miracast_loss_idr_20261001/sink-loss-idr.

Actual socket helper and existing RTP-byte/EPIPE tests ASan/UBSan pass.
Mock M1-M7 800 and loss-IDR/cooldown/duplicates/reordering tests pass.
First profile wrapper omitted800 argument; corrected and entire test rerun
passed before source commit/build/deployment. No untested deployment.

Same player a683cf70, raw800x48060, rotate90/native480x800/pool9/defaultaudio.
Both candidate connections controlled by ADB only, media settings untouched.
Same boot ea1b461e-c82b-4fec-9d32-5256689ece80. No kernel/driver/HCI reset.

| Stream | Span(s) | Video commits/s | Gaps >250ms | Max gap(us) |
| --- | ---: | ---: | ---: | ---: |
| Old sink baseline | 636.24 | 40.69 | 106 | 1784684 |
| Batch run1 | 170.32 | 45.39 | 0 | 200550 |
| Batch reconnect run2 | 210.47 | 48.16 | 1 | 1083776 |

Successful ioctls, not optical scanout proof. Loop phases/durations differ;
do not claim an exact percentage performance gain or all stalls resolved.
Baseline immediate samples12.81/12.51s,6.061/6.033Mbps,CPU97.50/95.59%,
localdrops330/437. Run1 six13.11..14.01s windows CPU87.42..96.06%,
drops34/0/0/4/0/0. Run2 six12.98..18.23s windows CPU86.41..93.03%,
drops55/8/0/28/0/1 at5.017..6.034Mbps. Audio counters rise in both runs.
Receive batches average~13datagrams/call; fallback0/truncated0. FIFO full0,
submitted=written at reports. Available RAM~17.5MiB, CMA24MiB unchanged.

Current player6796/watch6797/sink6846, original GO/WPA6960/DHCP12705/PBC7006.
Current logs player-frame-gap.log/sink.log under manual_miracast_20260920.
Decision: promising runtime improvement, keep candidate for more testing;
not accepted stable, cold regressions0/3, no stable archive/GitHub push.
Next: investigate bounded FIFO coalescing to reduce worker~700..800wakeups/s
without byte loss, unbounded buffering or appreciable video latency.
