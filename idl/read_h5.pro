;;
;;	Basic IDL code to read in our simple HDF5 file format
;;	Returns the content of the 'data' field in a given HDF5 file
;;	Anton Barty, CFEL, 2009
;;

function read_h5, filename, field=field

	if n_elements(filename) eq 0 then $
		filename = dialog_pickfile()
	if filename[0] eq '' then $
		return, -1
		
	if NOT KEYWORD_SET(field) then $
		field = '/data/data'

	file_id = H5F_OPEN(filename) 
 	dataset_id = H5D_OPEN(file_id, field) 
	data = H5D_READ(dataset_id) 
	H5D_CLOSE, dataset_id 
	H5F_CLOSE, file_id 

	return, data
END 