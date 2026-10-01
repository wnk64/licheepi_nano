import unittest
from board_sample import delta, parse_rtp_line


class SamplingTests(unittest.TestCase):
    def test_proc_udp_columns(self):
        row = "43: 0131A8C0:0404 00000000:0000 07 00000000:000069C0 00:00000000 00000000 0 0 161375 2 8fd5c6bb 475"
        self.assertEqual(parse_rtp_line(row), {"inode": 161375, "drops": 475,
                                              "queue_accounting_bytes": 27072})

    def test_interval_and_socket_identity(self):
        old = {"uptime": 10, "cpu": [0, 0, 0, 0, 0, 0, 0, 0],
               "rtp": {"inode": 8, "drops": 20}, "threads": {5: {"ticks": 0, "switches": 10}}}
        new = {"uptime": 20, "cpu": [100, 0, 700, 200, 0, 0, 0, 0],
               "rtp": {"inode": 8, "drops": 30}, "player": 1, "sink": 2,
               "threads": {5: {"ticks": 50, "switches": 30}}, "memory_kib": {}}
        sample = delta(old, new)
        self.assertEqual(sample["busy_pct"], 80)
        self.assertEqual(sample["rtp_drops_delta"], 10)
        self.assertEqual(sample["threads"][5], {"cpu_pct": 5, "voluntary_per_s": 2})
        new["rtp"]["inode"] = 9
        self.assertIsNone(delta(old, new)["rtp_drops_delta"])


if __name__ == "__main__":
    unittest.main()
