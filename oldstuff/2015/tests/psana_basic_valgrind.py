#!/usr/bin/env python
# -*- coding: utf-8 -*-

import sys
import os
import shutil
import urllib2
import tarfile


buildir =  os.path.realpath(os.getcwd()+"/..")
srcdir = os.getcwd()
if(len(sys.argv) > 1):
    srcdir = os.path.realpath(sys.argv[1])
    os.chdir(sys.argv[1])
filename = os.path.realpath("data/LCLS/amo/amo55912/xtc/e190-r0001-s03-c00.xtc")

test = "basic"
outputdir = "%s/Testing/Temporary/%s" % (buildir,test)
os.chdir(outputdir)
ret = os.system("valgrind --suppressions=%s/valgrind/supressions.txt --error-exitcode=1 --leak-check=yes --tool=memcheck %s/source/psana/psana -c psana.cfg %s" % (srcdir,buildir,filename))
if(ret != 0):
    sys.exit(ret)
sys.exit(0)
