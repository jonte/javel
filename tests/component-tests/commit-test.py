#!/usr/bin/env python3
import unittest
import os
import tempfile
from jvl_test import JvlTest, TmpGitDir
import re

class CommitTest(JvlTest):
    def test_commit_on_repo_initialized_by_jvl(self):
        with TmpGitDir(self) as d:
            self.e("echo test > test_file")
            self.jvl_e("commit \"Test commit\"")
            hash_jvl = self.jvl_e("show")
            hash_git = self.e("git rev-parse HEAD")
            self.assertEqual(re.split(" |\n", hash_jvl)[1], hash_git.strip())

    def test_commit_on_repo_initialized_by_git(self):
        with TmpGitDir(self) as d:
            self.e("echo test > test_file")
            self.e("echo test2 > test_file2")
            self.e("git add test_file")
            self.e("git commit -m \"Test commit\"")
            self.jvl_e("commit \"Test commit2\"")
            hash_jvl = self.jvl_e("show")
            hash_git = self.e("git rev-parse HEAD")
            self.assertEqual(re.split(" |\n", hash_jvl)[1], hash_git.strip())

if __name__ == '__main__':
    unittest.main()
