## 2026-09-30 / planned / PCF8563 RTC and IP5209 DT binding
Base:963147c with complete source/config and matching #245 kernel/DTB; local tag baseline-tca-before-rtc-ip5209-20260930 and archive /home/wnk/F1C200S_archives/rtc_ip5209_baseline_20260930.
Files:.config, drivers/rtc/rtc-pcf8563.c, Documentation/devicetree/bindings/rtc/pcf8563.txt, new suniv-f1c100s-licheepi-nano-rtc-ip5209.dts, this log and Chinese record.
Hardware:user schematic, PCF8563@0x51 on existing PE11/PE12 I2C0, alarmINT through PCA9555 P17/offset15 to PE5. CLKOUT feeds RTL8723DS, existing register0x0d=0xa4. IP5209@0x75 (manual8-bit0xEA), L3 unconnected, no IRQ invented.
Change:enable RTC_CLASS/PCF8563/HCTOSYS; honor preconfigured IRQ trigger instead of forcing LEVEL_LOW on a PCA953x edge-only child; optional nxp,keep-clkout skips the destructive probe CLKOUT clear and sets CLK_IGNORE_UNUSED. Add nodes in a separate DTS including the TCA candidate.
Protected:TCA GPIO/IRQ, existing AIC auto-network, USB/display/audio, IP5209 boost/battery chemistry settings.
Build:original Linaro7.2.1, exact release5.7.1 LOCALVERSION=, incremental zImage/modules/candidateDTB, tmux. External IP5209 module built in the original driver repository.
Deploy/rollback:keep #245 zImage and TCA DTB; SSH-only transfer/checksums and filesystem boot copy. User requested focused driver verification, no further network stress or cold-boot campaigns.
Source:b8b1d71; driverd7dfb13, startup94c2708. Numeric build rc=0; kernel5.7.1, zImage2b9b8cf3f41d88fc7182be2c63bcdf22, DTBc9904bd6d2e90634e165895928f804c6. Matched rebuilt sunxif9e4d79850a9ed80dbf8b965a2df2014 in both module paths; cedar53126e9c2cffa4c66e5634f99e15f3f9 and ionfbed3af9de750c73cc77d599e9c55601 in extra. Old files retained in /root/aic_miracast/candidates/rtc_ip5209_20260930/rollback; FAT hashes verified and unmounted before reboot.
Runtime:kernel5.7.1 #246, boot_id14a29e34-43f5-485b-8951-c17517b67ed2. RTC0-0051 and IP5209 0-0075 bind, /dev/rtc0 exists and alarm IRQ128 is nested PCA9555 offset15. RTC initial voltage-low flag cleared by setting PC UTC and writing /sbin/hwclock; time readback06:09:45 then06:12:04UTC confirms ticking. CLKOUT0x80 stays enabled32768Hz. IP power_supply reports battery voltage4506974uV, Charging, current limit2300000uA; VSET and charge/boost settings untouched. AIC automatic networking/SSH and existing panel/Cedar initialization succeed.
Decision:implemented/deployed/directly verified; provisional, no stable archive or GitHub push. Per user instruction, no repeated stress, alarm-firing or cold-boot campaigns; actual alarm firing and RTC backup retention not claimed.

