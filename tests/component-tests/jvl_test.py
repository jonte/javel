#!/usr/bin/env python3
import unittest
import subprocess
import os
import tempfile

class JvlTest(unittest.TestCase):
    jvl = os.environ.get("JVL")

    def e(self, cmd):
        print("Executing: {}".format(cmd))
        (ret, res) = subprocess.getstatusoutput(cmd)
        if (ret != 0):
            print("Bad return: {}".format(res))
        self.assertEqual(ret, 0)
        return res

    def jvl_e(self, cmd):
        return self.e("{} {}".format(self.jvl, cmd))

class TmpGitDir():
    def __init__(self, jvl):
        self.jvl = jvl

    def __enter__(self):
        self.d = tempfile.TemporaryDirectory()
        os.chdir(self.d.name)
        self.jvl.jvl_e("init")
        return self.d

    def __exit__(self, type, value, traceback):
        self.d.cleanup()
