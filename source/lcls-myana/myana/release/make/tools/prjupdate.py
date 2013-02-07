#!/usr/bin/python

import sys
import os
import shutil
import glob

from options import Options

def __prjupdate(options):
  prjdir = options.project

  if os.path.exists(prjdir):
    if os.path.exists(os.path.join(prjdir, 'Makefile')):
      os.rename(os.path.join(prjdir, 'Makefile'),
		os.path.join(prjdir, 'Makefile.orig'))
    if os.path.exists(os.path.join(prjdir, 'flags.mk')):
      os.rename(os.path.join(prjdir, 'flags.mk'),
		os.path.join(prjdir, 'flags.mk.orig'))
    shutil.copy('make/share/Makefile.project.template',
		os.path.join(prjdir, 'Makefile'))
    if options.hw is None:
      shutil.copy('make/sw/flags.mk.template',
		  os.path.join(prjdir, 'flags.mk'))
    else:
      shutil.copy('make/hw/flags.mk.template',
		  os.path.join(prjdir, 'flags.mk'))
  else:
    print "Project %s does not exist" %(prjdir)

  pkgmakes = glob.glob(os.path.join(prjdir, '*/Makefile'))
  for pkgmake in pkgmakes:
    os.rename(pkgmake, pkgmake+'.orig')
    if options.hw is None:
      shutil.copy('make/sw/Makefile.package.template', pkgmake)
    else:
      shutil.copy('make/hw/Makefile.package.template', pkgmake)


if __name__ == '__main__':
  options = Options(['project'], [], ['hw'])
  try:
    options.parse()
  except Exception, msg:
    options.usage(str(msg))
    sys.exit()

  __prjupdate(options)
