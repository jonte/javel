#!/usr/bin/env python3
import unittest
import os
import tempfile
from jvl_test import JvlTest

class InitTest(JvlTest):
    def test_init(self):
        with tempfile.TemporaryDirectory() as d:
            os.chdir(d)
            self.jvl_e("init")
            self.e("echo test > test_file")
            self.e("git add test_file")
            self.e("git commit test_file -m 'test'")
            self.jvl_e("log")

if __name__ == '__main__':
    unittest.main()
