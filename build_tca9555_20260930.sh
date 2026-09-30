#!/bin/sh
set -eu
cd /home/wnk/LicheePi_Nano/linux_musb_clean_ep1_20260811
test -f .config && test -f vmlinux && test -f System.map && test -f arch/arm/boot/zImage
PREFIX=/opt/gcc-linaro-7.2.1-2017.11-x86_64_arm-linux-gnueabi/bin/arm-linux-gnueabi-
release=$(make ARCH=arm CROSS_COMPILE="$PREFIX" LOCALVERSION= kernelrelease)
test "$release" = 5.7.1
set +e
make ARCH=arm CROSS_COMPILE="$PREFIX" LOCALVERSION= -j8 zImage modules suniv-f1c100s-licheepi-nano-tca9555.dtb >/tmp/f1-tca9555-build-0930.log 2>&1
rc=$?
printf '%s\n' "$rc" >/tmp/f1-tca9555-build-0930.rc
exec bash
