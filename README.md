# F1C200S 480x800 Display Candidate

Purpose: replace historical 360x640 SDL and 384x640 Cedar view defaults with
the native 480x800 framebuffer geometry reported by `/sys/class/graphics/fb0`.

Baseline board files:

- `/root/sdl_landscape_env.sh`: `5d18f47bc34bcd22fc6a9a52fa6894e3`
- `/root/run_gmenu2x.sh`: `4ff4c8d21730b0774f3b94dc95739664`
- `/root/aic_miracast/start_miracast_live_display_yuvcrop.sh`:
  `c55cddc6fb115f8182434225372962c2`
- `/root/cedar_drm_player`: `bbb7161eb694bd6f3898b86c7c3bf270` (binary-only;
  intentionally not modified by this candidate)

This candidate only changes visible/display-layer geometry. It does not change
the panel DTB, kernel, Cedar decoder, or source crop behavior.
