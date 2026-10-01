#ifndef FRAME_GAP_H
#define FRAME_GAP_H

#include <stdint.h>

typedef struct {
    uint64_t total;
    uint64_t window_frames;
    uint64_t over50;
    uint64_t over100;
    uint64_t over250;
    uint64_t max_gap_us;
    uint64_t lifetime_max_gap_us;
    int64_t last_us;
    int64_t report_us;
} frame_gap_t;

static inline void frame_gap_success(frame_gap_t *s, int64_t now)
{
    if (s->total && now >= s->last_us) {
        uint64_t gap = (uint64_t)(now - s->last_us);
        if (gap > s->max_gap_us) s->max_gap_us = gap;
        if (gap > s->lifetime_max_gap_us) s->lifetime_max_gap_us = gap;
        if (gap > 50000) s->over50++;
        if (gap > 100000) s->over100++;
        if (gap > 250000) s->over250++;
    }
    s->last_us = now;
    s->total++;
    s->window_frames++;
}

static inline uint64_t frame_gap_idle(const frame_gap_t *s, int64_t now)
{
    int64_t base = s->total ? s->last_us : s->report_us;
    return now >= base ? (uint64_t)(now - base) : 0;
}

static inline void frame_gap_reset_window(frame_gap_t *s, int64_t now)
{
    s->window_frames = s->over50 = s->over100 = s->over250 = 0;
    s->max_gap_us = 0;
    s->report_us = now;
}

#endif
