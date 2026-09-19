#!/bin/sh
set -eu

PATH=/sbin:/bin:/usr/sbin:/usr/bin
BASE=/root/aic_miracast
PAIR=$BASE/candidates/aic_20260725_known_good
RUNTIME=/tmp/manual_miracast_20260920
IFACE=wlan1
GO_IP=192.168.49.1
CLIENT_IP=${CLIENT_IP:-192.168.49.52}
CLIENT_MAC=${CLIENT_MAC:-a6:00:33:22:26:9e}
CTRL=$RUNTIME/wpa_ctrl
CONF=$RUNTIME/wpa.conf
WPA=$BASE/candidates/miracast_stable_protocol_20260818/wpa24_aic_wfd_supplicant
CLI=$BASE/candidates/miracast_stable_protocol_20260818/wpa24_aic_wfd_cli
DHCP=$BASE/candidates/miracast_stable_protocol_20260818/tiny_dhcpd_49
SINK=$BASE/miracast_sink_dump.lowest
LOADER=$PAIR/aic_load_fw.ko
FDRV=$PAIR/aic8800_fdrv.ko
FW_DIR=$PAIR/firmware

log() {
    echo "[manual-miracast] $*"
}

pid_stop() {
    pid_file=$1
    if [ -f "$pid_file" ]; then
        pid=$(cat "$pid_file" 2>/dev/null || true)
        [ -n "$pid" ] && kill "$pid" 2>/dev/null || true
        rm -f "$pid_file"
    fi
}

has_iface() {
    [ -e "/sys/class/net/$IFACE" ]
}

wait_usb() {
    wanted=$1
    i=0
    while [ "$i" -lt 25 ]; do
        lsusb 2>/dev/null | grep -qi "$wanted" && return 0
        i=$((i + 1))
        sleep 1
    done
    return 1
}

wait_iface() {
    i=0
    while [ "$i" -lt 25 ]; do
        has_iface && return 0
        i=$((i + 1))
        sleep 1
    done
    return 1
}

require_wpa() {
    [ -x "$WPA" ] && [ -x "$CLI" ] && [ -x "$DHCP" ] && [ -x "$SINK" ]
}

register() {
    [ -f "$LOADER" ] && [ -f "$FDRV" ] && [ -f "$FW_DIR/fmacfw_8800d80_u02.bin" ]
    md5sum "$LOADER" "$FDRV" "$FW_DIR/fmacfw_8800d80_u02.bin"
    has_iface && { log "$IFACE already exists"; return 0; }
    lsusb 2>/dev/null | grep -qi 'a69c:8d80' || {
        log "expected a69c:8d80 before loader"
        return 1
    }
    /sbin/insmod "$LOADER" aic_fw_path="$FW_DIR" aicwf_dbg_level=0
    wait_usb 'a69c:8d83' || {
        log "loader did not reach a69c:8d83"
        return 1
    }
    /sbin/insmod "$FDRV" aicwf_dbg_level=0
    wait_iface || {
        log "$IFACE did not register"
        return 1
    }
    /sbin/ifconfig "$IFACE" up
    log "$IFACE registered"
}

wpa_start() {
    has_iface || { log "$IFACE missing; run register first"; return 1; }
    require_wpa || { log "missing protocol executable"; return 1; }
    pid_stop "$RUNTIME/wpa.pid"
    rm -rf "$CTRL"
    mkdir -p "$RUNTIME" "$CTRL"
    cat >"$CONF" <<EOF
ctrl_interface=DIR=$CTRL GROUP=netdev
update_config=1
device_name=F1C200S-AIC
device_type=8-0050F204-5
manufacturer=F1C200S
model_name=F1C200S-AIC-WFD
model_number=1
serial_number=1
config_methods=virtual_display virtual_push_button pbc
p2p_go_intent=15
p2p_oper_reg_class=124
p2p_oper_channel=161
p2p_listen_reg_class=81
p2p_listen_channel=6
p2p_no_group_iface=1
driver_param=use_p2p_group_interface=0
EOF
    nohup "$WPA" -Dnl80211 -i "$IFACE" -c "$CONF" -f "$RUNTIME/wpa.log" \
        </dev/null >"$RUNTIME/wpa.stdout" 2>&1 &
    echo $! >"$RUNTIME/wpa.pid"
    sleep 2
    "$CLI" -p "$CTRL" -i "$IFACE" status >/dev/null
    "$CLI" -p "$CTRL" -i "$IFACE" set wifi_display 1
    "$CLI" -p "$CTRL" -i "$IFACE" set device_name F1C200S-AIC
    "$CLI" -p "$CTRL" -i "$IFACE" set device_type 8-0050F204-5
    "$CLI" -p "$CTRL" -i "$IFACE" wfd_subelem_set 0 000600111c44012c
    "$CLI" -p "$CTRL" -i "$IFACE" wfd_subelem_set 1 0006000000000000
    "$CLI" -p "$CTRL" -i "$IFACE" wfd_subelem_set 6 000700000000000000
    "$CLI" -p "$CTRL" -i "$IFACE" wfd_subelem_set 11 00020001
    log "WPA and WFD ready"
}

