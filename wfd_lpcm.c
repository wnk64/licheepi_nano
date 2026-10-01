#include "wfd_lpcm.h"
#include <string.h>

static void finish_pes(struct wfd_lpcm *d)
{
    const uint8_t *p = d->pes;
    size_t offset;
    int64_t pts = -1;
    if (d->target < 13 || p[3] != 0xbd || (p[6] & 0xc0) != 0x80)
        goto malformed;
    offset = 9u + p[8];
    if (offset > d->target || d->target - offset < 4)
        goto malformed;
    if ((p[7] & 0x80) && p[8] >= 5 && (p[9] & 1) && (p[11] & 1) && (p[13] & 1))
        pts = ((int64_t)((p[9] >> 1) & 7) << 30) | ((int64_t)p[10] << 22) |
              ((int64_t)(p[11] >> 1) << 15) | ((int64_t)p[12] << 7) | (p[13] >> 1);
    size_t first = offset;
    /* WFD PCM framing follows AOSP ESQueue::dequeueAccessUnitPCMAudio:
       a0 / number of 80-sample AUs / reserved / 16-bit,48kHz,stereo (11). */
    while (offset < d->target) {
        if (d->target - offset < 4 || p[offset] != 0xa0 || !p[offset + 1] ||
            p[offset + 3] != 0x11)
            goto malformed;
        size_t bytes = (size_t)p[offset + 1] * 80u * 4u;
        if (bytes > d->target - offset - 4)
            goto malformed;
        offset += 4 + bytes;
    }
    for (offset = first; offset < d->target;) {
        size_t bytes = (size_t)p[offset + 1] * 80u * 4u;
        if (d->emit)
            d->emit(d->opaque, p + offset + 4, bytes, pts);
        if (pts >= 0)
            pts += (int64_t)bytes * 90000 / (48000 * 4);
        offset += 4 + bytes;
    }
    d->completed++;
    return;
malformed:
    d->malformed++;
}

void wfd_lpcm_init(struct wfd_lpcm *d, wfd_pcm_emit emit, void *opaque)
{
    memset(d, 0, sizeof(*d));
    d->emit = emit;
    d->opaque = opaque;
}

void wfd_lpcm_ts(struct wfd_lpcm *d, const uint8_t *ts)
{
    if (ts[0] != 0x47 || (((ts[1] & 0x1f) << 8) | ts[2]) != WFD_LPCM_PID)
        return;
    d->packets++;
    unsigned afc = (ts[3] >> 4) & 3;
    size_t offset = 4;
    if ((ts[1] & 0x80) || !afc) {
        d->active = d->have_cc = 0;
        d->discontinuities++;
        return;
    }
    if (afc & 2) {
        offset += 1u + ts[4];
        if (offset > 188) {
            d->active = d->have_cc = 0;
            d->malformed++;
            return;
        }
        if (ts[4] && (ts[5] & 0x80)) {
            d->active = d->have_cc = 0;
            d->discontinuities++;
        }
    }
    if (!(afc & 1) || offset >= 188)
        return;
    unsigned cc = ts[3] & 15;
    if (d->have_cc && cc == d->cc)
        return;
    if (d->have_cc && cc != ((d->cc + 1) & 15)) {
        d->active = 0;
        d->discontinuities++;
    }
    d->cc = cc;
    d->have_cc = 1;
    if (ts[1] & 0x40) {
        if (d->active)
            d->discontinuities++;
        d->active = 1;
        d->length = d->target = 0;
    }
    if (!d->active)
        return;
    while (offset < 188 && d->active) {
        size_t need = (d->target ? d->target : 6) - d->length;
        size_t bytes = 188 - offset;
        if (bytes > need)
            bytes = need;
        memcpy(d->pes + d->length, ts + offset, bytes);
        d->length += bytes;
        offset += bytes;
        if (!d->target && d->length == 6) {
            unsigned length = ((unsigned)d->pes[4] << 8) | d->pes[5];
            if (d->pes[0] || d->pes[1] || d->pes[2] != 1 || length < 7) {
                d->active = 0;
                d->malformed++;
                return;
            }
            d->target = 6u + length;
        }
        if (d->target && d->length == d->target) {
            finish_pes(d);
            d->active = 0;
        }
    }
}