## 2026-09-30 / planned / new board TCA9555 on I2C0
Base: 7d8b46c; running zImage MD5 6cdbe5ca0485dc187751f1dee1682a2c; DTB 81c73b94251e2e48e8ba1f65c082dbb5; both match this original source tree.
Source preservation: local tag baseline-newboard-before-tca9555-20260930 and source.bundle, .config, generated prerequisite tar, vmlinux, System.map, zImage and DTB at /home/wnk/F1C200S_archives/kernel_7d8b46c_newboard_20260930. No remote push per user instruction.
Hypothesis: the existing PCA953x driver supports TCA9555 register layout and nested GPIO IRQs when GPIO_PCA953X and GPIO_PCA953X_IRQ are enabled.
Files: .config; arch/arm/boot/dts/suniv-f1c100s-licheepi-nano-tca9555.dts; this log and Chinese change record.
Hardware: PE11=SCL, PE12=SDA on I2C0 at 0x01c27000 IRQ7; PE5=active-low INT. A0/A1/A2 grounded, address 0x20. External pullups confirmed by user.
Protected: existing display/audio/USB nodes and MUSB/AIC source; use a separate candidate DTS including the unchanged board DTS. No new worktree.
Build: incremental kernel/DTB, original Linaro 7.2.1 toolchain, ARCH=arm CROSS_COMPILE=/opt/gcc-linaro-7.2.1-2017.11-x86_64_arm-linux-gnueabi/bin/arm-linux-gnueabi- LOCALVERSION=; preflight and release 5.7.1 required.
Deploy and rollback: pending; original kernel and DTB preserved, SSH-only copy and hash verification before boot selection.
Tests: I2C0 probe, address 0x20 binding, 16 input lines, PE5 IRQ; new-board AIC SSH regression. Three physical cold boots pending; no old-board topology assumed.
Decision: pending.
Source commit: cf3cfbd; reproducible tmux build wrapper 0b74b53. olddefconfig selected only GPIOLIB_IRQCHIP plus the two requested PCA953x symbols; release exactly 5.7.1.
Build result: /tmp/f1-tca9555-build-0930.rc contains numeric 0; log ends Kernel: arch/arm/boot/zImage is ready. Built zImage MD5 a2f140a094dcf101cf63df0e83041cc8; candidate DTB MD5 3cb146b45c1a63fa1059ffbf996e0315. vmlinux contains pca953x_probe and gpiochip_irqchip_add_key.
Modules: sunxi.ko cf1d3dca576543161957efe0d33d121d and phy-generic.ko 3103db03a0bfe371ddd30922cfbeff28 remain unchanged; vermagic 5.7.1 mod_unload ARMv5 p2v8.
Deployment: old board zImage/DTB copied to /root/aic_miracast/candidates/tca9555_20260930/rollback; hashes match baseline. Candidate transferred via SCP, hashes verified; FAT files replaced, verified and unmounted before reboot.
Measured: software reboot kernel5.7.1 #245, boot_id 0dcac36c-971b-4729-af4d-f9c2f98969fd. i2c-0 and 0-0020 bind to pca953x; gpiochip400 exposes 16 lines, all read direction=in/value=1 via driver and were unexported after observation. PE5 IRQ69 registered as sunxi_pio_level hwirq37, handler0-0020, count1; I2C controller IRQ31 hwirq7, count319 after GPIO reads. This verifies registration and a startup parent interrupt, not a user input transition.
Regression: AIC auto-service boot success and SSH at 192.168.2.5. No output GPIO driven. Software-boot network regression121/121 replies, zero loss, min/avg/max3/16/513ms; 32MiB SSH download rc=0, same boot ID and USB device2, no new disconnect.
First user-confirmed physical cold reboot: boot_id f4e3790f-5c2e-452d-9232-a9b3a2e41dbf. AIC8d80->8d83 device3, automatic WPA/DHCP/SSH with no serial networking intervention. TCA0-0020/pca953x, gpiochip400/ngpio16 and parentIRQ69 persist. P00 edge=both successfully registered nestedIRQ128 (0-0020 offset0); direction=input, value1; edge removed and GPIO unexported afterward. No actual input edge generated; remaining cold cycles and input-transition test pending. Not stable acceptance.
Final cold-boot observation: 61/61 ping replies, zero loss, min/avg/max3/29/529ms. pinctrl reports PE11 and PE12 owned by1c27000.i2c with i2c0 function; PE5 owned by0-0020 with gpio_in function. Debugfs mount removed after inspection. No temporary GPIO exports remain.

## 2026-09-22 / planned / MUSB disconnect unlink guard
Hypothesis: AIC/RTL USB disconnect under Miracast calls musb_urb_dequeue with urb->ep already NULL. usb_hcd_check_unlink_urb dereferences urb->ep and crashes usb_hub_wq, as captured on COM6 at usb_hcd_check_unlink_urb+0x18 from musb_urb_dequeue.
Files: drivers/usb/musb/musb_host.c, WORKTREE_CHANGELOG.md
Commit: pending
Build: exact kernel 5.7.1 LOCALVERSION= command after preflight; retain matched sunxi.ko and phy-generic.ko artifacts.
Deploy and rollback: backup boot zImage, deploy candidate only through SSH, verify hash, retain rollback zImage; no raw partition write.
Tests: cold boot x3; hub/RTL/wlan0; AIC 8d80->8d83->wlan1; 5745MHz GO; PBC/DHCP/RTSP/RTP/native rotation; confirm disconnect no longer causes usb_hub_wq Oops.
Decision: pending
# Worktree Change Log

