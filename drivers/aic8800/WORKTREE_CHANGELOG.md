## 2026-09-22 / planned / AIC USB disconnect submitted-URB teardown
Hypothesis: During hub disconnect, aicwf_usb_disconnect frees URBs from free lists while submitted RX URBs are still being handled by usbcore. COM6 captured a SLUB BUG in kfree from aicwf_usb_free_urb after the MUSB unlink guard avoided the earlier usb_hcd_check_unlink_urb NULL dereference.
Files: aic8800_fdrv/aicwf_usb.c, WORKTREE_CHANGELOG.md
Commit: pending
Build: same 64-buffer 20KiB preallocation loader/fdrv pair for 5.7.1; no aggregation-size or protocol change.
Deploy and rollback: isolated candidate only; retain pool64 module pair and #244 MUSB zImage backup.
Tests: loader pool=64, wlan1/GO, PBC/DHCP/RTSP/RTP, controlled disconnect, no usb_hub_wq Oops or aicwf_usb_free_urb SLUB BUG.
Decision: pending
# Worktree Change Log

Worktree: /home/wnk/SoftWare/Driver software/aic8800_ugreen_v14_20260919/Linux/aic8800_linux_driver
Component: external driver
Base: 35489f9 source-reproducible UGREEN AIC8800 V1.4 source; current dual-OUT source state includes c228e5f
Target release: 5.7.1
Purpose: 在不改动内核和系统模块的前提下，验证源码构建的 AIC8800 双 OUT 候选模块能否稳定注册 wlan1。

## 2026-09-20 / rejected / UGREEN 双 OUT 运行时加载验证
Hypothesis: 源码构建的 loader 5186d0db18ac146d2b940a54d11f226e 与 fdrv e68b49a3f30d8c061334987f1a237720 可在 #241 基线上完成 8d80 -> 8d83 -> wlan1，且不触发先前二进制 fdrv 的 IRQ 崩溃。
Files: WORKTREE_CHANGELOG.md
Commit: pending
Build: 已使用受控命令构建；模块 vermagic 已记录为 5.7.1 mod_unload ARMv5 p2v8。
Deploy and rollback: 仅从 /root/aic_miracast/candidates/ugreen_dual_out_20260920 临时 insmod，不覆盖 /lib/modules；失败时重启恢复冷启动 8d80 基线。
Tests: 先验证 loader、USB PID、fdrv、wlan1；通过后才运行协议脚本和手机连接测试。
Decision: rejected for GO/Miracast; registration-only result is reproducible but not accepted.

结果更新：commit 8a1c889。板端 #241 冷启动基线已核验为 a69c:8d80、wlan0、无 AIC 模块。候选目录三件套 MD5 分别为 loader 5186d0db18ac146d2b940a54d11f226e、fdrv e68b49a3f30d8c061334987f1a237720、firmware 01acfbebdfb15755e3fe853e7bc95c7d。loader insmod 返回 0 并注册 8d83；fdrv insmod 返回 0 并注册 wlan1，wlan0 保持在线。协议脚本 wpa 阶段完成 WFD 配置；go 阶段的 p2p_group_add freq=5805 后 SSH 控制链路超时，未到 DHCP、手机关联或 RTSP。使用 COM5 收到 OK CH2 OFF 和 OK CH2 ON 后冷启动恢复原基线。未覆盖 /lib/modules，未部署内核。
Decision: rejected for GO/Miracast; registration-only result is reproducible but not accepted. Rollback: physical cold boot via COM5 restores 8d80 baseline.

## 2026-09-20 / planned / GO 建组串口证据采集
Hypothesis: GO 建组失联发生于 AIC AP/beacon 或第二个 bulk OUT 端点活动时；同步 COM6 日志可以区分内核 panic、USB reset、AIC driver error 和纯 WLAN 断链。
Files: WORKTREE_CHANGELOG.md
Commit: pending
Build: none.
Deploy and rollback: reuse only the isolated ugreen_dual_out_20260920 artifacts and protocol-only script; no system file replacement. Failure rollback is COM5 cold boot.
Tests: record COM6 from before p2p_group_add freq=5805 until completion or loss of SSH, then verify cold baseline.
Decision: pending

