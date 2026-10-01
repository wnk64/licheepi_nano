#include "wfd_loss.h"
#include <assert.h>
#include <stdio.h>

static void packet(struct wfd_loss *s, uint16_t sequence, unsigned ssrc)
{
    uint8_t data[12] = {0x80, 33};
    data[2] = sequence >> 8; data[3] = sequence;
    data[11] = ssrc;
    wfd_loss_rtp(s, data, sizeof(data));
}

int main(void)
{
    struct wfd_loss s = {0};
    packet(&s, 65535, 1); packet(&s, 0, 1);
    assert(!s.missing && !s.pending);
    packet(&s, 0, 1); packet(&s, 65535, 1);
    assert(s.duplicates == 1 && s.reordered == 1 && s.sequence == 0);
    packet(&s, 2, 1);
    assert(s.missing == 1 && s.pending);
    wfd_loss_initial_request(&s, 100);
    assert(!wfd_loss_request_due(&s, 100));
    packet(&s, 4, 1);
    assert(!wfd_loss_request_due(&s, 99));
    assert(!wfd_loss_request_due(&s, 1000099));
    assert(wfd_loss_request_due(&s, 1000100));
    assert(s.requests == 1 && !s.pending && s.missing == 2);
    packet(&s, 123, 2);
    assert(s.missing == 2 && !s.pending);
    wfd_loss_socket(&s, 5);
    assert(s.socket_missing == 5 && s.pending);
    assert(!wfd_loss_request_due(&s, 1000101));
    assert(wfd_loss_request_due(&s, 2000100));
    assert(s.requests == 2);
    s.socket_drops = UINT32_MAX;
    wfd_loss_socket(&s, 0);
    assert(s.socket_missing == 6);
    for (unsigned i = 0; i < 100000; i++) {
        packet(&s, (uint16_t)(124 + i), 2);
        wfd_loss_request_due(&s, 2000100u + i);
    }
    assert(s.requests == 2 && s.missing == 2);
    uint8_t invalid[12] = {0};
    wfd_loss_rtp(&s, invalid, 12);
    wfd_loss_rtp(&s, invalid, 1);
    puts("RTP loss wrap/duplicate/reorder/SSRC/overflow/cooldown/100k packets passed");
    return 0;
}
