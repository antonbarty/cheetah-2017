


;; 
;; 	Function to read event data from CXIDB file format
;;	Work on the presumption that the last dimension is the frame number
;;
function read_cxi_data, filename, frameNum,  field=field
	
	;; For debugging
	verbose = 0

	if n_elements(filename) eq 0 then begin
		filename = dialog_pickfile()
		if filename[0] eq '' then return,-1
	endif

	if n_elements(frameNum) eq 0 then $
		frameNum = 0L
	if not keyword_set(field) then $
		field = 'entry_1/instrument_1/detector_1/data'
	
	
	if keyword_set(verbose) then begin
		print,'read_cxi(...)'
		print, 'filename=',filename
		print, 'field=',field
	endif
	
	;; Open requested data set
	file_id = H5F_OPEN(filename) 
 	dataset_id = H5D_OPEN(file_id, field) 
 	dataspace_id = H5D_GET_SPACE(dataset_id)
 	datatype_id = H5D_GET_TYPE(dataset_id)
 	
 	
 	;; Read data dimensions, etc.
 	data_ndims = H5S_GET_SIMPLE_EXTENT_NDIMS(dataspace_id)
 	data_dims = H5S_GET_SIMPLE_EXTENT_DIMS(dataspace_id)
 	data_npts = H5S_GET_SIMPLE_EXTENT_DIMS(dataspace_id)
 	data_simple = H5S_IS_SIMPLE(dataspace_id)
 	data_size = H5D_GET_STORAGE_SIZE(dataset_id)
 	index_dim = data_ndims-1

 	if keyword_set(verbose) then begin
		print, 'ndims = ', data_ndims
		print, 'dims = ', data_dims
		print, 'npts = ', data_npts
		print, 'size = ', data_size
		print, 'simple = ', data_simple 	
		print, 'index_dim = ', index_dim		
		print,' '
	endif
	 	
 	
 	if data_ndims lt 2 then begin
 		print,'Error: Number of dimensions of data < 2'
 		help, data_ndims
		print, 'ndims = ', data_ndims
		print, 'dims = ', data_dims
 		stop
 	endif
 	
 	
; 	nfs = data_dims[0]
; 	nss = data_dims[1]
;	nframes = data_dims[2]
; 	if keyword_set(verbose) then begin
;		print, 'nframes = ', nframes
;		print, 'nfs = ', nfs
;		print, 'nss = ', nss 	
;	endif 	

 	
 	;; Read and return list
	;slab_start = [0,0,frameNum]
	;slab_count = [nfs,nss,1]
 	slab_start = intarr(data_ndims)
 	slab_start[index_dim] = frameNum
 	slab_count = data_dims
 	slab_count[index_dim] = 1
	memoryspace_id = H5S_CREATE_SIMPLE(slab_count)
	
	H5S_SELECT_HYPERSLAB, dataspace_id, slab_start, slab_count, /reset
	data = H5D_READ(dataset_id, file_space=dataspace_id, memory_space = memoryspace_id) 
	H5S_CLOSE, memoryspace_id

 	if keyword_set(verbose) then begin
		help, data
	endif

	H5S_CLOSE, dataspace_id
	H5T_CLOSE, datatype_id
	H5D_CLOSE, dataset_id 
	H5F_CLOSE, file_id 

	return, data


end


