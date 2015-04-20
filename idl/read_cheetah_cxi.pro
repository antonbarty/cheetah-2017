


;; 
;; Function to read images from CXIDB file format
;;
function read_cheetah_cxi, filename, frameNum,  $
	field=field, $
	get_nframes=get_nframes, $
	get_dims=get_dims, $
	random=random, $
	verbose=verbose 
	

	if n_elements(frameNum) eq 0 then $
		frameNum = 0L
	if not keyword_set(field) then $
		field = 'entry_1/data_1/data'
		;;field = 'entry_1/instrument_1/detector_1/data'
	
	
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
 	if keyword_set(verbose) then begin
		print, 'ndims = ', data_ndims
		print, 'dims = ', data_dims
		print, 'npts = ', data_npts
		print, 'size = ', data_size
		print, 'simple = ', data_simple 	
		print,' '
	endif
	 	
 	
 	if data_ndims ne 3 then begin
 		print,'Error: Number of dimensions of data field unexpexted:'
 		help, data_ndims
 		stop
 	endif
 	nfs = data_dims[0]
 	nss = data_dims[1]
 	nframes = data_dims[2]
 	if keyword_set(verbose) then begin
		print, 'nframes = ', nframes
		print, 'nfs = ', nfs
		print, 'nss = ', nss 	
	endif 	

 	;; Option: Return number of frames
 	if keyword_set(get_nframes) then begin
		H5S_CLOSE, dataspace_id
		H5T_CLOSE, datatype_id
		H5D_CLOSE, dataset_id 
		H5F_CLOSE, file_id 
		return, nframes
 	endif 

 	;; Option: Return data dimensions
 	if keyword_set(get_dims) then begin
		H5S_CLOSE, dataspace_id
		H5T_CLOSE, datatype_id
		H5D_CLOSE, dataset_id 
		H5F_CLOSE, file_id 
		return, data_dims
 	endif 
 	
 	
 	;; Random frame from stack?
 	if keyword_set(random) then begin
 		frameNum = floor(nframes*randomu(seed))
 	end
 	
 	
 	;; Default action: read and return one single data frame
	slab_start = [0,0,frameNum]
	slab_count = [nfs,nss,1]
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



 	
	;; Code testing by looping through frames
	;; This bit should not normally be executed
	test = 0
	if(test) then begin
		nframes = data_npts[0]
		print,'Number of frames = ', nframes
		for i=0L, nframes-1 do begin
			msg = strcompress(string('Frame ', i, ' of ', nframes))
			print, msg
		
			;; Force a reset, just in case
			;H5S_SELECT_NONE, dataspace_id

			;; select_elements will work for a 1D array
			;coords = [i];
			;H5S_SELECT_ELEMENTS, dataspace_id, coords, /reset

			;; Alternative is to select a hyperslab
			slab_start = [0,0,i]
			slab_count = [nfs,nss,1]
			slab_stride = [nfs,nss,1]
			H5S_SELECT_HYPERSLAB, dataspace_id, slab_start, slab_count, /reset
			;H5S_SELECT_HYPERSLAB, dataspace_id, slab_start, slab_count, stride=[1], /reset
		
			;; Create a simple dataspace to hold the result. If we didn't supply
			;; the memory dataspace, then the result would be the same size
			;; as the image dataspace, with zeroes everywhere except our
			;; hyperslab selection.
			;; http://www.exelisvis.com/docs/HDF5_Overview.html
		
			memoryspace_id = H5S_CREATE_SIMPLE(slab_count)
			data = H5D_READ(dataset_id, file_space=dataspace_id, memory_space = memoryspace_id) 
			H5S_CLOSE, memoryspace_id
			help, data
		
			img = (data>(0.1))<1000
			img = alog10(img)
			if i eq 0 then $ 
				display, img $
			else $
				tvscl, img

			wait, 0.5
		
		endfor
		
		;; Close stream
		H5S_CLOSE, dataspace_id
		H5T_CLOSE, datatype_id
		H5D_CLOSE, dataset_id 
		H5F_CLOSE, file_id 
		return, data
	endif
	
 	
 	

end


