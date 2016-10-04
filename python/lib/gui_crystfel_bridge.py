import os
import glob
import shutil
import lib.cfel_filetools as cfel_file
import lib.gui_dialogs as gui_dialogs

#
#   Launch indexing
#   Handles a few special cases (adds some complexity but results in one function call for everything)
#
def index_runs(guiself, dirs=None, nocell=False, geopt=False):

    # Just some info
    print("Will process the following directories:")
    print(dirs)

    # Select geometry file and remember it
    geomfile = guiself.lastgeom
    if geomfile is None:
        geomfile = cfel_file.dialog_pickfile(path='../calib/geometry', filter='*.geom', qtmainwin=guiself)
        if geomfile is '':
            return
        guiself.lastgeom = geomfile

    # Default unit cell file
    cell=guiself.lastcell

    # This bit handles which recipe is selected
    # (including nocell and geopt options)
    recipe = guiself.lastindex
    if nocell:
        recipe = '../process/index_nocell.sh'
    if geopt:
        recipe = '../process/index_geopt.sh'



    # Launch dialog box for CrystFEL options
    dialog_in = {
        'pdb_files' : glob.glob('../calib/pdb/*.pdb')+glob.glob('../calib/pdb/*.cell'),
        'geom_files' : glob.glob('../calib/geometry/*.geom'),
        'recipe_files' : glob.glob('../process/index*.sh'),
        'default_geom' : geomfile,
        'default_cell' : cell,
        'default_recipe' : recipe
    }
    if dialog_in['default_cell'] is None or not cell in dialog_in['pdb_files']:
        dialog_in['default_cell'] = dialog_in['pdb_files'][0]

    dialog_out, ok = gui_dialogs.run_crystfel_dialog.dialog_box(dialog_in)

    # Exit if cancel was pressed
    if ok == False:
        return

    # Remember selections for later
    pdbfile = dialog_out['pdbfile']
    geomfile = dialog_out['geomfile']
    recipefile = dialog_out['recipefile']
    guiself.lastcell = pdbfile
    guiself.lastgeom = geomfile
    if not nocell and not geopt:
        guiself.lastindex = recipefile

    #geomfile = os.path.abspath(geomfile)


    #
    # Loop through selected directories
    # Much of this repeats what is in index-nolatt.... simplify later
    #
    for dirbase in dirs:
        # Data location and destination location
        print(dir)
        h5dir = "../hdf5/" + dirbase
        indexdir = "../indexing/" + dirbase

        # Remove contents of any existing directory then recreate directory
        shutil.rmtree(indexdir, ignore_errors=True)
        os.makedirs(indexdir, exist_ok=True)

        # Use find to create file list
        cmd = 'find ' + os.path.abspath(h5dir) + ' -name \*.cxi > ' + indexdir + '/files.lst'
        print(cmd)
        os.system(cmd)

        # Copy scripts and calibrations to target directory
        cmdarr = ['cp', recipefile , indexdir + '/.']
        cfel_file.spawn_subprocess(cmdarr, wait=True)
        cmdarr = ['cp', geomfile, indexdir + '/.']
        cfel_file.spawn_subprocess(cmdarr, wait=True)
        cmdarr = ['cp', pdbfile, indexdir + '/.']
        cfel_file.spawn_subprocess(cmdarr, wait=True)


        # Send indexing command to batch farm
        qlabel = 'indx-'+dirbase[1:5]
        logfile = 'bsub.log'
        abspath = os.path.abspath(indexdir)+'/'
        bsub_cmd = ['bsub', '-q', 'psanaq', '-x', '-J', qlabel, '-o', logfile, '-cwd', abspath, 'source', './'+os.path.basename(recipefile), dirbase, os.path.basename(pdbfile), os.path.basename(geomfile)]

        # Submit it
        cfel_file.spawn_subprocess(bsub_cmd)

    print(">------------------------------<")
    return


#
#   Merge stream files
#
def merge_streams(qtmainwin=None):

    # Files to merge
    stream_in = cfel_file.dialog_pickfile(path='../indexing/streams', filter='*.stream', multiple=True, qtmainwin=qtmainwin)
    if len(stream_in) is 0:
        return

    # Output stream name
    stream_out = cfel_file.dialog_pickfile(path='../indexing/streams', filter='*.stream', write=True, qtmainwin=qtmainwin)
    if stream_out is '':
        return

    # Remove destination (if it exists)
    try:
        os.remove(stream_out)
    except:
        pass

    # Concatenate stream files
    for stream in stream_in:
        cmd = 'cat ' + stream + ' >> ' + stream_out
        print(cmd)
        os.system(cmd)

    return

