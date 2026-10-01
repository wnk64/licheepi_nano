# Worktree Change Log

## 2026-10-01 / planned / bounded WFD LPCM to ALSA default
Base67ad37c/b96fa528, tag sink-hh800-before-audio-20261001, complete source
and binary archived under miracast_audio_20261001/before; original tree kept.
Existing M4 already selects LPCM48k stereo; reuse current video TS/FIFO path.
Add bounded TS/PES LPCM demux (PID1100, 65541byte PES ceiling) and separate
ALSA nonblocking worker (16KiB/85ms PCM ring, 64KiB stack,40ms requested buffer).
PCM framing verified against AOSP ESQueue::dequeueAccessUnitPCMAudio:
a0/numAUs/reserved/11, 80 samples per AU, stereo16bit big endian.
ALSA plug performs endian conversion; default selects connected Wt-070 A2DP.
Opt-in WFD_AUDIO_ENABLE=1, no audio files/FIFO backlog/no AAC decoder; drop
old audio on overflow rather than block video. Stop/drop instead of drain.
Tests: all184 header split points, duplicates/CC loss/resync/TEI/invalid
format/fuzz, ASan/UBSan and ALSA-null threaded lifecycle; unchanged M1-M7.
No kernel/DTB/CMA9MiB pool/rotation/driver/audio-route changes. Live source
has already sent TEARDOWN, sink0; no agent stop/reboot. Need phone reconnect
and audible/sync/memory/long-run verification; three cold cycles pending0/3.

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

Sourceaaef280, ARM sinkb96fa52854a2255555ab40c9c771496d, watcher27055c3b.
Two builds identical; native mock M1-M7 for640 and800 profiles pass, no video
acceptance inferred from mocks. Isolated board candidate deployed, oldceeeb995
and640 snapshot untouched. Complete640 checkpoint248MiB SHA25665fb6546fbd445447aa5a3beafc6975a3b78296bfc6d34c5237314da83b901f1
verified in Windows/Ubuntu before switching. 640 sink exits0, poolpeak6799360,
fallback0/live0; no board reboot. DHCP7001 blocksrecv onTERM, scoped KILL after
checking executable; no DHCP code/config changes. New GO5745MHz DIRECT-Ij;
player12679/sink13259, pool9MiB/raw80048060/rotate90/native480x800.
User confirms success. Real phone M4 HHmask00000002/CEAmask0; decoded frame
480x800 native/no scaling. Same#250/bootea1b461e-c82b-4fec-9d32-5256689ece80.
Streaming snapshot MemAvailable20008/MemFree14788/CmaFree9968KiB;
ordinary free4820KiB, Slab12592/SUnreclaim11312KiB; compared64022292KiB
available, decrease2284KiB. Pool reserved9MiB, peak/live usage not measured
without stopping; no claim of peak margin from CmaFree. Keep stream active.
Decision: user-confirmed initial800 success; duration/reconnect/three cold
cycles still pending0/3. No audio pipeline change, no GitHub stable push.
# 2026-10-01 audio candidate deployment (pending)
Source commit: 1e92840, branch candidate/wfd-lpcm-audio-20261001.
Host ASan/UBSan/LPCM fragmentation/fuzz/ALSA-null lifecycle tests passed;
mock M1-M7 HH800 profile passed. Two ARM builds byte-identical.
Board sink MD5: 2abf061085a4f828af9b6c1c6124bb1a.
Watcher MD5: 42f8ae3cba198044164ba043f38c24cf.
Deployment: /root/aic_miracast/candidates/miracast_audio_20261001/.
Pure-video watcher MD5: 27055c3b9291a15f35d13bc4793ce577, preserved at
/root/aic_miracast/candidates/miracast_hh800_20261001/rtsp-watch.before-audio.sh.
Original video sink b96fa52854a2255555ab40c9c771496d remains untouched.
Rollback: after graceful candidate shutdown, restore that watcher to
/tmp/manual_miracast_20260920/rtsp-watch.sh and restart player/watch for
a fresh phone connection. Do not copy a watcher over a live watcher process.
Current player16712/watch16713; existing WPA6960/DHCP12705/PBC7006 retained.
Player e1d6fed2, pool9MiB/raw80048060/rotate90/native480800/FIFO unchanged.
Default route is Bluetooth Wt-070 12:11:71:41:9C:4A.
No kernel/driver/DTB/CMA-total/audio-route changes; no reboot/active-stream kill.
Real phone reconnect, LPCM headers, audible output, sync, RAM and stability
are unverified. Three complete cold boot cycles remain 0/3, no stable push.
Helper final diagnostic failed because ip is absent after launch; processes
and artifact hashes independently verified. No launch failure inferred.
# 2026-10-01 planned: RTP-scoped FIFO write batching

