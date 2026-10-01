#define _GNU_SOURCE
#include "wfd_rx.h"
#include <arpa/inet.h>
#include <assert.h>
#include <stdio.h>
#include <unistd.h>

int main(void)
{
    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    int tx = socket(AF_INET, SOCK_DGRAM, 0);
    assert(fd >= 0 && tx >= 0);
    struct sockaddr_in addr = {.sin_family = AF_INET};
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    assert(bind(fd, (struct sockaddr *)&addr, sizeof(addr)) == 0);
    socklen_t len = sizeof(addr);
    assert(getsockname(fd, (struct sockaddr *)&addr, &len) == 0);
    struct wfd_rx rx = {0};
    assert(wfd_rx_read(fd, &rx, 16) == -1 && errno == EAGAIN);
    assert(wfd_rx_read(fd, &rx, 0) == -1 && errno == EINVAL);
    assert(wfd_rx_read(fd, &rx, 17) == -1 && errno == EINVAL);
    for (uint8_t i = 0; i < 40; i++)
        assert(sendto(tx, &i, 1, 0, (struct sockaddr *)&addr, sizeof(addr)) == 1);
    for (int base = 0; base < 40; base += 16) {
        int count = wfd_rx_read(fd, &rx, 16);
        assert(count == (40 - base > 16 ? 16 : 40 - base));
        for (int i = 0; i < count; i++) {
            assert(rx.messages[i].msg_len == 1);
            assert(rx.packets[i][0] == base + i);
            assert(!(rx.messages[i].msg_hdr.msg_flags & MSG_TRUNC));
        }
    }
    assert(rx.calls == 4 && rx.packets_received == 40);
    uint8_t large[WFD_RX_BYTES + 8] = {0};
    assert(sendto(tx, large, sizeof(large), 0, (struct sockaddr *)&addr, sizeof(addr)) == sizeof(large));
    assert(sendto(tx, large, 0, 0, (struct sockaddr *)&addr, sizeof(addr)) == 0);
    assert(wfd_rx_read(fd, &rx, 16) == 2);
    assert(rx.messages[0].msg_hdr.msg_flags & MSG_TRUNC);
    assert(rx.messages[1].msg_len == 0);
    assert(sendto(tx, large, 1, 0, (struct sockaddr *)&addr, sizeof(addr)) == 1);
    assert(wfd_rx_read(fd, &rx, 16) == 1);
    assert(!(rx.messages[0].msg_hdr.msg_flags & MSG_TRUNC));
    assert(rx.messages[0].msg_hdr.msg_controllen <= sizeof(rx.controls[0].bytes));
    rx.fallback = 1;
    assert(sendto(tx, large, 1, 0, (struct sockaddr *)&addr, sizeof(addr)) == 1);
    assert(wfd_rx_read(fd, &rx, 16) == 1);
    close(fd); close(tx);
    puts("UDP batch16/order/empty/zero/truncated/reuse/fallback passed");
    return 0;
}
