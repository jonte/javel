#!/usr/bin/env python3
import unittest
import subprocess
import os

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
