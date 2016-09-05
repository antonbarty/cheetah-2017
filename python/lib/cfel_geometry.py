#
#   CrystFEL Geometry parser
#   Mostly due to Valerio Mariani
#
#   Tested using Anaconda / Python 3.4
#

import h5py
import numpy
import sys
import traceback
import re

def pixelmap_from_CrystFEL_geometry_file(fnam):
    """
    Return pixel and radius maps from CrystFEL format geometry file
    
    Input: geometry filename
    
    Output: x: slab-like pixel map with x coordinate of each slab pixel in 
                the reference system of the detector
            y: slab-like pixel map with y coordinate of each slab pixel in 
                the reference system of the detector
            z: slab-like pixel map with distance of each pixel from the center
                of the reference system.  
    """

    f = open(fnam, 'r')
    f_lines = []
    for line in f:
        line_parsed_comments = line.split(";", 1)[0] 
        if(line_parsed_comments is not ""):
            f_lines.append(line_parsed_comments)

    keyword_list = ['min_fs', 'min_ss', 'max_fs', 'max_ss', 'fs', 'ss', 'corner_x', 'corner_y']

    detector_dict = {}

    panel_lines = [ x for x in f_lines if '/' in x and len(x.split('/')) == 2 and x.split('/')[1].split('=')[0].strip() in keyword_list ]

    for pline in panel_lines:
        items = pline.split('=')[0].split('/')
        panel = items[0].strip()
        property = items[1].strip()
        if property in keyword_list:
            if panel not in detector_dict.keys():
                detector_dict[panel] = {}
            detector_dict[panel][property] = pline.split('=')[1].split(';')[0]
        #endif
    #endfor


    parsed_detector_dict = {}

    for p in detector_dict.keys():

        parsed_detector_dict[p] = {}

        parsed_detector_dict[p]['min_fs'] = int( detector_dict[p]['min_fs'] )
        parsed_detector_dict[p]['max_fs'] = int( detector_dict[p]['max_fs'] )
        parsed_detector_dict[p]['min_ss'] = int( detector_dict[p]['min_ss'] )
        parsed_detector_dict[p]['max_ss'] = int( detector_dict[p]['max_ss'] )
        parsed_detector_dict[p]['fs'] = []
        parsed_detector_dict[p]['fs'].append( float( detector_dict[p]['fs'].split('x')[0].replace(" ", "") ) )
        parsed_detector_dict[p]['fs'].append( float( detector_dict[p]['fs'].split('x')[1].split('y')[0].replace(" ", "") ) )
        parsed_detector_dict[p]['ss'] = []
        parsed_detector_dict[p]['ss'].append( float( detector_dict[p]['ss'].split('x')[0].replace(" ", "") ) )
        parsed_detector_dict[p]['ss'].append( float( detector_dict[p]['ss'].split('x')[1].split('y')[0].replace(" ", "") ) )
        parsed_detector_dict[p]['corner_x'] = float( detector_dict[p]['corner_x'] )
        parsed_detector_dict[p]['corner_y'] = float( detector_dict[p]['corner_y'] )
    #endfor



    max_slab_fs = numpy.array([parsed_detector_dict[k]['max_fs'] for k in parsed_detector_dict.keys()]).max()
    max_slab_ss = numpy.array([parsed_detector_dict[k]['max_ss'] for k in parsed_detector_dict.keys()]).max()


    x = numpy.zeros((max_slab_ss+1, max_slab_fs+1), dtype=numpy.float32)
    y = numpy.zeros((max_slab_ss+1, max_slab_fs+1), dtype=numpy.float32)

    for p in parsed_detector_dict.keys():
        # get the pixel coords for this asic
        i, j = numpy.meshgrid( numpy.arange(parsed_detector_dict[p]['max_ss'] - parsed_detector_dict[p]['min_ss'] + 1),
                               numpy.arange(parsed_detector_dict[p]['max_fs'] - parsed_detector_dict[p]['min_fs'] + 1), indexing='ij')

        #
        # make the y-x ( ss, fs ) vectors, using complex notation
        dx  = parsed_detector_dict[p]['fs'][1] + 1J * parsed_detector_dict[p]['fs'][0]
        dy  = parsed_detector_dict[p]['ss'][1] + 1J * parsed_detector_dict[p]['ss'][0]
        r_0 = parsed_detector_dict[p]['corner_y'] + 1J * parsed_detector_dict[p]['corner_x']
        #
        r   = i * dy + j * dx + r_0
        #
        y[parsed_detector_dict[p]['min_ss']: parsed_detector_dict[p]['max_ss'] + 1, parsed_detector_dict[p]['min_fs']: parsed_detector_dict[p]['max_fs'] + 1] = r.real
        x[parsed_detector_dict[p]['min_ss']: parsed_detector_dict[p]['max_ss'] + 1, parsed_detector_dict[p]['min_fs']: parsed_detector_dict[p]['max_fs'] + 1] = r.imag
    #endfor
            
    # Calculate radius
    r = numpy.sqrt(numpy.square(x) + numpy.square(y))

    return x, y, r


def coffset_from_CrystFEL_geometry_file(fnam):
    """
    Read coffset from CrystFEL geometry file res = The resolution (in pixels
    per metre) for this panel. This is one over the pixel size in metres.
    :param fnam: :return: 
    """

    f = open(fnam, 'r')
    f_lines = []
    for line in f:
        # handle comments
        line_parsed_comments = line.split(";", 1)[0] 
        if(line_parsed_comments is not ""):
            f_lines.append(line_parsed_comments)
    # endfor
    f.close()

    coffset_lines = [x for x in f_lines if "coffset" in x]
    
    # There might be no coffset in the geometry file. We have to check for that.
    if coffset_lines:
        coffset = float(coffset_lines[-1].split('=')[1])
    else:
        coffset = numpy.nan

    res_lines = [x for x in f_lines if "res" in x]
    # There might be no res in the geometry file. We have to check for that.
    if res_lines:
        res = float(res_lines[-1].split('=')[1])
        dx_m = 1.0/res
    else:
        res = numpy.nan
        dx_m = numpy.nan

    return coffset, res, dx_m


