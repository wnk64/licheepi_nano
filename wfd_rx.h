#ifndef WFD_RX_H
#define WFD_RX_H

#include <errno.h>
#include <stdint.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/uio.h>

#define WFD_RX_BATCH 16
#define WFD_RX_BYTES 2048

struct wfd_rx {
    uint8_t packets[WFD_RX_BATCH][WFD_RX_BYTES];
    struct mmsghdr messages[WFD_RX_BATCH];
    struct iovec vectors[WFD_RX_BATCH];
    union {
        struct cmsghdr align;
        unsigned char bytes[CMSG_SPACE(sizeof(uint32_t))];
    } controls[WFD_RX_BATCH];
    int fallback;
    unsigned long calls, packets_received, truncated;
};

static inline int wfd_rx_read(int fd, struct wfd_rx *rx, unsigned int limit)
{
    if (!limit || limit > WFD_RX_BATCH) { errno = EINVAL; return -1; }
    if (rx->fallback) limit = 1;
    for (unsigned int i = 0; i < limit; i++) {
        memset(&rx->messages[i], 0, sizeof(rx->messages[i]));
        rx->vectors[i].iov_base = rx->packets[i];
        rx->vectors[i].iov_len = WFD_RX_BYTES;
        rx->messages[i].msg_hdr.msg_iov = &rx->vectors[i];
        rx->messages[i].msg_hdr.msg_iovlen = 1;
        rx->messages[i].msg_hdr.msg_control = rx->controls[i].bytes;
        rx->messages[i].msg_hdr.msg_controllen = sizeof(rx->controls[i].bytes);
    }
    int count;
    rx->calls++;
    if (limit > 1) {
        count = recvmmsg(fd, rx->messages, limit, MSG_DONTWAIT, NULL);
        if (count < 0 && errno == ENOSYS) {
            rx->fallback = 1;
            rx->calls++;
            ssize_t n = recvmsg(fd, &rx->messages[0].msg_hdr, MSG_DONTWAIT);
            count = n < 0 ? -1 : 1;
            if (n >= 0) rx->messages[0].msg_len = (unsigned int)n;
        }
    } else {
        ssize_t n = recvmsg(fd, &rx->messages[0].msg_hdr, MSG_DONTWAIT);
        count = n < 0 ? -1 : 1;
        if (n >= 0) rx->messages[0].msg_len = (unsigned int)n;
    }
    if (count > 0) rx->packets_received += (unsigned long)count;
    return count;
}

#endif
