# 2026-10-01 新播放器投屏回归

计划：在当前#250/单高度fbdev/CMA24MiB上恢复既有手动投屏流程，仅选用退出修复候选e1d6fed2782154bcf934fee67a298813，不升分辨率、不接音频、不改驱动/内核/协议/开机服务。

启动ID ea1b461e-c82b-4fec-9d32-5256689ece80，管理SSH10.0.0.234/rtlwlan0，默认路由10.0.0.1。AIC已8d83/wlan1；启动前没有投屏WPA/GO/DHCP/sink/player。

固定组件：WPA a9ae2b53fc4a3469fad24fd1abbd42fb、CLI97f7384a530c9514f75533ab9e530a56、DHCP2e8c7e6533eeb85095799092da06707f、sink ceeeb9958f8f84f2a0b7c6c3df63db0e。WPA配置300e8b756b24d8e3e9e0288e7b8e4f54，PBC监听32a3fa3d7f7d100a38364299cda62e08，RTSP监听905e81e9545d5b20b6a18f2081161696，均沿用新板成功记录，监听器只在当前测试期间保留。

运行/tmp/manual_miracast_20260920，FIFO/tmp/aic_h264_live.fifo；新播放器/root/aic_miracast/candidates/player_lifecycle_20261001/player-lifecycle，CEDAR_CMA_POOL_MB=7 CEDAR_ROTATE=90 CEDAR_AUDIODEV=default --raw-h264 640 480 60。90度输出原生480x640居中，无缩放。GO5745MHz，手机请求后开启PBC、DHCP成功后启动ARM sink；H.264不落视频文件。

回退：仅停止该目录pid文件对应的本次player/sink/watch/DHCP/WPA，撤销wlan1 GO；不得停止rtlwlan0管理supplicant或改默认路由。原播放器1259e0f3/11386ab路径保留。当前仅准备，手机连接、持续动态画面/音视频资源共存、停止/重连与完整三冷启动验收待测0/3，不称稳定、不推GitHub。

## 已启动，等待手机连接

协议文件远端哈希与上次记录一致，WFD 0/1/6/11及设备设置全部OK；GO5745MHz/COMPLETED/AP-ENABLED，SSID DIRECT-fI，投屏名称F1C200S-AIC。WPA PID6960，候选播放器6974，DHCP7001，PBC监听7006，RTSP监听7007；当前没有sink PID，DHCP日志仅初始化，尚无手机REQUEST。

播放器初始化为raw640x480@60、硬件90度旋转480x640原生显示，7MiB池已分配，等/tmp/aic_h264_live.fifo。GO准备后MemAvailable20544KiB、Slab12496KiB（不可回收11216KiB）、CmaFree12008KiB；不是解码峰值或泄漏证据。默认路由仍rtlwlan0/10.0.0.1，只新增wlan1直连网段；默认声卡headphone，raw视频-only未接音频。启动ID不变，没有重启/内核错误证据。

协议运行态监听器需要保留以接收手机连接；本轮一次性启动包装不保留。手机连接、持续出帧和视觉验收待测，不宣称投屏成功。计划记录提交cd0b01a，未改协议脚本内容，未替换任何旧驱动/播放器。
