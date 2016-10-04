#
#   Crawler routines for CrystFEL indexing directory
#   Tested using Anaconda / Python 3.4
#

import sys
import os
import glob
import csv
import lib.cfel_filetools as cfel_file


def scan_crystfel(crystfel_dir):

    debug = False

    #printf, fout, '# Run, status, directory, processed, hits, hitrate%, mtime'

    run_out = ['r0001']
    status_out = ['---']
    directory_out = ['---']
    processed_out = ['---']
    indexed_out = ['---']
    indexrate_out = ['---']
    mtime_out = ['---']

    # Create sorted file list or files come in seemingly random order
    directories = glob.glob(crystfel_dir + '/r*')
    directories.sort()
    if debug:
        print(directories)

    for dir in directories:

        # Default values are blanks
        processed = '---'
        indexed = '---'
        indexrate = '---'
        mtime = '---'

        # Extract the directory name and run number
        directory = os.path.basename(dir)
        run = directory[:5]


        # Presence of directory means job has been submitted
        status = 'Submitted'

        # Job is running if bsub file exists
        if os.path.exists(dir+'/bsub.log'):
            status = 'Running'
        else:
            continue

        # "Exited with code" in bsub.log means terminated
        # "Final:" in bsub.log means job finished cleanly
        with open(dir+'/bsub.log') as f:
            data = f.read()
            if "Exited with exit code" in data:
                status = 'Terminated'
            if "Exited with exit code 143" in data:
                status = 'Killed'
            if "Final:" in data:
                status = 'Finished'

            position = data.rfind("%")
            indexrate = data[position-4:position+1]
            #print(position, ' : ', indexrate)



        # Append to main list
        run_out.append(run)
        directory_out.append(directory)
        status_out.append(status)
        processed_out.append(processed)
        indexed_out.append(indexed)
        indexrate_out.append(indexrate)
        mtime_out.append(mtime)
    #endfor


    # Create the result
    result = {
        'run' : run_out,
        'status' : status_out,
        'directory' : directory_out,
        'processed' : processed_out,
        'indexed' : indexed_out,
        'indexrate%': indexrate_out
    }



    # Write dict to CSV file
    keys_to_save = ['run','status','directory','processed','indexed','indexrate%']
    cfel_file.dict_to_csv('crystfel_status.csv', result, keys_to_save )