Worktree: /home/wnk/LicheePi_Nano/linux_musb_clean_ep1_20260811
Component: Linux kernel MUSB host driver
Base: source-reproducible SII9022 #236, commit 6d24e044a3cae9175e96d6d4688938992f079738
Target release: 5.7.1, runtime candidate #237
Purpose: keep F1C200S USB FIFO allocation inside 2 KiB and verify RTL8723BU Wi-Fi/Bluetooth coexistence.

## 2026-09-15 / accepted evidence / audit and bounded FIFO allocation
Hypothesis: the generic Sunxi table allocated FIFO addresses beyond F1C200S 2 KiB USB SRAM, causing endpoint aliasing.
Files: drivers/usb/musb/sunxi.c, drivers/usb/musb/musb_core.c
Commit: 1aad258, 9aab5c8, ca8a380, cd287e2
Build: documented Linaro top-level make with LOCALVERSION= and .version 236; release 5.7.1.
Deploy and rollback: board backups under /root/roms/kernel_backups/usb_fifo_audit_237_20260915 and /root/roms/kernel_backups/usb_fifo_fix_237_20260915.
Tests: boot log proved the original static FIFO offsets exceeded 2 KiB; bounded initial layout booted, associated wlan0, and completed a 4.9 MiB SCP transfer with matching MD5.
Decision: accepted as evidence. Initial bounded layout was not sufficient for HCI because its EP3 FIFO was 256 bytes.

## 2026-09-15 / rejected / broad RTL endpoint diagnostic
Hypothesis: record RTL endpoint-to-MUSB allocation before changing scheduling.
Files: drivers/usb/musb/musb_host.c
Commit: 716bc4c
Build: zImage 9e4281dbfa60018da27d6295bfde1943, release 5.7.1.
Deploy and rollback: old zImage e3081e59be6f9955a37eeec5d7208156 saved at /root/roms/kernel_backups/usb_fifo_endpoint_map_237_20260915/zImage.before.
Tests: the log included EP0 control URBs and flooded the console, preventing WLAN startup.
Decision: rejected. Board was restored through its saved zImage. Follow-up commits 63ed3df and a57eb52 restricted logging to RTL non-control IN endpoints.

## 2026-09-15 / pending candidate / 512-byte ACL receive FIFO
Hypothesis: allocate a 512-byte EP3 shared FIFO so a 512-byte RTL ACL IN URB has a direct non-mux hardware endpoint after Wi-Fi consumes EP2.
Files: drivers/usb/musb/sunxi.c; diagnostic-only drivers/usb/musb/musb_host.c
Commit: 9f8c01fe11e73b5b885237861303e3a87fcb99af
Build: make ARCH=arm CROSS_COMPILE=arm-linux-gnueabi- -j8 LOCALVERSION= with .version 236; zImage f38eda1a9de749d3fdaa3bfa0cf2becd; sunxi.ko 1434316e72399f410beb6490be897f4d; vermagic 5.7.1 mod_unload ARMv5 p2v8.
Deploy and rollback: candidate archive /home/wnk/F1C200S_host_archive/kernel_usb_fifo_acl_ep3_237_20260915; board backup /root/roms/kernel_backups/usb_fifo_acl_ep3_237_20260915 with prior zImage 952afe16475d07548e675afa91c849f1 and prior sunxi.ko 32914592dbc35e210dcfdeece483fe56.
Tests: active FIFO layout is EP1 TX 512/RX 64, EP2 shared 512, EP3 shared 512, total 1664 bytes including EP0. wlan0 associated. HCI initialized with RTL firmware, BLE discovery found Wt-070, and the final mapping during connection was HCI interrupt USB ep1 to MUSB EP1 plus ACL bulk USB ep2 to MUSB EP3.
Decision: pending baseline acceptance. Three physical cold boots and AIC8800/Miracast regression remain required.

