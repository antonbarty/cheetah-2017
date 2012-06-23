#!/usr/bin/python

import sys

if __name__ == '__main__':
  template = sys.argv[1]
  projname = sys.argv[2]
  useiobuf = sys.argv[3]
  platform = sys.argv[4]
  options  = sys.argv[5]

  ftemplate = open(template, 'r')
  foptions = open(options, 'w')

  lines = ftemplate.readlines()
  for line in lines:
    line = line.replace('PROJNAME', projname)
    line = line.replace('USEIOBUF', useiobuf)
    line = line.replace('PLATFORM', platform)
    foptions.write(line)

  ftemplate.close()
  foptions.close()
