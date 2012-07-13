#!/usr/bin/python

import sys
import os

if __name__ == '__main__':
  template = sys.argv[1]
  projname = sys.argv[2]
  disiobuf = sys.argv[3]
  srcdir   = sys.argv[4]
  sdcfile  = sys.argv[5]
  srcfile  = sys.argv[6]
  tclfile  = sys.argv[7]

  ftemplate = open(template, 'r')
  fsrc = open(srcfile, 'r')
  ftcl = open(tclfile, 'w')

  for line in fsrc.readlines():
    tokens = line.split()
    if len(tokens) == 3 and (tokens[0] == 'vhdl' or tokens[0] == 'verilog'):
      lang = tokens[0]
      dep = tokens[1]
      src = tokens[2]
      src = os.path.join(srcdir, src)
      if os.access(src, os.R_OK):
        ftcl.write('add_file -%s -lib %s %s\n' %(lang, dep, src))
      else:
        ftcl.close()
	if os.access(tclfile, os.R_OK):
	  os.remove(tclfile)
        raise 'cannot find user core file %s' %(src)

  sdcfile = os.path.join(srcdir, sdcfile)
  ftcl.write('\nadd_file -constraint %s\n\n' %(sdcfile))

  lines = ftemplate.readlines()
  for line in lines:
    line = line.replace('PROJNAME', projname)
    line = line.replace('DISIOBUF', disiobuf)
    ftcl.write(line)

  ftemplate.close()
  fsrc.close()
  ftcl.close()