## 2026-09-16 / pending runtime candidate / BlueALSA A2DP on Wt-070
Hypothesis: after the bounded FIFO fixes, real A2DP can coexist with WLAN when BlueALSA runtime and D-Bus policy are present.
Files: no kernel source change. Board runtime candidate: /tmp/bluealsa/runtime and /etc/dbus-1/system.d/bluealsa.conf.
Commit: runtime-only; source record is C:/Users/26301/Desktop/F1C200S_A2DP_HANDOFF.md.
Build: BlueALSA runtime from /home/wnk/LicheePi_Nano/third_party/bluealsa-f1c200s/bluealsa-f1c200s-runtime.tar.gz.
Deploy and rollback: runtime is under /tmp and disappears at reboot. D-Bus policy is a candidate persistent rootfs change and is not integrated into an image. BlueALSA ALSA plugin was copied only for the playback test and removed afterward.
Tests: Wt-070 12:11:71:41:9C:4A discovered, paired, trusted, connected, and services-resolved. BlueALSA PCM is SBC, 2 channels, 48000 Hz. speaker-test completed Front Left and Front Right through A2DP. Concurrent wlan0 ping was 15/15 with 0 percent packet loss. HCI recorded zero errors.
Decision: pending. Confirm actual audible playback with the user, test sustained audio under WLAN traffic, complete three physical cold boots, and regress AIC8800 Miracast before promoting #237 to a baseline.

## 2026-09-16 / pending regression / post-reboot BlueALSA audio
Hypothesis: the #237 bounded FIFO layout can re-establish real RTL8723BU A2DP after a fresh board boot while wlan0 is active.
Files: no source change. Temporary board runtime only: /tmp/btrtl.ko, /tmp/btbcm.ko, /tmp/btintel.ko, /tmp/btusb.ko, and /tmp/bluealsa/runtime.
Commit: 83db67f06d13930e1e1dc7d3bcd72013ba3db644 remains the active source record.
Build: no build.
Deploy and rollback: temporary modules and BlueALSA runtime transferred by SCP; existing /etc/dbus-1/system.d/bluealsa.conf policy and persisted Wt-070 pairing were reused. ALSA config and temporary BlueALSA plugin were restored after the audio test.
Tests: wlan0 associated; Wt-070 discovered at RSSI -58, connected, paired, trusted, and services-resolved. BlueALSA returned an SBC 48000 Hz stereo PCM. speaker-test completed Front Left and Front Right. HCI ACL counters reached 36 RX and 36 TX with zero errors.
Decision: pending. This is a fresh boot-session regression, but physical power removal was not independently observed. Keep it separate from the required three physical cold-boot acceptance cycles and still run AIC8800/Miracast regression.

## 2026-09-16 / pass / Cedar hard decode with Wt-070 audio
Hypothesis: Cedar hard video decode can send its audio stream through the active BlueALSA default ALSA device without disrupting WLAN or HCI.
Files: no source change. Runtime-only wrapper configured /etc/asound.conf and /usr/lib/alsa-lib/libasound_module_pcm_bluealsa.so for the player lifetime; video file /root/roms/video/bad.mp4.
Commit: a9417b39e2eb4a5bf0c4a9d6857260ebf3879c4d is the prior worktree-log record.
Build: no build.
Deploy and rollback: Cedar wrapper restores the prior ALSA config and removes the temporary plugin when it exits. BlueALSA remains under /tmp for this boot session.
Tests: /root/cedar_drm_player hard-decoded bad.mp4 and reported playback started, H.264 plugin registration, AAC audio codec 44100 Hz two channels, and audio output default. HCI ACL counters increased to RX 41 and TX 272 with zero errors. User confirmed audible audio from Wt-070.
Decision: pass for this boot session. Physical cold-boot count and AIC/Miracast regression remain pending before baseline promotion.

