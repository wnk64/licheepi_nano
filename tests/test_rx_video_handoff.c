#define wfd_video_push capture_push
#define main sink_program_main
#include "miracast_sink_dump.c"
#undef main
#undef wfd_video_push
#include <assert.h>

static uint8_t captured[65536];
static size_t captured_bytes, calls;
static int fail_output;

int capture_push(const void *data, size_t size)
{
    if (fail_output) { errno = ENOBUFS; return -1; }
    assert(size <= sizeof(captured) - captured_bytes);
    memcpy(captured + captured_bytes, data, size);
    captured_bytes += size;
    calls++;
    return 0;
}

static void reset(void)
{
    captured_bytes = calls = 0;
    fail_output = g_stop = g_video_output_failed = 0;
    g_rx_video_bytes = g_rx_video_peak = g_rx_video_discarded = 0;
    g_video_writes = g_h264_bytes = 0;
    g_async_video = g_rx_video_enabled = 1;
}

static void packet(uint8_t *pkt, uint8_t value)
{
    memset(pkt, value, 200);
    memset(pkt, 0, 12);
    pkt[0] = 0x80; pkt[1] = 33;
    pkt[12] = 0x47; pkt[13] = 0x10; pkt[14] = 0x11; pkt[15] = 0x10;
}

int main(void)
{
    uint8_t pkt[200];
    reset();
    for (int i = 0; i < 16; i++) { packet(pkt, i); process_rtp_packet(pkt, sizeof(pkt)); }
    assert(calls == 0 && g_rx_video_bytes == 16 * 184);
    flush_rx_video();
    assert(calls == 1 && captured_bytes == 16 * 184 && g_rx_video_bytes == 0);
    for (size_t i = 0; i < captured_bytes; i++) assert(captured[i] == i / 184);
    assert(g_h264_bytes == captured_bytes && !g_stop);

    reset();
    for (int i = 0; i < 200; i++) { packet(pkt, i); process_rtp_packet(pkt, sizeof(pkt)); }
    assert(g_rx_video_peak <= RX_VIDEO_BYTES && calls == 1);
    flush_rx_video();
    assert(calls == 2 && captured_bytes == 200 * 184);
    for (size_t i = 0; i < captured_bytes; i++) assert(captured[i] == i / 184);

    reset();
    packet(pkt, 7); process_rtp_packet(pkt, sizeof(pkt));
    process_rtp_packet(pkt, 1);
    process_rtp_packet(pkt, RTP_MAX + 1);
    pkt[13] = 0x11; pkt[14] = 0; /* Audio PID; no audio instance in this test. */
    process_rtp_packet(pkt, sizeof(pkt));
    flush_rx_video();
    assert(calls == 1 && captured_bytes == 184 && captured[0] == 7);

    reset();
    packet(pkt, 2); process_rtp_packet(pkt, sizeof(pkt));
    fail_output = 1;
    flush_rx_video();
    assert(g_stop && g_video_output_failed && g_rx_video_bytes == 0);
    assert(g_rx_video_discarded == 184 && captured_bytes == 0);

    reset();
    packet(pkt, 3); process_rtp_packet(pkt, sizeof(pkt));
    g_stop = 1;
    flush_rx_video();
    assert(calls == 0 && g_rx_video_discarded == 184 && g_rx_video_bytes == 0);

    reset();
    g_rx_video_enabled = 0;
    packet(pkt, 4); process_rtp_packet(pkt, sizeof(pkt));
    assert(calls == 1 && captured_bytes == 184 && !g_rx_video_bytes);
    puts("RX handoff exact bytes/order/capacity/tail/invalid/mixedPID/failure/stop/default passed");
    return 0;
}
