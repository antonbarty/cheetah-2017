#
#   Crawler routines for Cheetah HDF5 directory
#   Tested using Anaconda / Python 3.4
#

import sys
import os
import glob
import csv
import lib.cfel_filetools as cfel_file


def scan_hdf5(hdf5_dir):
    print("Crawler HDF5: ", hdf5_dir)

    debug = False
    pattern = hdf5_dir + '/r*/status.txt'

    #printf, fout, '# Run, status, directory, processed, hits, hitrate%, mtime'

    run_out = []
    status_out = []
    directory_out = []
    processed_out = []
    hits_out = []
    hitrate_out = []
    mtime_out = []

    # Create sorted file list or files come in seemingly random order
    files = glob.glob(pattern)
    files.sort()
    if debug:
        print(files)

    #for filename in glob.iglob(pattern):
    for filename in files:

        # Default values are blanks
        run = ''
        status = ''
        directory = ''
        processed = ''
        hits = ''
        hitrate = ''
        mtime = ''

        # Extract the Cheetah HDF5 directory name
        basename = os.path.basename(filename)
        dirname = os.path.dirname(filename)
        dirname2 = os.path.basename(dirname)
        directory = dirname2

        # Extract the run number (Warning: LCLS-specific)
        run = directory[:5]
        #run = directory[1:5]



        #print(filename)
        f = open(filename, 'r')
        for line in f:
            #print(line, end='')
            part = line.partition(':')

            if part[0] == 'Status':
                status = part[2].strip()

            if part[0] == 'Frames processed':
                processed = part[2].strip()

            if part[0] == 'Number of hits':
                hits= part[2].strip()
        #endfor
        f.close()


        # Calculate hit rate (with some error checking)
        if hits != '' and processed != '' and processed != '0':
            hitrate = 100 * ( float(hits) / float(processed))
        else:
            hitrate='---'

        # Diagnostic
        if debug:
            print("---------------")
            print("Run: ", run)
            print(directory)
            print(status)
            print(processed)
            print(hits)
            print(hitrate)

        # Append to main list
        run_out.append(run)
        directory_out.append(directory)
        status_out.append(status)
        processed_out.append(processed)
        hits_out.append(hits)
        hitrate_out.append(str(hitrate))
        mtime_out.append(mtime)
    #endfor


    # Create the result
    result = {
        'run' : run_out,
        'status' : status_out,
        'directory' : directory_out,
        'processed' : processed_out,
        'hits' : hits_out,
        'hitrate%': hitrate_out
    }



    # Sorting solved by sorting the file list
    # For future reference, to return indices of the sorted list
    # you can use the python sorting functions' key parameter to sort the index array instead.
    # >>> s = [2, 3, 1, 4, 5]
    # >>> sorted(range(len(s)), key=lambda k: s[k])
    # [2, 0, 1, 3, 4]
    # http://stackoverflow.com/questions/7851077/how-to-return-index-of-a-sorted-list


    # Write dict to CSV file
    keys_to_save = ['run','status','directory','processed','hits','hitrate%']
    cfel_file.dict_to_csv('cheetah_status.csv', result, keys_to_save )