def clen_from_CrystFEL_geometry_file(fnam):
    """
    This fuction returns the clen from the geometry file.

    Returns:
        string: codeword under which the clen can be found in the streamfile
        float: clen value
        None: clen not found in the geometry file
    
    Note:
        This functions opens the geometry file all alone and scans the whole
        file for the clen flag. This might take long and is so far from being
        optimal. This implementation has been choosen such that the original
        geometry parser from cheetah remains unchanged.
    """

    try:
        with open(fnam, 'r') as f:
            clen = None
            for line in f:
                line_parsed_comments = line.split(";", 1)[0] 
                if "clen = " in line_parsed_comments:
                    clen = line.replace("clen = ", "").rstrip()
                    break
            if clen is None:
                return clen
            try:
                clen_float = float(clen)
                return clen_float
            except ValueError:
                return clen

            #float_matching_pattern = r"""
            #    [-+]? # optional sign
            #    (?:
            #    (?: \d* \. \d+ ) # .1 .12 .123 etc 9.1 etc 98.1 etc
            #    |
            #    (?: \d+ \.? ) # 1. 12. 123. etc 1 12 123 etc
            #    )
            #    # followed by optional exponent part if desired
            #    (?: [Ee] [+-]? \d+ ) ?
            #"""
            #float_matching_regex = re.compile(
            #    float_matching_pattern, re.VERBOSE)

            #match = re.findall(float_matching_regex, clen)
            #if match:
            #    return float(match[0])
            #else:
            #    return clen
    except IOError:
        return None

def read_pixelmap(filename):
    """
    Read Cheetah style pixelmap
    (HDF5 file with fields "x", "y" and "z" containing pixel coordinates in meters)
    """

    # Open HDF5 pixelmap file
    try:
        with h5py.File(filename, 'r') as fp:
            x = fp['x'][:]
            y = fp['y'][:]
            fp.close()
    except:
        print('Error reading pixelmap:')
        print(filename)


    # Correct for pixel size (meters --> pixels)
    # Currently hard coded for CSPAD
    # We can figure this out from the pixel map
    dx = 110e-6
    x /= dx
    y /= dx
    

    # Calculate radius
    r = numpy.sqrt(numpy.square(x) + numpy.square(y))

    return x, y, r, dx
    


def read_geometry(geometry_filename, quiet=False):
    """
    Read geometry files and return pixel map Determines file type and calls the
    appropriate routine for reading the geometry Note transposition and change
    of axes so that images appear the same orientation in hdfsee, cheetah/IDL
    and pyQtGraph

    Output is the following sttructure.  Return unit for geometry is pixels
    (CrystFEL unit 'res = 1/pix_size' is depreciated)

    result_dict = {
        'x' : x.flatten(),      # In pixels
        'y' : y.flatten(),      # In pixels
        'r' : r.flatten(),      # In pixels
        'dx' : dx_m,
        'coffset' : coffset,
        'shape' : img_shape
    }
    """

    # determine file format
    if geometry_filename.endswith(".geom"):
        format = 'CrystFEL'
    elif geometry_filename.endswith(".h5"):
        format = 'pixelmap'
    else:
        print("Unknown geometry file format: ", geometry_filename)
        format = 'unknown'


    # Read geometry, depending on format
    fail = False
    if format == 'CrystFEL':
        try:
            x, y, r = pixelmap_from_CrystFEL_geometry_file(geometry_filename)
            coffset, res, dx_m = coffset_from_CrystFEL_geometry_file(geometry_filename)
        except:
            fail = True
            traceback.print_exc()

    elif format == 'pixelmap':
        try:
            x, y, r, dx_m = read_pixelmap(geometry_filename)
            coffset = numpy.nan
        except:
            fail = True

    else:
        print('Unsupported geometry type:', geometry_filename)
        fail = True
    #endif



    if fail:
        print('Error reading geometry file:')
        print(geometry_filename)
        print('Fatal error.  Quitting.')
        exit()



    # find the smallest size of cspad_geom that contains all
    # xy values but is symmetric about the origin
    M = 2 * int(max(abs(x.max()), abs(x.min()))) + 2
    N = 2 * int(max(abs(y.max()), abs(y.min()))) + 2


    # convert x y values to i j values
    # Minus sign for y-axis because Python takes (0,0) in top left corner instead of bottom left corner
    #
    # Note to Valerio: 
    # Do not add the offset to convert to image coordinates staring at (0,0) as we may want the actual pixel coordinates
    # This means do not center array here --> it is done in pixel_remap instead
    # Returning actual coordinates (x,y) is better for other operations such as radial averages
    x = x
    y = -y
    img_shape = (M, N)

    # Print a sanity check unless suppressed
    if not quiet:
        print('----------')
        print('Geometry info:')
        print('X range (pix): ', x.min(), x.max())
        print('Y range (pix): ', y.min(), y.max())
        print('R range (pix): ', r.min(), r.max())
        print('Pixel size (m): %4.6f' % (dx_m))
        print("Geometry shape: ", x.shape)
        print("Geometry elements: ", x.flatten().shape)
        print("Assembled image size: ", img_shape)

    # Return dict
    result_dict = {
        'x' : x.flatten(),
        'y' : y.flatten(),
        'r' : r.flatten(),
        'dx' : dx_m,
        'coffset' : coffset,
        'shape' : img_shape
    }

    return result_dict
    #end read_geometry()