## 2026-09-22 / rejected / 5 GHz GO console capture
Test: cold boot; current UGREEN loader 5186d0db18ac146d2b940a54d11f226e, fdrv e68b49a3f30d8c061334987f1a237720, firmware 01acfbebdfb15755e3fe853e7bc95c7d; 8d80->8d83->wlan1 passed; WFD passed; p2p_group_add freq=5805 returned OK; COM6 then recorded only netlink attribute type 213 invalid length and chan.flags 0 before SSH and console became silent; DHCP, RTSP and player were not started; COM5 cold boot restored 8d80/wlan0/no AIC modules.
Decision: rejected for 5 GHz GO/Miracast. Evidence localizes failure after APM_START_REQ reaches firmware/USB confirmation path; no speculative beacon or command-queue source change.
中文结论: 本轮仅完成串口证据采集，5 GHz GO 失败后已物理冷启动恢复基线，未修改驱动 C 代码。
## 2026-09-22 / planned / F1C200S 8KiB USB RX aggregation
Hypothesis: D81 forced 20KiB GFP_ATOMIC RX skb allocation fails while native rotation is active; 2KiB eliminates allocation failure but prevents RTSP. An 8KiB aggregation buffer may retain RTSP while avoiding the 20KiB allocation failure.
Files: aic8800_fdrv/Makefile, aic8800_fdrv/aicwf_usb.h, WORKTREE_CHANGELOG.md
Commit: pending
Build: exact 5.7.1 external-module command with CONFIG_F1C200S_USB_RX_AGGR_8K=y and CONFIG_PREALLOC_RX_SKB=n.
Deploy and rollback: isolated board candidate only; restore fdrv e68b49a3f30d8c061334987f1a237720 after any failed registration, RTSP, stream, or control-path test.
Tests: 8d80->8d83->wlan1, 5745MHz GO, PBC, DHCP, RTSP, RTP continuity, native rotated display, wlan0 control path.
Decision: pending
## 2026-09-22 / planned / F1C200S bounded preallocated RX pool
Hypothesis: 20KiB aggregation frames require the original buffer length, but per-frame GFP_ATOMIC allocation fails after decoder CMA allocation. F1-specific preallocation of 64 x 20KiB buffers removes the atomic allocation while avoiding the original 1000-buffer 20MiB pool.
Files: aic_load_fw/Makefile, aic_load_fw/aicwf_rx_prealloc.c, WORKTREE_CHANGELOG.md
Commit: pending
Build: CONFIG_PREALLOC_RX_SKB=y and CONFIG_F1C200S_PREALLOC_RX_SMALL=y for loader and fdrv; no RX aggregation size override.
Deploy and rollback: isolated candidate only; restore loader 5186d0db18ac146d2b940a54d11f226e and fdrv e68b49a3f30d8c061334987f1a237720 after any registration, GO, or stream failure.
Tests: loader pool count, 8d80->8d83->wlan1, 5745MHz GO, PBC, DHCP, RTSP, RTP continuity, native rotation, control WLAN.
Decision: pending
## 2026-09-22 / planned / F1C200S non-realtime USB workers
Hypothesis: under continuous Miracast aggregation, the single-core AIC USB RX/TX SCHED_FIFO workers starve the system long enough to trigger the 16-second hardware watchdog. Keep the 64-buffer 20KiB preallocation and build only the fdrv with CONFIG_TXRX_THREAD_PRIO=n.
Files: aic8800_fdrv/Makefile, WORKTREE_CHANGELOG.md
Commit: pending
Build: loader with CONFIG_PREALLOC_RX_SKB=y and CONFIG_F1C200S_PREALLOC_RX_SMALL=y; fdrv same options plus CONFIG_TXRX_THREAD_PRIO=n. Keep 20KiB aggregation length.
Deploy and rollback: isolated candidate only; power cycle returns to 8d80 baseline; current pool64 modules are retained as rollback artifacts.
Tests: loader pool=64, wlan1, 5745MHz GO, PBC, DHCP, RTSP, RTP continuity, display, watchdog survival, wlan0 control path.
Decision: pending
