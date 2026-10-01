# Worktree Change Log

## 2026-10-01 planned: decoder idle backoff from original player
Base8f8a7b0/e1d6fed2, branchcandidate/decoder-idle-wait-20261001; prior AUD
candidate preserved but not stacked. Complete source/object/bundle and exact
binary copied to player_decoder_idle_20261001/before, no new worktree.
Dynamic decoder thread24795 had9228 voluntary switches in5.25s (~1758/s),
112CPUticks (~21%). Current per-loop50us sleep wakes even with no input.
Change decoder only: remove unconditional50us; if Cedar says NO_BITSTREAM5
or NO_FRAME_BUFFER4, sleep2000us then recheck stop/state/free pictures.
Keep successful decode timing/PTS/fps/native90/CMA/protocol/audio unchanged.
Add low-frequency result counts every4096calls and main exit state/signal
diagnostics for prior unexplained reader-close/EPIPE. No behavioral recovery
or silent resolution/driver changes. Record compile results then real same
phone video metrics, cancellation/returncodes. Kernel preflight exact5.7.1.
Automatic ADB cast-only disconnect/reconnect, no phone media/rotation/loop
operations. Rollback original e1d6fed2; threecold0/3, no stable push.

## 2026-10-01 / planned / ordered display-worker teardown
Original player tree /home/wnk/f1c200s_display_480x800_candidate_20260914,
base7263725/tagplayer-before-lifecycle-20261001, branchcandidate/player-lifecycle-20261001.
Full source/object snapshot and bundle at player_lifecycle_20261001/before;
make-B/strip exactly reproduces1259e0f3. Preserve dirty generated objects/boot.
Old mixed-object dtsview11386ab exits139 on SIGINT; current consistent1259e0f3
same MP4/SIGINT exits0. Existing drm destroy still closes fd/resources and
destroys live queues before joining its worker, with possible blocked return.
Change driver/drm_warpper.c only: stopflag/queueclose -> join -> drain metadata
and destroy queues -> free DRM resources -> close fd, idempotent destruction.
No header/layout change, decode/rotation/frame ownership/protocol/kernel unchanged.
Add mocked lifecycle test with a full return queue, delayed/blocking worker,
ASan/UBSan/leak detection, repeat destroy; baseline must fail ordering assertion.
Build consistent objects using existing player Makefile and Buildroot toolchain;
deploy isolated candidate, retain1259e0f3/11386ab. Board tests SIGINT/SIGTERM,
repeat MP4 rotation and FIFO/raw shutdown, returncodes/logs/memory. Pending,
no stable acceptance or GitHub push; full cold/regression gate remains0/3.
Stage1 source43d19d0/candidatec4efe667: ASan/UBSan50 cases pass, baseline
negativecontrol aborts134; two ARM builds identical. Board4 MP4 INT/TERM exits0,
but idle FIFO INT hangs in blocking open; own candidate forcibly stopped137,
no baseline target overwritten, stage1 not accepted for raw live use.
Follow-up same shutdown hypothesis: raw reader O_NONBLOCK plus100ms poll,
preserve regular-file EOF and FIFO writer-close EOF; check stop/error while
waiting for decoder stream space. No data format/frame parsing change.
Final sourcea57e979, ARM ELF e1d6fed2782154bcf934fee67a298813, two builds
byte-identical; host50 cases ASan/UBSan/leak detection pass, old negative134.
Deployed only /root/aic_miracast/candidates/player_lifecycle_20261001/player-lifecycle;
original1259e0f3 poolplayer and11386ab dtsview unchanged, protocol entry unchanged.
Board#250/e a1b461e boot unchanged (full ID ea1b461e-c82b-4fec-9d32-5256689ece80).
Eight tests pass rc0/no badfd/no timeout: MP4 INT/TERM repeated2x (800x480,
rotate90/native480x800/defaultaudio); raw FIFO INT/TERM with no writer and
with connected idle writer (640x480/800x480, pool7MiB). No actual streamed
raw frames in FIFO tests, no full Miracast/visual acceptance claim. MP4 parser
is configured to loop, so natural MP4 EOF exit is not claimed or changed.
Raw cases report peak1MiB/allocations1/fallback0/live0; post-tests ION total0,
orphaned0. Debugfs temporarily mounted by inspection, then unmounted.
Available RAM35260->35196KiB over8 cases, later35212KiB; no monotonic-leak
proof claimed from meminfo alone. All test processes/FIFO/logs removed.
Source changes remain only two C files plus mock test/records; same kernel,
DTB/CMA24MiB/fb0480x800/AIC/BS/BT. Decision: verified shutdown candidate;
phone-streaming, visual and three cold/regression cycles pending0/3, no push.