## 2026-10-01 / ADB-control verified / original candidate unchanged
Plan promote tested local ADB-shell transport helper into tools/adb_cast_control.py
on existing tree, no new worktree; existing authorized server5037 only, no
new adb keys/config/service. Native CLI fails android-home, protocol works.
Phone dda57287/model23013RK75C: openCAST_SETTINGS, dumpXML, choose enabled
F1C200S-AIC98:a1:4a:08:13:6a bounds76,759..387,825, tap350,817 succeeds.
Restart ended player/sink with unchanged async99a700f8, same watchdog/runtime.
Player22623/watch22624/sink22715; phone activeDisplayState2 and
mRemoteDisplayConnectedtrue; RTSP negotiated, player decoded native480800.
Settings-page stream first4096RTP socketdrops0/audio discontinuities0,
queue peak1808/full0; this low-motion stream is not a video-stability result.
No driver/kernel/CMA/player/source changes besides helper/docs. Current
stream remains active. Phone dump under/data/local/tmp removed per read.
Local one-off probe/start command removed; reusable helper intentionally
retained for next automatic reconnect/test, use Python and -- before --ui.
Rollback same unchanged candidate; threecold0/3 and stall fix unaccepted.

## 2026-10-01 planned: bounded asynchronous FIFO delivery
Deployment: user disconnected, old sink20548 exited0. New player21525/
watch21526 waiting new DHCP after11lines; sink99a700f8e1592a68d06d608ff983dfdd
and watcher2ad42f12091f64f548df07dc36fe9e8f verified overSSH. Build/docs3f26038.
Candidate /root/aic_miracast/candidates/miracast_async_fifo_20261001/.
Rollback old sink-burstf8a1058c untouched; watcher rtsp-watch.before-async.sh
retains WFD_SINK override, so restore with WFD_SINK pointing to old sink-burst
(otherwise defaults to observer). Save pretest logs sink-burst.log/player-burst.log.
Runtime/tmp only, no reboot or driver/boot-service changes. Live queue/CPU/
RTP-drop/A/V performance still pending. Temporary helpers cleaned after use.
Implemented1de102b, exit-status fix083544c (explicit failed output returns1,
not normal0). Host20lifecycle/exact-order/wrap/full/EPIPE/stop tests pass
ASan/UBSan/leaks; existing RTP batch sanitizer and mock800 pass. Final ARM
two builds equal99a700f8e1592a68d06d608ff983dfdd; final host regression rerun
before staging. Old intermediate22b0eab is not deployed. g_h264_bytes in
async mode counts submitted bytes; async written counter is actual output.
Watcher enables WFD_VIDEO_ASYNC1 and chooses versioned sink by WFD_SINK;
old runtime unchanged awaiting user disconnect, no force kill/reboot.
Base7bc75cf/source548f939/f8a1058c and complete archive retained.
Original branch candidate/async-fifo-20261001, no new worktree. Hypothesis:
FIFO writes in receive loop delay audio/RTP; independent output worker can
absorb bursts while preserving exact stream bytes. Opt-in WFD_VIDEO_ASYNC=1.
128KiB ring +4096byte in-flight block, 64KiB thread stack; nonblocking write
and50ms poll, cancellable stop/join. Queue full causes explicit candidate
failure, never silently discard H264 fragments. Stop may discard queued
tail after disconnect; report it. No video file/unbounded cache added.
Keep800/native90/player/CMA/audio/driver/sysctl unchanged, diagnostic receive
counters and360448socket capacity retained for same-base comparison.
Tests byte order/wrap/slow-consumer/full/EPIPE/idle-stop/full-pipe-stop,
ASan/UBSan/leaks and800RTSP; no acceptance from tests. Live reconnect needed,
monitor queue peak/drop/CPU/RAM/A/V delay. Rollback sink-burstf8a1058c.
Shared FIFO/player/network/audio scheduling; threecold0/3, no stable push.

## 2026-10-01 planned: observe live FIFO/receive stalls
Observed live: phone TEARDOWN/sink0, 21822RTP packets, socket drops1550;
receive gap max108226us. FIFO single-write max19838us, no >20ms, interval
write totals sum12.97s. Cannot establish multi-second FIFO blockage from this.
Pool peak8437760/cap9437184/fallback0/live0, clean lifecycle, no OOM.

