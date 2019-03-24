#!/usr/bin/env python3
import unittest
import os
from jvl_test import JvlTest, TmpGitDir

class InitTest(JvlTest):
    def test_init(self):
        with TmpGitDir(self) as d:
            self.e("echo test > test_file")
            self.e("git add test_file")
            self.e("git commit test_file -m 'test'")
            self.jvl_e("log")

if __name__ == '__main__':
    unittest.main()
