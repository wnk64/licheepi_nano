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
#include <time.h>
#include <unistd.h>

#define VIDEO_QUEUE_BYTES (128u * 1024u)
#define VIDEO_BLOCK_BYTES 4096u
#define VIDEO_COALESCE_NS 3000000L
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
    int coalesce;
    unsigned long write_calls, wake_signals, coalesce_blocks;
};
static struct video_output *output;

static void *output_thread(void *opaque)
{
    struct video_output *s = opaque;
    uint8_t block[VIDEO_BLOCK_BYTES];
    while (!atomic_load(&s->stop)) {
        pthread_mutex_lock(&s->lock);
        while (!s->count && !atomic_load(&s->stop))
            pthread_cond_wait(&s->ready, &s->lock);
        if (s->coalesce && s->count && s->count < sizeof(block) &&
            !atomic_load(&s->stop)) {
            struct timespec deadline;
            if (clock_gettime(CLOCK_MONOTONIC, &deadline) != 0) {
                s->failed = errno;
                atomic_store(&s->stop, 1);
            } else {
                deadline.tv_nsec += VIDEO_COALESCE_NS;
                if (deadline.tv_nsec >= 1000000000L) {
                    deadline.tv_sec++;
                    deadline.tv_nsec -= 1000000000L;
                }
                s->coalesce_blocks++;
                while (s->count < sizeof(block) && !atomic_load(&s->stop)) {
                    int rc = pthread_cond_timedwait(&s->ready, &s->lock, &deadline);
                    if (rc == ETIMEDOUT) break;
                    if (rc != 0) {
                        s->failed = rc;
                        atomic_store(&s->stop, 1);
                    }
                }
            }
        }
        if (atomic_load(&s->stop)) {
            pthread_mutex_unlock(&s->lock);
            break;
        }
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
                s->write_calls++;
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
    const char *env = getenv("WFD_VIDEO_COALESCE");
    s->coalesce = env && strcmp(env, "1") == 0;
    pthread_condattr_t cond_attr;
    int cond_initialized = 0, cond_error = 0;
    if (s->coalesce) {
        cond_error = pthread_condattr_init(&cond_attr);
        if (!cond_error) {
            cond_initialized = 1;
            cond_error = pthread_condattr_setclock(&cond_attr, CLOCK_MONOTONIC);
        }
    }
    if (!cond_error)
        cond_error = pthread_cond_init(&s->ready, s->coalesce ? &cond_attr : NULL);
    if (cond_initialized) pthread_condattr_destroy(&cond_attr);
    if (cond_error) {
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
    fprintf(stderr, "FIFO coalesce: enabled=%d threshold=%u wait_us=%ld monotonic=1\n",
            s->coalesce, VIDEO_BLOCK_BYTES, s->coalesce ? VIDEO_COALESCE_NS / 1000 : 0L);
    return 0;
}

int wfd_video_push(const void *data, size_t bytes)
{
    struct video_output *s = output;
    if (!s) return -1;
    pthread_mutex_lock(&s->lock);
    size_t previous_count = s->count;
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
    if (!s->coalesce || error || !previous_count ||
        (previous_count < VIDEO_BLOCK_BYTES && s->count >= VIDEO_BLOCK_BYTES)) {
        s->wake_signals++;
        pthread_cond_signal(&s->ready);
    }
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
    fprintf(stderr, "FIFO IPC: writes=%lu wake_signals=%lu coalesce_blocks=%lu enabled=%d\n",
            s->write_calls, s->wake_signals, s->coalesce_blocks, s->coalesce);
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
