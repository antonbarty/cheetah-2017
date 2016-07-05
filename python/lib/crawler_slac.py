#
#   Crawler routines for SLAC/XTC
#   Tested using Anaconda / Python 3.4
#

import sys
import os
import glob
import csv
import lib.cfel_filetools as cfel_file



def scan_data(data_dir):
    #print("Crawler data: ", data_dir)

    pattern = data_dir + '/*.xtc*'
    debug = False

    # Create sorted file list (glob seems to return files in random order)
    files = glob.glob(pattern)
    files.sort()

    if debug:
        print(files)

    # Extract the run bit from XTC file name
    out = []
    for filename in files:
        thisrun = filename.split('-')[1]
        #thisrun = thisrun[1:5]
        out.append(thisrun)

    #print('Number of XTC files: ', len(out))


    # Find unique run values (due to multiple XTC files per run)
    run_list = list(sorted(set(out)))
    nruns = len(run_list)
    #print('Number of unique runs: ', nruns)


    # Default status for each is ready
    status = ['Ready']*nruns

    # Loop through file names checking for '.inprogress' suffix
    for filename in files:
        if filename.endswith('.inprogress'):
            thisrun = filename.split('-')[1]
            #thisrun = thisrun[1:5]
            run_indx = run_list.index(thisrun)
            status[run_indx] = 'Copying'


    # Create the result
    result = {
        'run': run_list,
        'status' : status
    }

    if debug:
        print(result['run'])

    # Write dict to CSV file
    keys_to_save = ['run','status']
    cfel_file.dict_to_csv('data_status.csv', result, keys_to_save)

#end scan_data


    # Some notes pinched from earlier attempts at doing things

    #for filename in glob.iglob(pattern):


    # Split a string
    #>>> '1,2,3'.split(',')
    #['1', '2', '3']

    #
    #>> > ["foo", "bar", "baz"].index("bar")
    #1

    # Find unique values
    # l = ['r0001', 'r0002', 'r0002', 'r0003']
    # s = set(l)
    # s = sorted(s)
    # ll = list(s)


