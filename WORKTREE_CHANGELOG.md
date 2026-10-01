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

## 2026-10-01 planned: observe live FIFO/receive stalls
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
