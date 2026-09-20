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

## 2026-09-20 / planned / source-backed dual USB OUT fdrv
Hypothesis: the current 20260725 binary fdrv causes an IRQ/softirq NULL
pointer panic during AP association. Rebuild the UGREEN source candidate with
CONFIG_USB_MSG_OUT_EP=y so AIC data OUT and message OUT remain distinct.
Files: drivers/aic8800/aic8800_fdrv/Makefile.
Commit: pending.
Build: external Kbuild against linux_musb_clean_ep1_20260811, ARCH=arm,
fixed Linaro prefix, LOCALVERSION=, CONFIG_PREALLOC_RX_SKB=n.
Deploy: runtime-only candidate directory; no system AIC modules or firmware
will be replaced.
Tests: require loader 8d80->8d83, wlan1, WPA/P2P GO, DHCP, RTSP/RTP, and
wlan0/SSH regression. Do not use the protocol script to load driver modules.
Rollback: unload candidate modules or physical cold boot.

Commit: c228e5fc8e3a302a7f1198b1b591bbb128f943f5.
Build: make -C /home/wnk/LicheePi_Nano/linux_musb_clean_ep1_20260811
M=/home/wnk/aic8800_ugreen_v14_20260919 ARCH=arm with fixed Linaro prefix,
LOCALVERSION=, CONFIG_PREALLOC_RX_SKB=n, modules; passed with one existing
const-qualifier warning in rwnx_radar.c.
Artifacts: loader MD5 5186d0db18ac146d2b940a54d11f226e; dual OUT fdrv MD5
e68b49a3f30d8c061334987f1a237720; both vermagic 5.7.1 mod_unload ARMv5 p2v8.
Decision: build passed; runtime deployment remains pending.

Deploy: runtime-only /root/aic_miracast/candidates/ugreen_dual_out_20260920;
no system module or firmware file was replaced. Board hashes: loader
5186d0db18ac146d2b940a54d11f226e; fdrv
e68b49a3f30d8c061334987f1a237720; fmac
01acfbebdfb15755e3fe853e7bc95c7d.
Tests: pending loader and wlan1 registration.
\n## 2026-09-19 / planned / beacon payload bounds\nHypothesis: P2P/WFD beacon exceeds the fixed 512-byte APM_SET_BEACON_IE message array and memcpy corrupts the kernel message allocation.\nFiles: drivers/aic8800/aic8800_fdrv/rwnx_msg_tx.c\nProtected: #237 zImage, MUSB modules, board system AIC files, WLAN.\nRollback: runtime-only candidate directory; unload modules or cold boot.\n\n## 2026-09-19 / planned / single USB OUT queue\nHypothesis: AIC firmware exposes bulk OUT endpoints 1 and 2; CONFIG_USB_MSG_OUT_EP directs IPC/APM messages to endpoint 2 while F1C200S only has one 512-byte MUSB TX FIFO. Use endpoint 1 for both data and IPC messages.\nFiles: drivers/aic8800/aic8800_fdrv/Makefile\nProtected: #237 kernel, MUSB, DTB, board system AIC files, wlan0.\nRollback: runtime-only candidate modules; cold boot.\n\n## 2026-09-19 / rejected / single USB OUT queue\nHypothesis: use one logical USB OUT endpoint to avoid MUSB TX QH contention.\nFiles: drivers/aic8800/aic8800_fdrv/Makefile\nCommit: cbbf910\nBuild: fdrv 6ea8c42cb22c5bd27ab78a922a4e670a, vermagic 5.7.1 mod_unload ARMv5 p2v8.\nTests: loader reached 8d83, but fdrv timed out on MM_SET_STACK_START_REQ and did not create wlan1. wlan0 stayed online.\nDecision: rejected; AIC firmware requires its distinct message OUT endpoint.\n