## 2026-09-17 / planned / AIC8800 nonshared FIFO RX priority
Hypothesis: the current EP2/EP3 512-byte FIFO_RXTX configuration permits an IN QH and OUT QH to overwrite each other on the same shared endpoint during AIC 5 GHz P2P traffic. Use only independent directions within the 2 KiB SRAM: EP1 TX512/RX512, EP2 RX512, EP3 RX256, including EP0 total 1856 bytes. Route AIC (a69c) bulk IN to idle EP1 RX; generic RTL Wi-Fi bulk IN continues to use EP2 RX; bulk OUT uses MUSB's same-direction mux on EP1.
Files: drivers/usb/musb/sunxi.c; drivers/usb/musb/musb_host.c.
Commit: pending.
Build: exact documented top-level zImage command with LOCALVERSION=; current output files are stale from rejected four-direction FIFO candidate and will be regenerated by dependency-aware make.
Deploy and rollback: none while board offline. Preserve #237 board zImage and sunxi.ko before any later deployment.
Tests: board offline; no runtime test.
Decision: pending.

## 2026-09-18 / planned / TK032F8004 panel graph enable
Hypothesis: the #237 DTS leaves the DPI panel and its LG4573A GPIO initializer
disabled while routing TCON0 to SII9022, so the built-in sun4i DRM driver has
no enabled panel connector to bind. Select the external TK032F8004 panel.
Files: arch/arm/boot/dts/suniv-f1c100s-licheepi-nano.dts.
Build: documented DTS-only target with the documented Linaro toolchain.
Deploy and rollback: backup the board DTB before any replacement; restoring it
returns the SII9022 display route. No zImage or module change is planned.
Tests: DTB syntax/decompile, cold boot, tk032 init probe, DRM card0/fb0, then
panel illumination. USB, wlan0, and AIC remain protected regressions.
Decision: pending.


2026-09-18 planned restore parent USB FIFO SII9022 DTS
Hypothesis: restore the parent USB FIFO SII9022 display graph only.
Files: arch/arm/boot/dts/suniv-f1c100s-licheepi-nano.dts
Protected: zImage MUSB USB RTL AIC rootfs unchanged.
Rollback: f09b225 and board DTB backup before deployment.
Commit: 98a2533f1e71aa6512fd191c6e5d9a342bdd6590
Build: DTS_only_Linaro_LOCALVERSION_empty
Artifact:
Test: board cold boot with restored parent USB FIFO SII DTB md5 dc40f1b4c19ccaa8710ef49204889f86
Protected: USB hub RTL8723BU and AIC cold PID 8d80 remained present.
Artifact_md5_dc40f1b4c19ccaa8710ef49204889f86
Result_display_failed_fb0_missing_card0_missing
Runtime_i2c_0_0039_sii902x_bind_ENODEV_after_tk_module_unload
Decision_pending_no_more_display_or_AIC_changes_in_this_test
Cold_boot_recheck_5_7_1_usb_hub_RTL8723BU_AIC_8d80_passed
Display_recheck_fb0_missing_and_tk_module_autoloaded
Planned_restore_TK032_panel_graph_from_f09b225
Scope_DTS_only_no_SII_bind_no_AIC_no_zImage
Planned_TK032_panel_timing_binding_fix
Hypothesis_panel_dpi_requires_panel_timing_not_display_timings
Planned_move_TK032_timing_properties_to_panel_timing
Build_DTB_md5_81c73b94251e2e48e8ba1f65c082dbb5
Cold_boot_panel_simple_bound_sun4i_drm_card0_fb0_present
Protected_USB_hub_RTL8723BU_AIC_8d80_present
Decision_panel_kernel_nodes_pass_visual_panel_confirmation_pending

