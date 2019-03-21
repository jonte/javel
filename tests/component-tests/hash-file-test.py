#!/usr/bin/env python3
import unittest
import os
import tempfile
from jvl_test import JvlTest

class HashFileTest(JvlTest):
    def test_hash_file(self):
        with tempfile.TemporaryDirectory() as d:
            os.chdir(d)
            self.jvl_e("init")
            self.e("echo test > test_file")
            hash = self.jvl_e("hash-file test_file")
            out = self.e("git cat-file blob {}".format(hash))
            out2 = self.jvl_e("cat-file {}".format(hash))
            self.assertEqual(out, "test")
            self.assertEqual(out, out2)

if __name__ == '__main__':
    unittest.main()
