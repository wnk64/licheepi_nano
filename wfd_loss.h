#ifndef WFD_LOSS_H
#define WFD_LOSS_H
#include <stdint.h>
#include <stddef.h>

struct wfd_loss {
    uint16_t sequence;
    uint32_t ssrc, socket_drops;
    uint64_t missing, socket_missing, last_request_us;
    unsigned long duplicates, reordered, requests;
    int have_sequence, have_request, pending;
};

static inline void wfd_loss_rtp(struct wfd_loss *s, const uint8_t *packet, size_t bytes)
{
    if (bytes < 12 || (packet[0] >> 6) != 2) return;
    uint16_t sequence = ((uint16_t)packet[2] << 8) | packet[3];
    uint32_t ssrc = ((uint32_t)packet[8] << 24) | ((uint32_t)packet[9] << 16) |
                    ((uint32_t)packet[10] << 8) | packet[11];
    if (!s->have_sequence || ssrc != s->ssrc) {
        s->have_sequence = 1;
        s->sequence = sequence;
        s->ssrc = ssrc;
        return;
    }
    uint16_t step = sequence - s->sequence;
    if (!step) { s->duplicates++; return; }
    if (step >= 32768) { s->reordered++; return; }
    if (step > 1) {
        s->missing += step - 1;
        s->pending = 1;
    }
    s->sequence = sequence;
}

static inline void wfd_loss_socket(struct wfd_loss *s, uint32_t drops)
{
    if (drops != s->socket_drops) {
        s->socket_missing += (uint32_t)(drops - s->socket_drops);
        s->socket_drops = drops;
        s->pending = 1;
    }
}

static inline void wfd_loss_initial_request(struct wfd_loss *s, uint64_t now)
{
    s->have_request = 1;
    s->last_request_us = now;
    s->pending = 0;
}

static inline int wfd_loss_request_due(struct wfd_loss *s, uint64_t now)
{
    if (!s->pending) return 0;
    if (s->have_request && (now < s->last_request_us ||
                           now - s->last_request_us < 1000000u)) return 0;
    wfd_loss_initial_request(s, now);
    s->requests++;
    return 1;
}
#endif
