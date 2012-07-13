#!/usr/bin/python

import sys
import os
import glob

if __name__ == '__main__':
  xipcores = sys.argv[1]
  usrcores = sys.argv[2]
  prjfile = sys.argv[3]
  srcfile = sys.argv[4]

  fprj = open(prjfile, 'w')

  if len(sys.argv) > 5:
    lsofile = sys.argv[5]
    flso = open(lsofile, 'r')
    for library in flso.readlines():
      library = library.strip()
      paos = glob.glob('%s/%s/data/*.pao' %(xipcores, library))
      for pao in paos:
        fpao = open(pao, 'r')
        for depline in fpao.readlines():
          tokens = depline.split()
          if len(tokens) > 2 and tokens[0] == 'lib':
            dep = tokens[1]
            src = tokens[2]
            if src == 'all':
              vhdls = glob.glob('%s/%s/hdl/vhdl/*.vhd' %(xipcores, dep))
              verilogs = glob.glob('%s/%s/hdl/verilog/*.v' %(xipcores, dep))
              for vhdl in vhdls:
                fprj.write('vhdl %s %s\n' %(dep, vhdl))
              for verilog in verilogs:
                fprj.write('verilog %s %s\n' %(dep, verilog))
            else:
              vhdl = '%s/%s/hdl/vhdl/%s.vhd' %(xipcores, dep, src)
              verilog = '%s/%s/hdl/verilog/%s.v' %(xipcores, dep, src)
              if os.access(vhdl, os.R_OK):
                fprj.write('vhdl %s %s\n' %(dep, vhdl))
              elif os.access(verilog, os.R_OK):
                fprj.write('verilog %s %s\n' %(dep, verilog))
              else:
                fprj.close()
                os.remove(prjfile)
                raise 'cannot find core file %s or %s' %(vhdl, verilog)
    flso.close()
          
  fsrc = open(srcfile, 'r')
  for line in fsrc.readlines():
    tokens = line.split()
    if len(tokens) == 3 and (tokens[0] == 'vhdl' or tokens[0] == 'verilog'):
      lang = tokens[0]
      dep = tokens[1]
      src = tokens[2]
      src = os.path.join(usrcores, src)
      if os.access(src, os.R_OK):
        fprj.write('%s %s %s\n' %(lang, dep, src))
      else:
        fprj.close()
        os.remove(prjfile)
        raise 'cannot find user core file %s' %(src)

  fsrc.close()
  fprj.close()
