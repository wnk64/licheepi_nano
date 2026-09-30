# Worktree Change Log

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
