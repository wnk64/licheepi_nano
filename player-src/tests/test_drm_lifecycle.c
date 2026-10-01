#include <assert.h>
#include <errno.h>
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "driver/drm_warpper.h"

static atomic_int worker_ready, worker_finished;
static int closed_fd;

static int lifecycle_close(int fd)
{
    assert(atomic_load(&worker_finished));
    closed_fd++;
    return close(fd);
}

#ifndef DRM_TEST_SOURCE
#define DRM_TEST_SOURCE "../driver/drm_warpper.c"
#endif
#define close lifecycle_close
#include DRM_TEST_SOURCE
#undef close

void drmModeFreeConnector(drmModeConnectorPtr ptr)
{
    assert(atomic_load(&worker_finished));
    free(ptr);
}

void drmModeFreePlaneResources(drmModePlaneResPtr ptr)
{
    assert(atomic_load(&worker_finished));
    free(ptr);
}

void drmModeFreeResources(drmModeResPtr ptr)
{
    assert(atomic_load(&worker_finished));
    free(ptr);
}

static drm_warpper_queue_item_t *new_item(void)
{
    drm_warpper_queue_item_t *item = calloc(1, sizeof(*item));
    assert(item);
    return item;
}

static void *blocked_worker(void *arg)
{
    drm_warpper_t *w = arg;
    drm_warpper_queue_item_t *item = new_item();
    atomic_store(&worker_ready, 1);
    /* The return queue is full, so only close can release this producer. */
    assert(spsc_bq_push(&w->layer[0].free_queue, item) == EPIPE);
    assert(w->fd >= 0 && w->res && w->conn && w->plane_res);
    free(item);
    atomic_store(&worker_finished, 1);
    return NULL;
}

int main(void)
{
    for (int iteration = 0; iteration < 50; iteration++) {
        drm_warpper_t w = {0};
        atomic_store(&worker_ready, 0);
        atomic_store(&worker_finished, 0);
        closed_fd = 0;
        w.fd = dup(STDERR_FILENO);
        assert(w.fd >= 0);
        w.conn = calloc(1, sizeof(*w.conn));
        w.plane_res = calloc(1, sizeof(*w.plane_res));
        w.res = calloc(1, sizeof(*w.res));
        assert(w.conn && w.plane_res && w.res);
        assert(drm_warpper_init_layer(&w, 0, 480, 800,
                                     DRM_WARPPER_LAYER_MODE_MB32_NV12) == 0);
        layer_t *layer = &w.layer[0];
        assert(spsc_bq_push(&layer->free_queue, new_item()) == 0);
        assert(spsc_bq_push(&layer->free_queue, new_item()) == 0);
        assert(spsc_bq_push(&layer->display_queue, new_item()) == 0);
        layer->curr_item = new_item();
        atomic_store(&w.thread_running, 1);
        assert(pthread_create(&w.display_thread, NULL, blocked_worker, &w) == 0);
        while (!atomic_load(&worker_ready))
            usleep(100);
        assert(drm_warpper_destroy(&w) == 0);
        assert(atomic_load(&worker_finished) && closed_fd == 1);
        assert(!layer->used && !layer->display_queue.buf && !layer->free_queue.buf);
        assert(!layer->curr_item && w.fd == -1);
        assert(drm_warpper_destroy(&w) == 0 && closed_fd == 1);
    }
    puts("DRM lifecycle: 50 blocked-worker/idempotent teardown cases passed");
    return 0;
}
