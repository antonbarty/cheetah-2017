#!/usr/bin/env python

import sys
import os
import re
import getpass

if(len(sys.argv) < 3):
    print "Usage: download_psana.py <ana-version> <output-directory>"
    print ""
    print "Example: download_psana.py ana-0.9.1 /davinci/psana/g/psdm/portable/sw/releases/"
    print ""
    print "Will download the components of ana necessary to build cheetah into the output directory"
    sys.exit(0)

ana_version = sys.argv[1] 
output_dir = sys.argv[2]


                    
username = raw_input('Please enter your username on psexport (%s): ' % getpass.getuser())
if username == "": username = getpass.getuser()

# copy release
os.system('ssh '+username+'@psexport.slac.stanford.edu "cd /reg/g/psdm/sw/releases/ && tar czf - '+ana_version+'" | tar xzvf - -C '+output_dir)

# copy package pdsdata
pdsdata_version = os.popen('grep "pdsdata_ver =" '+output_dir+'/'+ana_version+'/pdsdata/SConscript').read()
pdsdata_version = re.search('\d+\.\d+\.\d+',pdsdata_version).group(0)
os.system('ssh '+username+'@psexport.slac.stanford.edu "cd /reg/g/psdm/sw/external/pdsdata/'+pdsdata_version+'/x86_64-rhel6-gcc44-opt && tar czf - pdsdata" | tar xzvf - -C '+output_dir+'/'+ana_version+'/include')
os.system('ssh '+username+'@psexport.slac.stanford.edu "cd /reg/g/psdm/sw/external/pdsdata/ && tar czf - '+pdsdata_version+'" | tar xzvf - -C '+output_dir+'/../external/pdsdata')


#os.system('ssh '+username+'@psexport.slac.stanford.edu "cd /reg/g/psdm/sw/external/root/5.30.06-python2.7/x86_64-rhel6-gcc44-opt/include && tar czf - root" | tar xzvf - -C '+output_dir+'/'+ana_version+'/include')

# Fix broken symlinks
indir = output_dir+'/'+ana_version+'/arch'
for root, dirs, filenames in os.walk(indir):
    for f in filenames:
        fpath = os.path.join(root, f)
        if(os.path.islink(fpath) and not os.path.exists(os.readlink(fpath))):
            orig_path = os.readlink(fpath)
            if(orig_path.find('external')):
                end_path = orig_path[orig_path.find('external/')+9:]
                new_path = '../../../../../external/'+end_path
                if(os.path.exists(os.path.dirname(fpath)+'/'+new_path)):
                    os.unlink(fpath)
                    os.symlink(new_path,fpath)
                else:
#                    print "Can't find: "+os.path.dirname(fpath)+'/'+new_path
                    new_path = output_dir+"../../../../../common/package/"+end_path
                    if(os.path.exists(new_path)):
                        os.unlink(fpath)
                        os.symlink(new_path,fpath)
#                    else:
#                        print "Could not find link:"+orig_path

# Copy package ndarray (this step became necessary with ana-0.10)
ndarray_version = os.popen('grep "pkg_ver =" '+output_dir+'/'+ana_version+'/ndarray_ext/SConscript').read()
ndarray_version = re.search('\d+\.\d+\.\d+',ndarray_version).group(0)
os.system('mkdir '+output_dir+'/'+ana_version+'/ndarray')
os.system('ssh '+username+'@psexport.slac.stanford.edu "cd /reg/g/psdm/sw/external/ndarray/'+ndarray_version+'/x86_64-rhel6-gcc44-opt && tar czf - ndarray" | tar xzvf - -C '+output_dir+'/'+ana_version+'/ndarray')
os.system('mv '+output_dir+'/'+ana_version+'/ndarray/ndarray '+output_dir+'/'+ana_version+'/ndarray/include')
os.system('ln -s '+output_dir+'/'+ana_version+'/ndarray/include '+output_dir+'/'+ana_version+'/include/ndarray')

