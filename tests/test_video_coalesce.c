#define _GNU_SOURCE
#include "wfd_video.c"
#include <assert.h>
#include <signal.h>

static uint64_t now_us(void)
{
    struct timespec ts;
    assert(!clock_gettime(CLOCK_MONOTONIC, &ts));
    return (uint64_t)ts.tv_sec * 1000000 + ts.tv_nsec / 1000;
}

static void read_exact(int fd, uint8_t *out, size_t bytes)
{
    size_t got = 0;
    while (got < bytes) {
        struct pollfd p = {.fd = fd, .events = POLLIN};
        assert(poll(&p, 1, 2000) > 0);
        ssize_t n = read(fd, out + got, bytes - got);
        assert(n > 0);
        got += (size_t)n;
    }
}

static void wait_written(size_t expected)
{
    uint64_t start = now_us();
    for (;;) {
        pthread_mutex_lock(&output->lock);
        int ready = output->written == expected;
        assert(!output->failed);
        pthread_mutex_unlock(&output->lock);
        if (ready) return;
        assert(now_us() - start < 1000000);
        usleep(100);
    }
}

int main(void)
{
    signal(SIGPIPE, SIG_IGN);
    assert(!setenv("WFD_VIDEO_COALESCE", "1", 1));
    uint8_t input[12000], got[12000];
    for (size_t i = 0; i < sizeof(input); i++) input[i] = (uint8_t)(i * 19);
    int p[2];
    assert(!pipe(p));
    assert(!wfd_video_start(p[1]));
    close(p[1]);
    uint64_t start = now_us();
    assert(!wfd_video_push(input, 1));
    read_exact(p[0], got, 1);
    assert(now_us() - start >= 2000); /* Tail flush uses the monotonic timer. */
    assert(got[0] == input[0]);
    for (size_t off = 0; off < sizeof(input); ) {
        size_t n = sizeof(input) - off;
        if (n > 113) n = 113;
        assert(!wfd_video_push(input + off, n));
        off += n;
    }
    read_exact(p[0], got, sizeof(got));
    assert(!memcmp(input, got, sizeof(input)));
    wait_written(sizeof(input) + 1);
    pthread_mutex_lock(&output->lock);
    assert(output->coalesce && output->coalesce_blocks > 0);
    assert(output->submitted == sizeof(input) + 1);
    assert(output->written == output->submitted);
    assert(!output->failed && !output->full);
    unsigned long previous_blocks = output->coalesce_blocks;
    pthread_mutex_unlock(&output->lock);
    assert(!wfd_video_push(input, VIDEO_BLOCK_BYTES));
    read_exact(p[0], got, VIDEO_BLOCK_BYTES);
    assert(!memcmp(input, got, VIDEO_BLOCK_BYTES));
    wait_written(sizeof(input) + 1 + VIDEO_BLOCK_BYTES);
    pthread_mutex_lock(&output->lock);
    assert(output->coalesce_blocks == previous_blocks);
    pthread_mutex_unlock(&output->lock);
    wfd_video_report();
    wfd_video_stop();
    close(p[0]);
    for (int i = 0; i < 50; i++) {
        assert(!pipe(p));
        assert(!wfd_video_start(p[1]));
        close(p[1]);
        assert(!wfd_video_push(input, 1));
        start = now_us();
        wfd_video_stop();
        assert(now_us() - start < 1000000);
        close(p[0]);
    }
    puts("Coalesced exact bytes/3ms tail flush/repeated stop passed");
    return 0;
}
