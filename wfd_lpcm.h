#ifndef WFD_LPCM_H
#define WFD_LPCM_H
#include <stddef.h>
#include <stdint.h>

#define WFD_LPCM_PID 0x1100
#define WFD_PES_MAX 65541u
typedef void (*wfd_pcm_emit)(void *, const uint8_t *, size_t, int64_t);
struct wfd_lpcm {
    uint8_t pes[WFD_PES_MAX];
    size_t length, target;
    int active, have_cc;
    unsigned cc;
    unsigned long packets, completed, malformed, discontinuities;
    wfd_pcm_emit emit;
    void *opaque;
};
void wfd_lpcm_init(struct wfd_lpcm *, wfd_pcm_emit, void *);
void wfd_lpcm_ts(struct wfd_lpcm *, const uint8_t *);
int wfd_audio_start(const char *device);
void wfd_audio_ts(const uint8_t *ts);
void wfd_audio_report(void);
void wfd_audio_stop(void);
#endif
