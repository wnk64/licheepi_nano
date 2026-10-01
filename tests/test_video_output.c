#include "wfd_video.h"
#include <assert.h>
#include <errno.h>
#include <poll.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

static uint64_t now_ms(void)
{
    struct timespec t;
    assert(!clock_gettime(CLOCK_MONOTONIC, &t));
    return (uint64_t)t.tv_sec * 1000 + t.tv_nsec / 1000000;
}

int main(void)
{
    signal(SIGPIPE, SIG_IGN);
    uint8_t data[3000], readback[3000];
    for (size_t i = 0; i < sizeof(data); i++) data[i] = (uint8_t)(i * 7);
    for (int cycle = 0; cycle < 20; cycle++) {
        int p[2];
        assert(!pipe(p));
        assert(!wfd_video_start(p[1]));
        close(p[1]);
        for (int n = 0; n < 100; n++) {
            size_t bytes = (size_t)(n * 31 % 2999 + 1);
            assert(!wfd_video_push(data, bytes));
            size_t got = 0;
            while (got < bytes) {
                struct pollfd ready = { .fd = p[0], .events = POLLIN };
                assert(poll(&ready, 1, 2000) > 0);
                ssize_t count = read(p[0], readback + got, bytes - got);
                assert(count > 0);
                got += (size_t)count;
            }
            assert(!memcmp(data, readback, bytes));
        }
        uint64_t start = now_ms();
        wfd_video_stop();
        assert(now_ms() - start < 1000);
        wfd_video_stop();
        close(p[0]);
    }
    int p[2];
    assert(!pipe(p));
    assert(!wfd_video_start(p[1]));
    close(p[1]);
    int failed = 0;
    for (int i = 0; i < 100; i++) {
        if (wfd_video_push(data, sizeof(data)) < 0) {
            assert(errno == ENOBUFS);
            failed = 1;
            break;
        }
    }
    assert(failed && wfd_video_report() == ENOBUFS);
    uint64_t start = now_ms();
    wfd_video_stop();
    assert(now_ms() - start < 1000);
    close(p[0]);
    assert(!pipe(p));
    assert(!wfd_video_start(p[1]));
    close(p[1]); close(p[0]);
    assert(!wfd_video_push(data, sizeof(data)));
    usleep(100000);
    assert(wfd_video_report() == EPIPE);
    wfd_video_stop();
    puts("Async FIFO exact bytes/wrap/idle/full/full-pipe-stop/EPIPE/20 lifecycles passed");
    return 0;
}
