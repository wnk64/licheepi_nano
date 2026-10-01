#include "wfd_lpcm.h"
#include <alsa/asoundlib.h>
#include <errno.h>
#include <pthread.h>
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PCM_QUEUE_BYTES 16384u
struct audio_state {
    struct wfd_lpcm demux;
    uint8_t queue[PCM_QUEUE_BYTES];
    size_t head, count;
    pthread_mutex_t lock;
    pthread_cond_t ready;
    pthread_t thread;
    atomic_int stop;
    int failed;
    char device[128];
    unsigned long long received, written, dropped, underruns;
};
static struct audio_state *audio;

static void pcm_arrived(void *opaque, const uint8_t *data, size_t bytes, int64_t pts)
{
    struct audio_state *s = opaque;
    pthread_mutex_lock(&s->lock);
    if (!s->received)
        fprintf(stderr, "LPCM: PID=1100 S16_BE 48000Hz stereo first_pts=%lld bytes=%zu\n",
                (long long)pts, bytes);
    s->received += bytes;
    if (atomic_load(&s->stop) || s->failed) {
        pthread_mutex_unlock(&s->lock);
        return;
    }
    if (bytes > PCM_QUEUE_BYTES) {
        s->dropped += bytes - PCM_QUEUE_BYTES;
        data += bytes - PCM_QUEUE_BYTES;
        bytes = PCM_QUEUE_BYTES;
    }
    if (s->count + bytes > PCM_QUEUE_BYTES) {
        size_t discard = s->count + bytes - PCM_QUEUE_BYTES;
        s->head = (s->head + discard) % PCM_QUEUE_BYTES;
        s->count -= discard;
        s->dropped += discard;
    }
    size_t tail = (s->head + s->count) % PCM_QUEUE_BYTES;
    size_t first = PCM_QUEUE_BYTES - tail;
    if (first > bytes)
        first = bytes;
    memcpy(s->queue + tail, data, first);
    memcpy(s->queue, data + first, bytes - first);
    s->count += bytes;
    pthread_cond_signal(&s->ready);
    pthread_mutex_unlock(&s->lock);
}

static void *audio_thread(void *opaque)
{
    struct audio_state *s = opaque;
    snd_pcm_t *pcm = NULL;
    uint8_t block[4096];
    int error = 0;
    while (!atomic_load(&s->stop)) {
        pthread_mutex_lock(&s->lock);
        while (!s->count && !atomic_load(&s->stop))
            pthread_cond_wait(&s->ready, &s->lock);
        size_t bytes = s->count < sizeof(block) ? s->count : sizeof(block);
        size_t first = PCM_QUEUE_BYTES - s->head;
        if (first > bytes)
            first = bytes;
        memcpy(block, s->queue + s->head, first);
        memcpy(block + first, s->queue, bytes - first);
        s->head = (s->head + bytes) % PCM_QUEUE_BYTES;
        s->count -= bytes;
        pthread_mutex_unlock(&s->lock);
        if (atomic_load(&s->stop))
            break;
        if (!pcm) {
            error = snd_pcm_open(&pcm, s->device, SND_PCM_STREAM_PLAYBACK, SND_PCM_NONBLOCK);
            if (error < 0)
                break;
            /* ALSA plug handles endian conversion for Codec or BlueALSA. */
            error = snd_pcm_set_params(pcm, SND_PCM_FORMAT_S16_BE,
                                      SND_PCM_ACCESS_RW_INTERLEAVED, 2, 48000, 1, 40000);
            if (error < 0)
                break;
            fprintf(stderr, "LPCM output: device=%s S16_BE 48000Hz stereo latency=40ms\n", s->device);
        }
        size_t offset = 0;
        while (offset < bytes && !atomic_load(&s->stop)) {
            snd_pcm_sframes_t frames = snd_pcm_writei(pcm, block + offset, (bytes - offset) / 4);
            if (frames == -EAGAIN || frames == 0) {
                snd_pcm_wait(pcm, 50);
                continue;
            }
            if (frames < 0) {
                pthread_mutex_lock(&s->lock);
                s->underruns++;
                pthread_mutex_unlock(&s->lock);
                error = snd_pcm_recover(pcm, (int)frames, 1);
                if (error < 0)
                    goto finished;
                continue;
            }
            offset += (size_t)frames * 4;
            pthread_mutex_lock(&s->lock);
            s->written += (unsigned long long)frames * 4;
            pthread_mutex_unlock(&s->lock);
        }
    }
finished:
    if (error < 0) {
        fprintf(stderr, "LPCM output disabled, video retained: %s\n", snd_strerror(error));
        pthread_mutex_lock(&s->lock);
        s->failed = 1;
        s->count = 0;
        pthread_mutex_unlock(&s->lock);
    }
    if (pcm) {
        snd_pcm_drop(pcm);
        snd_pcm_close(pcm);
    }
    return NULL;
}

int wfd_audio_start(const char *device)
{
    if (audio || !device || strlen(device) >= sizeof(audio->device))
        return -1;
    struct audio_state *s = calloc(1, sizeof(*s));
    if (!s)
        return -1;
    strcpy(s->device, device);
    atomic_init(&s->stop, 0);
    if (pthread_mutex_init(&s->lock, NULL)) {
        free(s);
        return -1;
    }
    if (pthread_cond_init(&s->ready, NULL)) {
        pthread_mutex_destroy(&s->lock);
        free(s);
        return -1;
    }
    wfd_lpcm_init(&s->demux, pcm_arrived, s);
    pthread_attr_t attr;
    int result = pthread_attr_init(&attr);
    int attr_initialized = !result;
    if (!result)
        result = pthread_attr_setstacksize(&attr, 64 * 1024);
    if (!result)
        result = pthread_create(&s->thread, &attr, audio_thread, s);
    if (attr_initialized)
        pthread_attr_destroy(&attr);
    if (result) {
        pthread_cond_destroy(&s->ready);
        pthread_mutex_destroy(&s->lock);
        free(s);
        return -1;
    }
    audio = s;
    fprintf(stderr, "LPCM enabled: bounded queue=%u PES=%u stack=65536; device=%s\n",
            PCM_QUEUE_BYTES, WFD_PES_MAX, device);
    return 0;
}

void wfd_audio_ts(const uint8_t *ts)
{
    if (audio)
        wfd_lpcm_ts(&audio->demux, ts);
}

void wfd_audio_report(void)
{
    if (!audio)
        return;
    struct audio_state *s = audio;
    pthread_mutex_lock(&s->lock);
    fprintf(stderr, "LPCM stats: PES=%lu bytes=%llu written=%llu queue=%zu dropped=%llu underruns=%llu malformed=%lu discontinuities=%lu\n",
            s->demux.completed, s->received, s->written, s->count, s->dropped,
            s->underruns, s->demux.malformed, s->demux.discontinuities);
    pthread_mutex_unlock(&s->lock);
}

void wfd_audio_stop(void)
{
    if (!audio)
        return;
    struct audio_state *s = audio;
    atomic_store(&s->stop, 1);
    pthread_mutex_lock(&s->lock);
    pthread_cond_broadcast(&s->ready);
    pthread_mutex_unlock(&s->lock);
    pthread_join(s->thread, NULL);
    wfd_audio_report();
    pthread_cond_destroy(&s->ready);
    pthread_mutex_destroy(&s->lock);
    audio = NULL;
    free(s);
}