## 2026-09-18 / runtime failure / AIC P2P GO radio activation
Hypothesis: the confirmed TK032 LCD/DRM baseline can progress through the existing matched AIC8800 Miracast path before Cedar starts.
Files: no kernel source, DTB, module, rootfs, or launch script changed. Runtime commands only.
Build: none.
Deploy and rollback: none. The active display DTB remained MD5 81c73b94251e2e48e8ba1f65c082dbb5. Old Cedar FIFO supervisor, player wrapper, and Cedar player PIDs were stopped before test; no persistent board file was replaced.
Tests: the matching AIC set passed exactly a69c:8d80 -> a69c:8d83 -> wlan1. Bare wpa24_aic_wfd_supplicant on wlan1 stayed live; its control socket answered and WFD subelements 0, 1, and 6 were accepted. No Cedar, DHCP, RTSP sink, or FIFO player was active.
Failure: p2p_group_add freq=5805 reproducibly reset the entire board before DHCP, capture, or display could start. After boot, AIC was again cold PID 8d80. The new boot had no retained OOM or panic trace.
Decision: do not use the old all-in-one Miracast wrappers and do not attribute this to panel/Cedar memory. The active blocker is the 5 GHz P2P radio-enable reset; next work must isolate AIC power/firmware behavior at GO creation with serial observation before attempting phone or player stages.
Failure-update: LOWMEM_TUNE=0, STOP_DBUS=0, DROP_PAGE_CACHE=0, CAPTURE_MODE=null prevents the reset, but GO still fails before DHCP/capture/player: DIRECT-XV appears, then WPA reports Failed to set beacon parameters, Could not connect to kernel driver, and wlan1 disappears. Active blocker is AIC AP/GO beacon configuration, not panel/Cedar.\n

## 2026-09-20 / planned / Miracast 接入 Cedar 90 度旋转 FIFO 播放
Hypothesis: 旋转 Cedar 播放器作为唯一解码与 DRM 显示消费者，并在启动前关闭非必要蓝牙、gmenu、D-Bus 和页缓存，可在当前 54 MiB RAM / 24 MiB CMA 平台上为实时 Miracast 留出连续内存。
Files: scripts/runtime/manual_miracast_chain_20260920.sh, WORKTREE_CHANGELOG.md, 变更记录.md
Commit: pending
Build: shell syntax check only; no kernel build or module change.
Deploy and rollback: deploy only the tracked protocol script into its isolated candidate directory. Rollback uses the existing stop command and leaves /lib/modules, kernel, AIC modules, DTS, and rootfs unchanged.
Tests: stop local test player; verify pre-cast MemAvailable/CmaFree; start GO and FIFO player; require player log rotation=1 output=480x800, phone association, DHCP, RTSP, then test display.
Decision: pending

Result update: On a subsequent physical cold boot with 18.8 MiB free CMA, loading the same source-built loader/fdrv registered wlan1, but the scripted WPA/GO launch reset or disconnected the AIC before GO completion: a later SSH observation found the board newly booted at #241 with a69c:8d80, wlan0 only, no AIC modules, and no retained panic/OOM trace. This happened before phone association, DHCP, RTSP, FIFO, or Cedar decoding. Do not repeat this startup sequence blindly; isolate the runtime trigger before another end-to-end test. Decision: rejected for this attempt; board restored to cold baseline.

## 2026-09-21 / measured / AIC wlan1 and minimum GO staged reproduction
Method: Two physical cold boot trials, both with #241, source-built UGREEN loader 5186d0db18ac146d2b940a54d11f226e, fdrv e68b49a3f30d8c061334987f1a237720, firmware 01acfbebdfb15755e3fe853e7bc95c7d. Stages were loader -> 8d83 -> fdrv -> wlan1 -> WPA/WFD only -> p2p_flush -> p2p_stop_find -> p2p_group_add freq=5805. No DHCP, RTSP sink, Cedar player, FIFO, PBC, or phone connection was started.
Trial 1: loader/fdrv and WPA/WFD passed; p2p_group_add returned OK then board rebooted to #241 / 8d80 / wlan0 before status read.
Trial 2: same staged command path passed; WPA logged P2P-GROUP-STARTED GO ssid=DIRECT-rX freq=5805, kernel logged AP started channel=5805, and the group stayed present throughout a 30-second idle observation with SSH alive.
Conclusion: wlan1 registration and WPA configuration are not the deterministic fault. The AP/GO launch has an intermittent AIC/AP runtime failure before higher layers. Keep this successful group for the next staged PBC/DHCP test; do not add Cedar until association and DHCP are confirmed.
Decision: pending; no kernel or driver source change.

