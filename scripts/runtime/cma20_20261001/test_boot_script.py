import os
import pathlib
import unittest
from inspect_boot_script import inspect


class BootScriptTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.image = pathlib.Path(os.environ.get("BOOT_SCRIPT_FIXTURE",
            str(pathlib.Path(__file__).with_name("boot-before.scr")))).read_bytes()

    def test_valid_payload_and_metadata(self):
        result = inspect(self.image, self.image[72:])
        self.assertEqual(result["md5"], "89f5aa3504a1f95b9aa9dc3df824e37d")
        self.assertIn("cma=24M", result["script"])
        self.assertEqual(result["script_size"], 241)

    def test_header_corruption_rejected(self):
        image = bytearray(self.image)
        image[40] ^= 1
        with self.assertRaises(ValueError): inspect(image)

    def test_data_corruption_rejected(self):
        image = bytearray(self.image)
        image[-1] ^= 1
        with self.assertRaises(ValueError): inspect(image)

    def test_truncation_and_wrong_source_rejected(self):
        with self.assertRaises(ValueError): inspect(self.image[:-1])
        with self.assertRaises(ValueError): inspect(self.image, b"wrong source")


if __name__ == "__main__": unittest.main()