## 2026-10-01 / runtime pending / new player Miracast readiness
Plan sourcecd0b01a/MIRACAST_LIFECYCLE_20261001.md. Existing protocol/config
hashes unchanged; only use e1d6fed2 player candidate. Same#250/ea1b461e boot.
WFD setters allOK, GO5745MHz/DIRECT-fI/COMPLETED, deviceF1C200S-AIC;
WPA6960/player6974/DHCP7001/PBC7006/RTSPwatch7007, no phone DHCP/sink yet.
Pool7MiB/rotate90/raw640x48060/FIFO, no scaling or audio implementation change.
Available20544KiB before stream; default BS route and headphone preserved.
Runtime/tmp/manual_miracast_20260920 kept for connection; one-off wrappers
removed locally. Await phone/visual/long test; full acceptance0/3, no push.

## 2026-09-30 / planned / opt-in private CMA arena
Historical provenance resolved: reconstruct main.o using drm_warpper.h from
3993c9b parent; compile all other sources with current header. Exact ELF MD5
11386abeb2137bfb3f28c47832b05552 reproduced, not an approximated binary.
Save existing dirty source as a dedicated snapshot commit, preserving objects.
Candidate: Makefile header dependencies; optional raw-H264 CMA pool adapter.
One arena with page-aligned suballocations, physical/cache operations delegated
to the verified original ION adapter; no reference frame/VBV reduction.
InitializeVideoDecoder replaces vConfig.memops, so wrap the process-private
adapter table returned by MemAdapterGetOpsS, not only the caller config pointer.
Only explicit CEDAR_CMA_POOL_MB enables it, default remains original allocation.
Protected: active casting untouched, native90degree display, AIC/BS/BT/kernel.
Tests: allocator mock tests, deterministic ARM builds, isolated player candidate.
Rollback: active player11386ab, preflight complete archive; no default replacement.
Snapshot00bb212, immutable tagplayer-before-cma-pool-20260930; preserved user's
preexisting source changes separately from the arena implementationa9a4ad6.
Real-test sourced5254ca. ARM candidate1259e0f3f151a06fc2270ff885dc4443;
two make -B/-j4 and strip runs identical. Host ASan/UBSan tests pass, including
4 threads/2000 suballocations, alignment/reuse/full pool/native fallback and
invalid frees; no reference frame or VBV reduction.
Board MemAdapter MD5 f67be6bb553890ae32c35bdcf05fae72 matches build dependency.
First isolated real7MiB test while old decoder running failed with PFNs busy;
pool used native fallback; strict contiguous test aborted, OS released all
temporary allocations. Old playback unaffected; ION returned6799360 bytes.
User approved one interruption. Stopped old sink2234/player1719 gracefully;
ION0/CMAused1072 pages. Repeated real test succeeded: 33 suballocations,
CPU/VE offset and CPU reverse mapping/cache/reuse checks; peak6799360,
capacity7340032, allocations34, fallback0, live0 after release; CMAused1072.
Started candidate6588 with CEDAR_CMA_POOL_MB=7, 90degree/native640x480@60;
one arena reserved. RTSP watcher6589, same GO/AIC and BS route; awaiting phone.
No current default player overwritten, no boot/kernel/module changes.

Worktree: /home/wnk/f1c200s_display_480x800_candidate_20260914
Component: native Cedar rotation player
Base: 0569026 plus existing dirty source/objects; reproduction gate failed
Purpose: investigate clustered CMA allocation without disrupting active casting.

## 2026-09-30 / pending provenance / clustered decoder allocation
User requests concentrated player memory allocation. No functional source edit.
Protected: current AIC/sink/FIFO, hardware rotation/native geometry, BS SSH,
Bluetooth audio routing, kernel/DTB/modules. Board binary11386ab retained.
Preflight archive: /home/wnk/F1C200S_archives/player_before_cma_pool_20260930.
Build check: make -B -j4 && make strip with existing Buildroot toolchain.
Output dd4ba586 differs from board11386ab; only main.o differs from saved objects.
main.o old g_drm_warpper0x550 vs full-source rebuild0x558; historical3993c9b
adds two display geometry fields, Makefile does not track header dependencies.
Generated artifacts restored from our preflight archive; existing user dirty
source unchanged. No board deployment or code change, no GitHub push.
Decision: stop functional edit until an exact complete source/build baseline is
established. Detailed evidence and prospective arena constraints in
CMA_INVESTIGATION_20260930.md. No temporary helper retained.
