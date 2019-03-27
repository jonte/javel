#!/usr/bin/env python3

import unittest
import os
from jvl_test import JvlTest, TmpGitDir

class LsFilesTest(JvlTest):
    def test_ls_files(self):
        with TmpGitDir(self) as d:
            self.e("echo test > test1")
            self.e("echo test > test2")
            self.e("echo test > test3")
            self.e("git add test1 test2 test3")
            output = self.jvl_e("ls-files")

            self.assertIn("test1", output)
            self.assertIn("test2", output)
            self.assertIn("test3", output)
            self.assertNotIn("test4", output)

if __name__ == '__main__':
    unittest.main()