## 2026-10-01 planned: bounded per-socket receive burst capacity
Source548f939, plan178de23, watcher89d3c9e; ASan/UBSan byte/EPIPE and
mock800 pass, two ARM builds equal f8a1058ccde51d0f44c8ac2f2e718cc9.
Deployed versioned sink-burst, remote hash verified, watcher64148622ebaaede823b023a7dc9e7ec0.
Player20413/watch20414/sink20548, phone connected, originalWPA/DHCP/PBC.
Actual SO_RCVBUF effective360448 confirmed in log. No sysctl/driver changes.
Uptime12539.69..12549.93 (10.24s): socketinode111433 drops1390->1522
(+132), CPU1009ticks/user330/system569/softirq1/idle110 =>89.1%busy.
Intervals include FIFOmax30587us, receivegap98221us; syscall wall duration
includes scheduling, not proof all that time is pipe-full blocking. Drop
counter cumulative1436 in ancillary, proc1522; no contradiction (async).
MemAvailable18992KiB, CmaFree9264KiB/total24576; video-clock late warnings.
Decision: insufficient to fix frequent stalls; not accepted. Different
content/time means no controlled percentage improvement from previous132/190.
Current stream kept active, no rollback interruption yet. Before next source
candidate, separate hypothesis and baseline required. Proposed next scope
is bounded receive/output separation; no unbounded/disk video buffering.
Rollback observerf2d95119 and candidate/rtsp-watch.before-burst.sh retained.
Base6c64cc1/dc42acc/f2d95119, exact source/build archived. Original tree
branch candidate/rtp-receive-burst-20261001, no new worktree. Only set
SO_RCVBUF requested180224 before bind; board default/max180224 and Linux
doubles explicit request, expected effective360448 (verify getsockopt).
No sysctl change; kernel memory is demand-driven, not all upfront; cannot
claim added video RAM equals requested socket-accounting bytes. Existing
FIFO/audio queues unchanged; buffering may add latency when overloaded.
Hypothesis: absorb ~100ms short stalls, not cure sustained CPU overload.
Keep diagnostic counters and test same media drops/gap/CPU/RAM/audible sync.
Rollback f2d95119 and observer watcher. Threecold0/3, pending, no stable push.
Deployment now active after user disconnect: player19151/watch19152, waiting
DHCP after7existing lines. Remote sinkf2d951196746d8ada684310d8c53f1f6,
watcher53c679dbce544d9d1a7a8e998ae7f0ff verified. Existing WPA6960/DHCP12705
and PBC retained; defaultBluetooth unchanged, no driver/kernel/reboot changes.
Rollback watcher copied to candidate/rtsp-watch.before-observe.sh, old
sink/player logs preserved there. Runtime/tmp only, no startup integration.
Performance diagnosis pending fresh phone stream. Temporary deploy helper
removed after successful execution.
Source dc42acc (plan e1a209e), ASan/UBSan batch-byte/EPIPE and mock800
tests passed; two ARM builds identical MD5f2d951196746d8ada684310d8c53f1f6.
Artifact staged and SSH remote MD5 verified, current live sink-batch18173
is not replaced yet. Waiting user disconnect; no stop/reboot. New watcher
candidate only changes sink path; original installed watcher is rollback.
Base3aff659/sourcea282376/db9732dd, exact source bundle and binary retained.
Branch candidate/rtp-stall-observe-20261001 in original tree, no worktree.
User reports frequent stalls; batching alone insufficient, not certified.
Only sink timing/counters: monotonic FIFO write duration and receive gaps,
socket-local SO_RXQ_OVFL ancillary counter via recvmsg, periodic summaries.
No new buffering, thread, socket capacity, scheduling priority, video codec,
player, driver or CMA change. Stats every4096RTP packets, not per-packet logs.
Test old byte-output/EPIPE sanitizer tests, mock800 RTSP, deterministic ARM.
Rollback sink-batch/db9732dd and current watcher remain untouched. Live
reconnect needed to deploy diagnostics; optimize only from measured evidence.
Shared dependencies FIFO/player/audio/network scheduling, threecold0/3.
Deployment verified: sinkdb9732dd9062657d84465281a9186d99,
watcher0c84cef40e4190957f423f4add99c982. User disconnected; old sink16874
exited0 and player released VE. New player17760/watch17761 waiting DHCP
after5existing lines; no forced kill/reboot/driver/service change.
Logs: /tmp/manual_miracast_20260920/player-batch800.log and sink.log.
Runtime under/tmp, not integrated into boot image. Pending reconnect and
performance comparison. Build/results record commit6bfaaab.
Source commit a282376, plan102e26d. ASan/UBSan exact-output/EPIPE tests pass,
mixed video/nonvideo TS six writes become one; short final RTP flush verified.
Mock800 M1-M7 passed; twice-built ARM MD5db9732dd9062657d84465281a9186d99.
Candidate staged to board versioned directory with matching MD5. User has
disconnected; switch uses same DHCP/PBC/protocol/player, only sink path.
Rollback old audio sink2abf0610 and rtsp-watch.before-batch.sh preserved at
/root/aic_miracast/candidates/miracast_rtp_batch_20261001/.
Runtime performance unmeasured; do not claim CPU or drop reduction from mocks.
Base f6bd85a/source1e92840, sink2abf0610, exact original source/build retained.
Original tree branch candidate/rtp-fifo-batch-20261001, tag
sink-audio-before-rtp-batch-20261001; archive miracast_rtp_batch_20261001/before.
User requests optimization after audible success with intermittent stalls.
10.62s diagnosis: CPU92% busy, board UDP RcvbufErrors+565; not WLAN proof.
Change only sink H264 output batching within each RTP packet, <=RTP_MAX
static scratch bytes; flush immediately per packet, no interpacket delay.
Keep PES extraction byte-identical, audio worker, RTSP, driver/player/CMA
and resolution unchanged. Test exact bytes across mixed TS/headers/tails,
write-count reduction, EPIPE and invalid RTP under ASan/UBSan; mock M1-M7.
Retain old board audio sink/watcher; runtime switch requires reconnect.
Shared dependencies: FIFO/player scheduling, audio delivery, network receive.
Acceptance: real A/V/counters/RAM and three cold cycles pending0/3.
