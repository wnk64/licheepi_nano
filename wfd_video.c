#include "wfd_video.h"
#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <pthread.h>
#include <stdatomic.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define VIDEO_QUEUE_BYTES (128u * 1024u)
struct video_output {
    uint8_t queue[VIDEO_QUEUE_BYTES];
    size_t head, count, peak, inflight;
    int fd, failed;
    atomic_int stop;
    pthread_mutex_t lock;
    pthread_cond_t ready;
    pthread_t thread;
    unsigned long long submitted, written, discarded;
    unsigned long full;
};
static struct video_output *output;

static void *output_thread(void *opaque)
{
    struct video_output *s = opaque;
    uint8_t block[4096];
    while (!atomic_load(&s->stop)) {
        pthread_mutex_lock(&s->lock);
        while (!s->count && !atomic_load(&s->stop))
            pthread_cond_wait(&s->ready, &s->lock);
        size_t bytes = s->count < sizeof(block) ? s->count : sizeof(block);
        size_t first = VIDEO_QUEUE_BYTES - s->head;
        if (first > bytes) first = bytes;
        memcpy(block, s->queue + s->head, first);
        memcpy(block + first, s->queue, bytes - first);
        s->head = (s->head + bytes) % VIDEO_QUEUE_BYTES;
        s->count -= bytes;
        s->inflight = bytes;
        pthread_mutex_unlock(&s->lock);
        size_t off = 0;
        while (off < bytes && !atomic_load(&s->stop)) {
            ssize_t n = write(s->fd, block + off, bytes - off);
            if (n > 0) {
                off += (size_t)n;
                pthread_mutex_lock(&s->lock);
                s->written += (size_t)n;
                s->inflight -= (size_t)n;
                pthread_mutex_unlock(&s->lock);
            } else if (n < 0 && errno == EINTR) {
                continue;
            } else if (n < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
                struct pollfd p = { .fd = s->fd, .events = POLLOUT };
                int ready = poll(&p, 1, 50);
                if (ready >= 0 || errno == EINTR) continue;
                pthread_mutex_lock(&s->lock);
                s->failed = errno;
                pthread_mutex_unlock(&s->lock);
                atomic_store(&s->stop, 1);
            } else {
                int error = n == 0 ? EIO : errno;
                pthread_mutex_lock(&s->lock);
                s->failed = error;
                pthread_mutex_unlock(&s->lock);
                atomic_store(&s->stop, 1);
            }
        }
    }
    return NULL;
}

int wfd_video_start(int fd)
{
    if (output) return -1;
    struct video_output *s = calloc(1, sizeof(*s));
    if (!s) return -1;
    s->fd = dup(fd);
    if (s->fd < 0) { free(s); return -1; }
    int flags = fcntl(s->fd, F_GETFL);
    if (flags < 0 || fcntl(s->fd, F_SETFL, flags | O_NONBLOCK) < 0) {
        close(s->fd); free(s); return -1;
    }
    atomic_init(&s->stop, 0);
    if (pthread_mutex_init(&s->lock, NULL)) {
        close(s->fd); free(s); return -1;
    }
    if (pthread_cond_init(&s->ready, NULL)) {
        pthread_mutex_destroy(&s->lock); close(s->fd); free(s); return -1;
    }
    pthread_attr_t attr;
    int error = pthread_attr_init(&attr);
    int initialized = !error;
    if (!error) error = pthread_attr_setstacksize(&attr, 64 * 1024);
    if (!error) error = pthread_create(&s->thread, &attr, output_thread, s);
    if (initialized) pthread_attr_destroy(&attr);
    if (error) {
        pthread_cond_destroy(&s->ready); pthread_mutex_destroy(&s->lock);
        close(s->fd); free(s); return -1;
    }
    output = s;
    fprintf(stderr, "FIFO async: queue=%u block=4096 stack=65536 full=fail\n", VIDEO_QUEUE_BYTES);
    return 0;
}

int wfd_video_push(const void *data, size_t bytes)
{
    struct video_output *s = output;
    if (!s) return -1;
    pthread_mutex_lock(&s->lock);
    int error = s->failed ? s->failed : (atomic_load(&s->stop) ? ECANCELED : 0);
    if (!error && bytes > VIDEO_QUEUE_BYTES - s->count) {
        s->full++;
        error = s->failed = ENOBUFS;
        atomic_store(&s->stop, 1);
    }
    if (!error) {
        size_t tail = (s->head + s->count) % VIDEO_QUEUE_BYTES;
        size_t first = VIDEO_QUEUE_BYTES - tail;
        if (first > bytes) first = bytes;
        memcpy(s->queue + tail, data, first);
        memcpy(s->queue, (const uint8_t *)data + first, bytes - first);
        s->count += bytes;
        s->submitted += bytes;
        if (s->count > s->peak) s->peak = s->count;
    }
    pthread_cond_signal(&s->ready);
    pthread_mutex_unlock(&s->lock);
    if (error) errno = error;
    return error ? -1 : 0;
}

int wfd_video_report(void)
{
    if (!output) return 0;
    struct video_output *s = output;
    pthread_mutex_lock(&s->lock);
    fprintf(stderr, "FIFO async stats: submitted=%llu written=%llu queue=%zu inflight=%zu peak=%zu full=%lu failed=%d discarded=%llu\n",
            s->submitted, s->written, s->count, s->inflight, s->peak,
            s->full, s->failed, s->discarded);
    int failed = s->failed;
    pthread_mutex_unlock(&s->lock);
    return failed;
}

void wfd_video_stop(void)
{
    if (!output) return;
    struct video_output *s = output;
    atomic_store(&s->stop, 1);
    pthread_mutex_lock(&s->lock);
    pthread_cond_broadcast(&s->ready);
    pthread_mutex_unlock(&s->lock);
    pthread_join(s->thread, NULL);
    s->discarded = s->count + s->inflight;
    wfd_video_report();
    close(s->fd);
    pthread_cond_destroy(&s->ready);
    pthread_mutex_destroy(&s->lock);
    free(s);
    output = NULL;
}
