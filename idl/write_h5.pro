;;
;;	Basic IDL code to write a single data set to an HDF5 file
;;	Puts the contents of 'data' variable into the 'data' field of the HDF5 file
;;	Anton Barty, CFEL, 2009
;;
pro write_h5, filename, data, compress=compress, nocompress=nocompress

	if n_elements(filename) eq 0 then begin
		print,'write_simple_hdf5: No filename specified...'
		return
	endif
	
	if n_elements(data) eq 0 then begin
		print,'write_simple_hdf5: No data specified...'
		return
	endif
	
	if not keyword_set(compress) then begin
		compress=3
	endif
	
	if keyword_set(nocompress) then begin
		compress = 0
	endif
	
	dim = size(data,/dimensions)
	chunksize = dim
	;if n_elements(dim) ge 3 then $
	;	chunksize[0:n_elements(chunksize)-3] = 1
	
	fid = H5F_CREATE(filename) 
	group_id = H5G_CREATE(fid, 'data')
	datatype_id = H5T_IDL_CREATE(data) 
	dataspace_id = H5S_CREATE_SIMPLE(dim) 

	if keyword_set(nocompress) then begin
		dataset_id = H5D_CREATE(fid,'data/data',datatype_id,dataspace_id) 
	endif else begin
		dataset_id = H5D_CREATE(fid,'data/data',datatype_id,dataspace_id, chunk_dimensions=chunksize, gzip=compression, /shuffle) 
	endelse

	H5D_WRITE,dataset_id,data 
	H5D_CLOSE,dataset_id   
	H5S_CLOSE,dataspace_id 
	H5T_CLOSE,datatype_id 
	H5G_CLOSE,group_id
	H5F_CLOSE,fid 

end