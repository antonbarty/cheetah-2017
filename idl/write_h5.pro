;;
;;	Basic IDL code to write a single data set to an HDF5 file
;;	Puts the contents of 'data' variable into the 'data' field of the HDF5 file
;;	Anton Barty, CFEL, 2009
;;
pro write_h5, filename, data

	if n_elements(filename) eq 0 then begin
		print,'write_simple_hdf5: No filename specified...'
		return
	endif
	
	if n_elements(data) eq 0 then begin
		print,'write_simple_hdf5: No data specified...'
		return
	endif
	
	
	fid = H5F_CREATE(filename) 
	group_id = H5G_CREATE(fid, 'data')
	datatype_id = H5T_IDL_CREATE(data) 
	dataspace_id = H5S_CREATE_SIMPLE(size(data,/DIMENSIONS)) 
	dataset_id = H5D_CREATE(fid,'data/data',datatype_id,dataspace_id) 
	H5D_WRITE,dataset_id,data 
	H5D_CLOSE,dataset_id   
	H5S_CLOSE,dataspace_id 
	H5T_CLOSE,datatype_id 
	H5G_CLOSE,group_id
	H5F_CLOSE,fid 

end