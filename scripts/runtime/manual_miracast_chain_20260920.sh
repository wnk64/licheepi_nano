#!/bin/sh
set -eu

PATH=/sbin:/bin:/usr/sbin:/usr/bin
BASE=/root/aic_miracast
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
FIFO=$RUNTIME/live.h264.fifo
PLAYER_SUPERVISOR=$BASE/candidates/miracast_stable_protocol_20260818/supervise_h264_fifo_player.sh
ROTATE_PLAYER=/root/display_480x800_candidate_20260914/rotation/cedar_drm_player_rotate_x0
PLAYER_WIDTH=${PLAYER_WIDTH:-800}
PLAYER_HEIGHT=${PLAYER_HEIGHT:-480}
PLAYER_FPS=${PLAYER_FPS:-30}

log() {
    echo "[manual-miracast] $*"
}

pid_stop() {
    pid_file=$1
    if [ -f "$pid_file" ]; then
        pid=$(cat "$pid_file" 2>/dev/null || true)
        if [ -n "$pid" ]; then
            kill "$pid" 2>/dev/null || true
            sleep 1
            kill -0 "$pid" 2>/dev/null && kill -9 "$pid" 2>/dev/null || true
        fi
        rm -f "$pid_file"
    fi
}

has_iface() {
    [ -e "/sys/class/net/$IFACE" ]
}

require_wpa() {
    [ -x "$WPA" ] && [ -x "$CLI" ] && [ -x "$DHCP" ] && [ -x "$SINK" ] &&
        [ -x "$PLAYER_SUPERVISOR" ] && [ -x "$ROTATE_PLAYER" ]
}

player_stop() {
    pid_stop "$RUNTIME/h264_fifo_player.pid"
    pid_stop "$RUNTIME/player_supervisor.pid"
    "$PLAYER_SUPERVISOR" stop 2>/dev/null || true
    rm -f "$FIFO"
}

player_start() {
    player_stop
    rm -f "$RUNTIME/player.log" "$RUNTIME/player.stdout"
    CEDAR_ROTATE=90 CEDAR_NO_PACE=0 CEDAR_VIEW_X=0 CEDAR_VIEW_Y=0 \
        CEDAR_VIEW_W=480 CEDAR_VIEW_H=800 LOWMEM=1 STOP_GMENU=1 \
        PLAYER="$ROTATE_PLAYER" WIDTH="$PLAYER_WIDTH" HEIGHT="$PLAYER_HEIGHT" \
        FPS="$PLAYER_FPS" FIFO="$FIFO" LOG="$RUNTIME/player.log" \
        RUN_DIR="$RUNTIME" nohup "$PLAYER_SUPERVISOR" start \
        </dev/null >"$RUNTIME/player.stdout" 2>&1 &
    echo $! >"$RUNTIME/player_supervisor.pid"
    i=0
    while [ "$i" -lt 10 ]; do
        [ -p "$FIFO" ] && break
        i=$((i + 1))
        sleep 1
    done
    [ -p "$FIFO" ] || { log "rotation player FIFO not ready"; return 1; }
    log "rotation player ready: ${PLAYER_WIDTH}x${PLAYER_HEIGHT}@${PLAYER_FPS}, 90 degrees"
}

wpa_start() {
    has_iface || { log "$IFACE missing; register it with a source-backed driver first"; return 1; }
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
    player_start
    "$CLI" -p "$CTRL" -i "$IFACE" wps_pbc any
    "$CLI" -p "$CTRL" -i "$IFACE" status
}

watch_loop() {
    seen=$(grep -c "REQUEST .*ip=$CLIENT_IP" "$RUNTIME/dhcp.log" 2>/dev/null || true)
    pbc_timeout_seen=$(grep -c "WPS-TIMEOUT" "$RUNTIME/wpa.log" 2>/dev/null || true)
    pbc_request_seen=$(grep -c "P2P-PROV-DISC-PBC-REQ" "$RUNTIME/wpa.log" 2>/dev/null || true)
    while true; do
        pbc_timeout_current=$(grep -c "WPS-TIMEOUT" "$RUNTIME/wpa.log" 2>/dev/null || true)
        pbc_request_current=$(grep -c "P2P-PROV-DISC-PBC-REQ" "$RUNTIME/wpa.log" 2>/dev/null || true)
        if [ "$pbc_timeout_current" -gt "$pbc_timeout_seen" ] || \
           [ "$pbc_request_current" -gt "$pbc_request_seen" ]; then
            "$CLI" -p "$CTRL" -i "$IFACE" wps_pbc any >>"$RUNTIME/wpa_rearm.log" 2>&1 || true
            pbc_timeout_seen=$pbc_timeout_current
            pbc_request_seen=$pbc_request_current
            log "PBC rearmed after timeout or peer request"
        fi
        current=$(grep -c "REQUEST .*ip=$CLIENT_IP" "$RUNTIME/dhcp.log" 2>/dev/null || true)
        if [ "$current" -gt "$seen" ]; then
            /sbin/arp -d "$CLIENT_IP" 2>/dev/null || true
            /sbin/arp -s "$CLIENT_IP" "$CLIENT_MAC" -i "$IFACE"
            exec "$SINK" "$CLIENT_IP" "$FIFO"
        fi
        sleep 1
    done
}

watch_start() {
    [ -f "$RUNTIME/dhcp.pid" ] || { log "run go first"; return 1; }
    pid_stop "$RUNTIME/watch.pid"
    rm -f "$RUNTIME/sink.log"
    nohup "$0" _watch_loop </dev/null >"$RUNTIME/sink.log" 2>&1 &
    echo $! >"$RUNTIME/watch.pid"
    log "watching DHCP REQUEST for $CLIENT_IP"
}

stop() {
    "$CLI" -p "$CTRL" -i "$IFACE" p2p_group_remove "$IFACE" 2>/dev/null || true
    pid_stop "$RUNTIME/watch.pid"
    pid_stop "$RUNTIME/dhcp.pid"
    pid_stop "$RUNTIME/wpa.pid"
    player_stop
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
    echo '--- player ---'
    tail -80 "$RUNTIME/player.log" 2>/dev/null || true
}

case "${1:-}" in
    wpa) wpa_start ;;
    go) go_start ;;
    watch) watch_start ;;
    stop) stop ;;
    status) status ;;
    _watch_loop) watch_loop ;;
    *)
        echo "Usage: $0 {wpa|go|watch|stop|status}"
        exit 2
        ;;
esac
