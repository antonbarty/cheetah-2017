# -*- coding: utf-8 -*-
#
#	CFEL file handling tools
#	Anton Barty
#

import h5py


def read_h5(filename, field="/data/data"):
    """
    Read a simple HDF5 file
    """
   
    """
    To implement from IDL: dialog box for filename if not specified
	if n_elements(filename) eq 0 then $
		filename = dialog_pickfile()
	if filename[0] eq '' then $
		return, -1
  	if NOT KEYWORD_SET(field) then $
		field = '/data/data'
    """

    # Open HDF5 file
    fp = h5py.File(filename, 'r')     
    
    # Read the specified field
    data = fp[field][:]
    
    # Close and clean up
    fp.close()    

    # return
    return data
#end read_h5
    
  
  
    
def write_h5(filename, field="data/data", compress=3):
    """ 
    Write a simple HDF5 file
    """
    
    """ 
    IDL code
    	if n_elements(filename) eq 0 then begin
		print,'write_simple_hdf5: No filename specified...'
		return
	endif
	
	if n_elements(data) eq 0 then begin
		print,'write_simple_hdf5: No data specified...'
		return
	endif
	dim = size(data,/dimensions)
	
	
	if not keyword_set(compress) then begin
		compress=3
	endif
	

	;; HDF5 compression chunks
	chunksize = dim
	if n_elements(dim) ge 3 then $
	;;	chunksize[0] = dim[0]/8;
	;;	chunksize[0:n_elements(chunksize)-3] = 1

	
	fid = H5F_CREATE(filename) 
	group_id = H5G_CREATE(fid, 'data')
	datatype_id = H5T_IDL_CREATE(data) 
	dataspace_id = H5S_CREATE_SIMPLE(dim) 

	if (compress eq 0) then begin
		dataset_id = H5D_CREATE(fid,'data/data',datatype_id,dataspace_id) 
	endif else begin
		;; GZIP keyword is ignored if CHUNK_DIMENSIONS is not specified.
		dataset_id = H5D_CREATE(fid,'data/data',datatype_id,dataspace_id, gzip=compression,  chunk_dimensions=chunksize) 
		;dataset_id = H5D_CREATE(fid,'data/data',datatype_id,dataspace_id, chunk_dimensions=chunksize, gzip=compression, /shuffle) 
	endelse

	H5D_WRITE,dataset_id,data 
	H5D_CLOSE,dataset_id   
	H5S_CLOSE,dataspace_id 
	H5T_CLOSE,datatype_id 
	H5G_CLOSE,group_id
	H5F_CLOSE,fid 
     """
#end write_h5

     
def read_cxi(filename, frameID=0, mask=False, peaks=False, photon_energy=False, camera_length=False, slab_size=False):
    """ 
    Read a frame from multi-event CXI file
    Also read mask and peak lists if requested
    There is probably a smarter way to read the requested stuff at once and return it all at the same time from the same file handle (a job for later)
    """
    
    hdf5_fh = h5py.File(filename, 'r')      
    
    # Return peak list
    if peaks == True:
        n_peaks = hdf5_fh['/entry_1/result_1/nPeaks'][frameID]            
        peak_x_data = hdf5_fh['/entry_1/result_1/peakXPosRaw'][frameID]
        peak_y_data = hdf5_fh['/entry_1/result_1/peakYPosRaw'][frameID]
        peak_xy = (peak_x_data.flatten(), peak_y_data.flatten())
        hdf5_fh.close()
        return n_peaks, peak_xy


    # Return masks
    if mask == True:
        mask = hdf5_fh['/entry_1/data_1/mask'][frameID, :, :]
        hdf5_fh.close()
        return mask
    
    # Return photon energy    
    if photon_energy == True: 
        photon_energy = hdf5_fh['/LCLS/photon_energy_eV'][frameID]
        hdf5_fh.close()
        return photon_energy
        

    # Return camera length
    if camera_length == True: 
        camera_length = hdf5_fh['/LCLS/detector_1/EncoderValue'][frameID]
        hdf5_fh.close()
        return camera_length

    # Return the number of events and slab size
    # As the file is being written the slab size can be greater than the number of saved images (rest are blank)
    # Cross-check against photon energy values to get the real number of saved images (to avoid displaying blanks at the end)
    # In IDL we do this:
    #			if(nframes gt 0) then begin
    #				check = read_h5(cxifile[i], field = 'entry_1/instrument_1/detector_1/x_pixel_size')
    #				;;help, check
    #				w = where(check ne 0)
    #				if w[0] ne -1 then ncheck=n_elements(w) else ncheck=0
    #				if nframes gt ncheck then $
    #					nframes = ncheck
    #			endif
    if slab_size == True:
        size = hdf5_fh['/entry_1/data_1/data'].shape
        hdf5_fh.close()
        return size
        


    # Default is to return image data
    #data = hdf5_fh['/entry_1/instrument_1/detector_1/detector_corrected/data'][frameID, :, :]
    data = hdf5_fh['/entry_1/data_1/data'][frameID, :, :]
    hdf5_fh.close()
    return data
#end read_cxi


def find_cheetah_images(dir):
    """
    function find_cheetah_images, dir
    
    	file = ['']
    	
    	;; Find .cxi files
    	cxifile = file_search(dir,"*.cxi",/fully_qualify)
    	if n_elements(cxifile) ne 0 AND cxifile[0] ne '' then begin
    		file_type = 'cxi'
    		print,strcompress(string('CXI files: ', file_basename(cxifile)))
    		total_nframes = 0
    		index = 0
    
    		print, cxifile
    		
    		for i=0, n_elements(cxifile)-1 do begin
    			nframes = read_cheetah_cxi(cxifile[i], /get_nframes)
    
    			;; Cross-check against number of non-zero elements in pixel size array
    			;; (this is to avoid the blank frames problem when the file is not completed)
    			if(nframes gt 0) then begin
    				check = read_h5(cxifile[i], field = 'entry_1/instrument_1/detector_1/x_pixel_size')
    				;;help, check
    			
    				w = where(check ne 0)
    				if w[0] ne -1 then ncheck=n_elements(w) else ncheck=0
    				if nframes gt ncheck then $
    					nframes = ncheck
    			endif
    				
    			print, strcompress(string('Number of frames in ', file_basename(cxifile[i]), ' = ', nframes))
    
    			if total_nframes eq 0 AND nframes gt 0 then begin
    				file = replicate(cxifile[i], nframes)
    				index = indgen(nframes)
    				total_nframes += nframes
    			endif $
    			else if nframes ne 0 then begin
    				file = [file, replicate(cxifile[i], nframes)]
    				index = [index, indgen(nframes)]
    				total_nframes += nframes
    			endif
    		endfor
    	endif $
    
    	;; Find single frane .h5 files
    	else begin
    		file = file_search(dir,"LCLS*.h5",/fully_qualify)	
    	 	if n_elements(file) eq 0 OR file[0] eq '' then begin
    		 	message,'No files found in directory: '+dir, /info
    	 	endif
    		file_type = 'little_h5'
    		index = intarr(n_elements(file))
    		print,strcompress(string(n_elements(file),' files found'))
    	endelse
    
    	;; Return result and indexes
    	result = {	file_type : file_type, $
    					file : file, $
    					index : index $
    				}
    
    	;; Debugging				
    	;;print, result.file
    	;;print, result.index
    	
    	return, result
    end
    """
#end find_cheetah_images
    