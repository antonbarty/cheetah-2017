#!/usr/bin/env python
# -*- coding: utf-8 -*-

import sys
import os
import shutil
import urllib2
import tarfile

def query_yes_no(question, default="yes"):
    """Ask a yes/no question via raw_input() and return their answer.

    "question" is a string that is presented to the user.
    "default" is the presumed answer if the user just hits <Enter>.
        It must be "yes" (the default), "no" or None (meaning
        an answer is required of the user).

    The "answer" return value is one of "yes" or "no".
    """
    valid = {"yes":True,   "y":True,  "ye":True,
             "no":False,     "n":False}
    if default == None:
        prompt = " [y/n] "
    elif default == "yes":
        prompt = " [Y/n] "
    elif default == "no":
        prompt = " [y/N] "
    else:
        raise ValueError("invalid default answer: '%s'" % default)

    while True:
        sys.stdout.write(question + prompt)
        choice = raw_input().lower()
        if default is not None and choice == '':
            return valid[default]
        elif choice in valid:
            return valid[choice]
        else:
            sys.stdout.write("Please respond with 'yes' or 'no' "\
                             "(or 'y' or 'n').\n")


def download_file(url):
    file_name = url.split('/')[-1]
    u = urllib2.urlopen(url)
    f = open(file_name, 'wb')
    meta = u.info()
    file_size = int(meta.getheaders("Content-Length")[0])
    print "Downloading: %s Bytes: %s" % (file_name, file_size)

    file_size_dl = 0
    block_sz = 8192
    while True:
        buffer = u.read(block_sz)
        if not buffer:
            break

        file_size_dl += len(buffer)
        f.write(buffer)
        status = r"%10d  [%3.2f%%]" % (file_size_dl, file_size_dl * 100. / file_size)
        status = status + chr(8)*(len(status)+1)
        print status,
    f.close()

buildir =  os.path.realpath(os.getcwd()+"/..")
srcdir = os.getcwd()
if(len(sys.argv) > 1):
    srcdir = os.path.realpath(sys.argv[1])
    os.chdir(sys.argv[1])
filename = os.path.realpath("data/LCLS/amo/amo55912/xtc/e190-r0001-s03-c00.xtc")

# Check if we have the necessary files
if(not os.path.isfile(filename)):
    print "Could not find %s" % (filename)
    print "Downloading data from CXIDB.org..."
    download_file("http://cxidb.org/cheetah/tests/basic_data.tar.gz")
    tgz = tarfile.open("basic_data.tar.gz")
    tgz.extractall()
    tgz.close()
    os.unlink("basic_data.tar.gz")

test = "basic"
outputdir = "%s/Testing/Temporary/%s" % (buildir,test)
try:
    os.makedirs(outputdir)
except os.error:
    pass

shutil.copy("conf/basic/psana.cfg",outputdir)
shutil.copy("conf/basic/cheetah.ini",outputdir)
if(os.path.lexists(outputdir+"/masks")):
    os.unlink(outputdir+"/masks")
os.symlink(srcdir+"/masks/"+test, outputdir+"/masks")

os.chdir(outputdir)
ret = os.system("%s/source/psana/psana -c psana.cfg %s" % (buildir,filename))
if(ret != 0):
    sys.exit(ret)
sys.exit(0)
