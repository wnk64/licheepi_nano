#define main sink_program_main
#include "miracast_sink_dump.c"
#undef main
#include <assert.h>

struct output { unsigned char bytes[4096]; size_t length, writes; };
static ssize_t output_write(void *opaque, const char *data, size_t size)
{
    struct output *o = opaque;
    assert(size <= sizeof(o->bytes) - o->length);
    memcpy(o->bytes + o->length, data, size);
    o->length += size;
    o->writes++;
    return (ssize_t)size;
}

int main(void)
{
    FILE *original = stdout;
    struct output out = {0};
    cookie_io_functions_t io = { .write = output_write };
    stdout = fopencookie(&out, "w", io);
    assert(stdout);
    setvbuf(stdout, NULL, _IONBF, 0);
    uint8_t pkt[RTP_MAX] = {0x80, 33};
    uint8_t expected[7 * 184];
    size_t expected_bytes = 0;
    for (int i = 0; i < 7; i++) {
        uint8_t *ts = pkt + 12 + i * TS_SIZE;
        memset(ts, i + 1, TS_SIZE);
        ts[0] = 0x47;
        ts[1] = i == 0 ? 0x50 : 0x10;
        ts[2] = i == 3 ? 0x00 : 0x11;
        ts[3] = 0x10;
        size_t off = 4;
        if (i == 0) {
            memset(ts + off, 0, 9);
            ts[off + 2] = 1;
            ts[off + 3] = 0xe0;
            off += 9;
        }
        if (i != 3) {
            memcpy(expected + expected_bytes, ts + off, TS_SIZE - off);
            expected_bytes += TS_SIZE - off;
        }
    }
    process_rtp_packet(pkt, 12 + 7 * TS_SIZE);
    assert(!g_stop && out.writes == 1 && g_video_writes == 1);
    assert(out.length == expected_bytes && g_h264_bytes == expected_bytes);
    assert(!memcmp(out.bytes, expected, expected_bytes));
    process_rtp_packet(pkt + 12, 2); /* Truncated RTP must not emit. */
    assert(out.writes == 1);
    process_rtp_packet(pkt, 12 + TS_SIZE);
    assert(out.writes == 2 && out.length == expected_bytes + 175);
    assert(!memcmp(out.bytes + expected_bytes, expected, 175));
    process_rtp_packet(pkt, RTP_MAX + 1);
    assert(out.writes == 2);
    fclose(stdout);
    int pipefd[2];
    assert(!pipe(pipefd));
    close(pipefd[0]);
    signal(SIGPIPE, SIG_IGN);
    stdout = fdopen(pipefd[1], "w");
    assert(stdout);
    setvbuf(stdout, NULL, _IONBF, 0);
    process_rtp_packet(pkt, 12 + TS_SIZE);
    assert(g_stop && g_video_batch_bytes == 0);
    fclose(stdout);
    stdout = original;
    puts("RTP batch bytes/mixed PID/PES/tail/invalid/EPIPE passed; 6 writes -> 1");
    return 0;
}