go_start() {
    "$CLI" -p "$CTRL" -i "$IFACE" p2p_flush
    "$CLI" -p "$CTRL" -i "$IFACE" p2p_stop_find
    sleep 2
    "$CLI" -p "$CTRL" -i "$IFACE" p2p_group_add freq=5805
    sleep 4
    /sbin/ifconfig "$IFACE" "$GO_IP" netmask 255.255.255.0 up
    pid_stop "$RUNTIME/dhcp.pid"
    nohup "$DHCP" "$IFACE" </dev/null >"$RUNTIME/dhcp.log" 2>&1 &
    echo $! >"$RUNTIME/dhcp.pid"
    "$CLI" -p "$CTRL" -i "$IFACE" wps_pbc any
    "$CLI" -p "$CTRL" -i "$IFACE" status
}

watch_loop() {
    seen=$(grep -c "REQUEST .*ip=$CLIENT_IP" "$RUNTIME/dhcp.log" 2>/dev/null || true)
    while true; do
        current=$(grep -c "REQUEST .*ip=$CLIENT_IP" "$RUNTIME/dhcp.log" 2>/dev/null || true)
        if [ "$current" -gt "$seen" ]; then
            /sbin/arp -d "$CLIENT_IP" 2>/dev/null || true
            /sbin/arp -s "$CLIENT_IP" "$CLIENT_MAC" -i "$IFACE"
            exec "$SINK" "$CLIENT_IP" "$RUNTIME/stream.h264"
        fi
        sleep 1
    done
}

watch_start() {
    [ -f "$RUNTIME/dhcp.pid" ] || { log "run go first"; return 1; }
    pid_stop "$RUNTIME/watch.pid"
    rm -f "$RUNTIME/sink.log" "$RUNTIME/stream.h264"
    nohup "$0" _watch_loop </dev/null >"$RUNTIME/sink.log" 2>&1 &
    echo $! >"$RUNTIME/watch.pid"
    log "watching DHCP REQUEST for $CLIENT_IP"
}

stop() {
    "$CLI" -p "$CTRL" -i "$IFACE" p2p_group_remove "$IFACE" 2>/dev/null || true
    pid_stop "$RUNTIME/watch.pid"
    pid_stop "$RUNTIME/dhcp.pid"
    pid_stop "$RUNTIME/wpa.pid"
    /sbin/ifconfig "$IFACE" down 2>/dev/null || true
    rm -rf "$RUNTIME"
}

status() {
    echo '--- usb ---'
    lsusb
    echo '--- interfaces ---'
    ls /sys/class/net
    echo '--- modules ---'
    cat /proc/modules | grep '^aic' || true
    echo '--- wpa ---'
    "$CLI" -p "$CTRL" -i "$IFACE" status 2>&1 || true
    echo '--- dhcp ---'
    tail -60 "$RUNTIME/dhcp.log" 2>/dev/null || true
    echo '--- sink ---'
    tail -80 "$RUNTIME/sink.log" 2>/dev/null || true
}

case "${1:-}" in
    register) register ;;
    wpa) wpa_start ;;
    go) go_start ;;
    watch) watch_start ;;
    stop) stop ;;
    status) status ;;
    _watch_loop) watch_loop ;;
    *)
        echo "Usage: $0 {register|wpa|go|watch|stop|status}"
        exit 2
        ;;
esac
