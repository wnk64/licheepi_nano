#include "wfd_lpcm.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static unsigned calls;
static size_t emitted;
static void collect(void *opaque, const uint8_t *pcm, size_t bytes, int64_t pts)
{
    (void)opaque;
    assert(bytes == 1920 && pts == 90000);
    for (size_t i = 0; i < bytes; ++i)
        assert(pcm[i] == (uint8_t)i);
    calls++;
    emitted += bytes;
}

static size_t make_pes(uint8_t *pes)
{
    const size_t length = 14 + 4 + 1920;
    memset(pes, 0, length);
    pes[2] = 1; pes[3] = 0xbd;
    pes[4] = (length - 6) >> 8; pes[5] = (length - 6) & 255;
    pes[6] = 0x80; pes[7] = 0x80; pes[8] = 5;
    uint64_t pts = 90000;
    pes[9] = 0x21 | ((pts >> 29) & 14);
    pes[10] = pts >> 22;
    pes[11] = 1 | ((pts >> 14) & 254);
    pes[12] = pts >> 7;
    pes[13] = 1 | ((pts << 1) & 254);
    pes[14] = 0xa0; pes[15] = 6; pes[17] = 0x11;
    for (size_t i = 0; i < 1920; i++)
        pes[18 + i] = (uint8_t)i;
    return length;
}

static void packet(uint8_t *ts, const uint8_t *data, size_t bytes, int start, unsigned cc)
{
    assert(bytes >= 1 && bytes <= 184);
    memset(ts, 0xff, 188);
    ts[0] = 0x47; ts[1] = 0x11 | (start ? 0x40 : 0); ts[2] = 0;
    ts[3] = 0x10 | (cc & 15);
    size_t offset = 4;
    if (bytes < 184) {
        ts[3] |= 0x20;
        ts[4] = 183 - bytes;
        if (ts[4])
            ts[5] = 0;
        offset = 188 - bytes;
    }
    memcpy(ts + offset, data, bytes);
}

static void feed(struct wfd_lpcm *d, const uint8_t *pes, size_t length,
                 size_t first, int lost, int duplicates)
{
    size_t offset = 0;
    unsigned cc = 0;
    uint8_t ts[188];
    while (offset < length) {
        size_t bytes = offset ? 184 : first;
        if (bytes > length - offset)
            bytes = length - offset;
        packet(ts, pes + offset, bytes, !offset, cc);
        if (!lost || cc != 1) {
            wfd_lpcm_ts(d, ts);
            if (duplicates)
                wfd_lpcm_ts(d, ts);
        }
        offset += bytes;
        cc++;
    }
}

int main(int argc, char **argv)
{
    uint8_t pes[2000], ts[188];
    size_t length = make_pes(pes);
    struct wfd_lpcm *d = malloc(sizeof(*d));
    assert(d);
    for (size_t split = 1; split <= 184; split++) {
        wfd_lpcm_init(d, collect, NULL);
        unsigned old = calls;
        feed(d, pes, length, split, 0, 1);
        assert(calls == old + 1 && d->completed == 1 && !d->malformed);
    }
    wfd_lpcm_init(d, collect, NULL);
    unsigned old = calls;
    feed(d, pes, length, 184, 1, 0);
    assert(calls == old && d->discontinuities);
    feed(d, pes, length, 3, 0, 0);
    assert(calls == old + 1);
    pes[17] = 0x10;
    feed(d, pes, length, 184, 0, 0);
    assert(d->malformed && calls == old + 1);
    length = make_pes(pes);
    packet(ts, pes, 184, 1, 0);
    ts[1] |= 0x80;
    wfd_lpcm_ts(d, ts);
    assert(!d->active);
    for (unsigned i = 0; i < 20000; i++) {
        for (unsigned j = 0; j < 188; j++)
            ts[j] = rand();
        ts[0] = 0x47; ts[1] = 0x51; ts[2] = 0;
        wfd_lpcm_ts(d, ts);
    }
    free(d);
    if (argc > 1 && strcmp(argv[1], "--alsa-null") == 0) {
        for (unsigned repeat = 0; repeat < 20; repeat++) {
            assert(wfd_audio_start("null") == 0);
            for (unsigned cc = 0; cc < 20; cc++) {
                size_t offset = 0;
                unsigned counter = cc * 11;
                while (offset < length) {
                    size_t bytes = length - offset;
                    if (bytes > 184)
                        bytes = 184;
                    packet(ts, pes + offset, bytes, !offset, counter++);
                    wfd_audio_ts(ts);
                    offset += bytes;
                }
            }
            usleep(30000);
            wfd_audio_stop();
            wfd_audio_stop();
            assert(wfd_audio_start("null") == 0);
            wfd_audio_stop();
        }
    }
    printf("LPCM: 184 split points, duplicates/loss/resync/format/TEI and fuzz passed; PCM=%zu\n", emitted);
    return 0;
}
