# Worktree Change Log

Worktree: /home/wnk/SoftWare/Driver software/aic8800_ugreen_v14_20260919/Linux/aic8800_linux_driver
Component: external AIC8800 USB driver
Base: UGREEN CM762 V1.4 source commit 35489f9c0370b4f6112634d6cc07c0690604bf4d
Target release: 5.7.1
Purpose: test source-reproducible AIC8800 loader/fdrv against #237 USB FIFO kernel.

## 2026-09-19 / rejected / UGREEN AP beacon DMA zero
Hypothesis: APM_START used uninitialized elem dma_addr after the beacon was sent separately.
Files: drivers/aic8800/aic8800_fdrv/rwnx_msg_tx.c
Commit: 7119ad1
Build: external Kbuild against linux_musb_clean_ep1_20260811, ARCH=arm, fixed Linaro prefix, LOCALVERSION=, CONFIG_PREALLOC_RX_SKB=n.
Artifacts: loader 5186d0db18ac146d2b940a54d11f226e; fdrv a3112f263e072a41dd8cb295f4ba00f6; vermagic 5.7.1 mod_unload ARMv5 p2v8.
Deploy: runtime-only under /root/aic_miracast/candidates/ugreen_v14_20260919; no system AIC files replaced.
Tests: loader passed 8d80 to 8d83; fdrv created wlan1 with wlan0 retained; GO 5805 caused board loss of SSH and serial. Cold power restored wlan0.
Decision: rejected for P2P GO.