Result update: commit d5145b4. Cold boot #241 with source-built UGREEN dual-OUT modules reached wlan1, 5805 MHz GO, PBC, AP-STA-CONNECTED, DHCP 192.168.49.52, and complete RTSP SETUP/PLAY. Sink received 44 RTP packets / 15448 H.264 bytes. Cedar exited because its raw input was configured as 800x480 while the negotiated stream was 640x480; no OOM, CMA allocation failure, kernel panic, USB reset, or AIC disconnect occurred. A subsequent 640x480@60 GO startup lost SSH/serial control before phone connection and was recovered by COM5 cold boot. This is pending independent local 640x480 rotation validation; not accepted.

## 2026-09-21 / planned / PBC 超时自动重新授权
Hypothesis: 手机的 P2P PBC 请求在固定 120 秒授权窗结束后到达；watcher 立即检测 WPS-TIMEOUT 或 P2P-PROV-DISC-PBC-REQ 并重新执行 wps_pbc any，可避免人工时序造成的关联失败。
Files: scripts/runtime/manual_miracast_chain_20260920.sh, WORKTREE_CHANGELOG.md
Commit: pending
Build: shell syntax check only.
Deploy and rollback: redeploy only the isolated runtime candidate script; stop removes WPA, DHCP, watcher, FIFO supervisor and rotated player.
Tests: require PBC rearm log, then AP-STA-CONNECTED, DHCP REQUEST, RTSP and player FIFO data.
Decision: pending

## 2026-09-21 / planned / 旋转播放器单实例清理
Hypothesis: FIFO supervisor stop path does not match the rotation binary's absolute path, leaving a Cedar DRM plane owner. Explicitly stopping h264_fifo_player.pid before its supervisor prevents drmModeSetPlane err -13 on the next cast.
Files: scripts/runtime/manual_miracast_chain_20260920.sh, WORKTREE_CHANGELOG.md
Commit: pending
Build: shell syntax check only.
Deploy and rollback: isolated runtime script only; stop should leave no rotated Cedar player, supervisor, FIFO, WPA or DHCP process.
Tests: after stop, no cedar_drm_player_rotate_x0 process remains; next GO starts exactly one player with no DRM plane error.
Decision: pending

## 2026-09-21 / planned / 运行时 PID 强制清理
Hypothesis: FIFO-blocked Cedar and DHCP child processes may ignore the script's initial SIGTERM. pid_stop should check after one second and use SIGKILL only for the still-recorded PID, preventing residual CMA/DRM owners.
Files: scripts/runtime/manual_miracast_chain_20260920.sh, WORKTREE_CHANGELOG.md
Commit: pending
Build: shell syntax check only.
Deploy and rollback: isolated runtime script only. Stop must return the board to one wlan0 WPA process, no Miracast WPA/DHCP/Cedar/supervisor process, and released CMA.
Tests: stop after a started FIFO player; confirm no matching process and compare MemAvailable/CmaFree.
Decision: pending

## 2026-09-21 / planned / 对齐 Redmi 实际协商视频尺寸
Hypothesis: RTSP/RTP succeeded but the rotated raw-H.264 player was configured for the local 800x480 sample while Redmi negotiated 640x480. Set the live-player input to 640x480@60 so the Cedar decoder accepts the stream and rotates it to native 480x640 output.
Files: scripts/runtime/manual_miracast_chain_20260920.sh, WORKTREE_CHANGELOG.md
Commit: pending
Build: shell syntax check only.
Deploy and rollback: isolated runtime script only; stop old session before launch. No kernel, AIC driver, DTS, module or rootfs modification.
Tests: player log must no longer contain err size; require raw H.264 decode, rotation request=1, RTSP keepalive and stable phone association.
Decision: pending

## 2026-09-20 / planned / 回退已拒绝的 MUSB TX 轮转改动
Hypothesis: 当前源码中的 `ep->tx_reinit = 1` 来自已拒绝的 343d718，板端 #241 未使用该行；先将源码恢复到 #241 对应调度状态，避免后续实验叠加已证实导致 GO 后失联的改动。
Files: drivers/usb/musb/musb_host.c, WORKTREE_CHANGELOG.md
Commit: pending
Build: none; this is source-state correction only.
Deploy and rollback: no board deployment. Source rollback is the parent form from 343d718^.
Tests: inspect exact diff and confirm the board remains at #241 cold baseline without deployment.
Decision: pending
