# Worktree Change Log

Worktree: /home/wnk/SoftWare/Driver software/aic8800_ugreen_v14_20260919/Linux/aic8800_linux_driver
Component: external driver
Base: 35489f9 source-reproducible UGREEN AIC8800 V1.4 source; current dual-OUT source state includes c228e5f
Target release: 5.7.1
Purpose: 在不改动内核和系统模块的前提下，验证源码构建的 AIC8800 双 OUT 候选模块能否稳定注册 wlan1。

## 2026-09-20 / pending / UGREEN 双 OUT 运行时加载验证
Hypothesis: 源码构建的 loader 5186d0db18ac146d2b940a54d11f226e 与 fdrv e68b49a3f30d8c061334987f1a237720 可在 #241 基线上完成 8d80 -> 8d83 -> wlan1，且不触发先前二进制 fdrv 的 IRQ 崩溃。
Files: WORKTREE_CHANGELOG.md
Commit: pending
Build: 已使用受控命令构建；模块 vermagic 已记录为 5.7.1 mod_unload ARMv5 p2v8。
Deploy and rollback: 仅从 /root/aic_miracast/candidates/ugreen_dual_out_20260920 临时 insmod，不覆盖 /lib/modules；失败时重启恢复冷启动 8d80 基线。
Tests: 先验证 loader、USB PID、fdrv、wlan1；通过后才运行协议脚本和手机连接测试。
Decision: pending

结果更新：commit 8a1c889。板端 #241 冷启动基线已核验为 a69c:8d80、wlan0、无 AIC 模块。候选目录三件套 MD5 分别为 loader 5186d0db18ac146d2b940a54d11f226e、fdrv e68b49a3f30d8c061334987f1a237720、firmware 01acfbebdfb15755e3fe853e7bc95c7d。loader insmod 返回 0 并注册 8d83；fdrv insmod 返回 0 并注册 wlan1，wlan0 保持在线。协议脚本 wpa 阶段完成 WFD 配置；go 阶段的 p2p_group_add freq=5805 后 SSH 控制链路超时，未到 DHCP、手机关联或 RTSP。使用 COM5 收到 OK CH2 OFF 和 OK CH2 ON 后冷启动恢复原基线。未覆盖 /lib/modules，未部署内核。
Decision: rejected for GO/Miracast; registration-only result is reproducible but not accepted. Rollback: physical cold boot via COM5 restores 8d80 baseline.
