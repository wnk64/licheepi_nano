import unittest
from summarize_frame_gaps import summarize


class SummaryTests(unittest.TestCase):
    def test_no_frames_is_not_playback(self):
        self.assertEqual(summarize("video commits: window=0"), {"active_windows": 0})

    def test_actual_gaps_not_pts_lateness(self):
        text = "video clock late: 9999999 us\n"
        text += "video commits: window=200 span_us=5000000 over250=2 max_gap_us=500001 idle_us=12 errors=0\n"
        text += "video commits: window=100 span_us=5000000 over250=1 max_gap_us=800000 idle_us=123 errors=1\n"
        result = summarize(text)
        self.assertEqual(result["commits_per_s"], 30)
        self.assertEqual(result["gaps_over250ms"], 3)
        self.assertEqual(result["max_completed_gap_us"], 800000)
        self.assertEqual(result["max_sampled_idle_us"], 123)
        self.assertEqual(result["errors"], 1)

    def test_zero_frame_windows_after_start_are_not_discarded(self):
        text = "video commits: total=0 window=0\n"
        text += "video commits: total=300 window=300 span_us=5000000 over250=0 max_gap_us=16666 idle_us=0 errors=0\n"
        text += "video commits: total=300 window=0 span_us=5000000 over250=0 max_gap_us=0 idle_us=5000000 errors=0\n"
        result = summarize(text)
        self.assertEqual(result["commits_per_s"], 30)
        self.assertEqual(result["stalled_windows"], 1)
        self.assertEqual(result["max_sampled_idle_us"], 5000000)


if __name__ == "__main__":
    unittest.main()
