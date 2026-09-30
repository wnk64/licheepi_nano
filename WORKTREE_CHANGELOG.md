# Worktree Change Log

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
