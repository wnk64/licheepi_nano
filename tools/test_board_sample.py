import unittest
from board_sample import delta, parse_rtp_line, parse_wlan1_line


class SamplingTests(unittest.TestCase):
    def test_nic_counters(self):
        row = " wlan1: 1200000 1000 0 2 0 0 0 0 42 1 0 0 0 0 0 0"
        self.assertEqual(parse_wlan1_line(row), {"rx_bytes": 1200000, "rx_packets": 1000, "rx_drops": 2})
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
        old["wlan1"] = {"rx_bytes": 0}
        new["wlan1"] = {"rx_bytes": 1000000}
        self.assertEqual(delta(old, new)["wlan1_rx_mbps"], 0.8)
        new["wlan1"]["rx_bytes"] = -1
        self.assertIsNone(delta(old, new)["wlan1_rx_mbps"])


if __name__ == "__main__":
    unittest.main()
