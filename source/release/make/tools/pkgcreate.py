#!/usr/bin/python

import sys
import os
import shutil

from options import Options

def __pkgcreate(options):
  prjdir = options.project
  pkgdir = os.path.join(prjdir, options.package)

  if not os.path.exists(prjdir):
    os.mkdir(prjdir)
    shutil.copy('make/share/Makefile.project.template',
		os.path.join(prjdir, 'Makefile'))
    shutil.copy('make/share/packages.mk.template',
		os.path.join(prjdir, 'packages.mk'))
    if options.hw is None:
      shutil.copy('make/sw/flags.mk.template',
		  os.path.join(prjdir, 'flags.mk'))
    else:
      shutil.copy('make/hw/flags.mk.template',
		  os.path.join(prjdir, 'flags.mk'))

  if not os.path.exists(pkgdir):
    os.mkdir(pkgdir)
    if options.hw is None:
      shutil.copy('make/sw/Makefile.package.template',
		  os.path.join(pkgdir, 'Makefile'))
      shutil.copy('make/sw/constituents.mk.template',
		  os.path.join(pkgdir, 'constituents.mk'))
    else:
      shutil.copy('make/hw/Makefile.package.template',
		  os.path.join(pkgdir, 'Makefile'))
      shutil.copy('make/hw/constituents.mk.template',
		  os.path.join(pkgdir, 'constituents.mk'))
  else:
    print "Package %s already exists" %(pkgdir)


if __name__ == '__main__':
  options = Options(['project', 'package'], [], ['hw'])
  try:
    options.parse()
  except Exception, msg:
    options.usage(str(msg))
    sys.exit()

  __pkgcreate(options)
