#!/usr/bin/env python3

import unittest
import os
from jvl_test import JvlTest, TmpGitDir

class StatusTest(JvlTest):
    def test_status(self):
        with TmpGitDir(self) as d:
            self.e("echo test > test1")
            self.e("echo test > test2")
            self.e("git add test1 test2")
            self.e("echo TEST > test1")
            output = self.jvl_e("status")

            self.assertIn("test1 - MODIFIED", output)
            self.assertIn("test2", output)
            self.assertNotIn("test3", output)

if __name__ == '__main__':
    unittest.main()
