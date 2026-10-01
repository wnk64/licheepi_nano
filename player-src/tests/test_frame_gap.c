#include <assert.h>
#include <stdio.h>
#include "frame_gap.h"

int main(void)
{
    frame_gap_t s = {0};
    s.report_us = 1000;
    assert(frame_gap_idle(&s, 6000) == 5000);
    frame_gap_success(&s, 10000);
    assert(s.total == 1 && s.max_gap_us == 0);
    frame_gap_success(&s, 60000);
    assert(s.over50 == 0);
    frame_gap_success(&s, 160001);
    assert(s.over50 == 1 && s.over100 == 1 && s.over250 == 0);
    frame_gap_success(&s, 410002);
    assert(s.over50 == 2 && s.over100 == 2 && s.over250 == 1);
    assert(s.max_gap_us == 250001 && s.total == 4);
    assert(frame_gap_idle(&s, 810002) == 400000);
    frame_gap_reset_window(&s, 810002);
    assert(s.total == 4 && s.window_frames == 0 && s.max_gap_us == 0);
    assert(s.lifetime_max_gap_us == 250001);
    assert(frame_gap_idle(&s, 910002) == 500000);
    frame_gap_success(&s, 910002);
    assert(s.max_gap_us == 500000 && s.window_frames == 1);
    assert(s.lifetime_max_gap_us == 500000 && s.over250 == 1);
    assert(frame_gap_idle(&s, 1) == 0);
    puts("frame_gap tests passed");
    return 0;
}
