;; Geometry according to Garth
;;quad 0 (asics 0-3) 370x388 pixels
;; Reflect X
;; Rotate 90
;;quad 1 (asics 4-7) 388x370
;;reflect X
;;reflect Y					
;;quad 2 (asics 8-11) 370x388
;;rotate -90								
;;quad 3 (asics 12-15) 388x370
;;reflect X
;;reflect Y



pro rotate_module, x, y, theta

	if theta eq 0 then return

	tx = x*cos(!dtor*theta) - y*sin(!dtor*theta)
	ty = x*sin(!dtor*theta) + y*cos(!dtor*theta)
	
	x = tx
	y = ty

end


pro cspad_geometry

	;; Output filename
	filename = 'cspad_pixelmap.h5'
	
	
	;; Base module geometry
	ROWS = 194
	COLS = 185
	pixsize = 100e-6
	module_width = 2*max([ROWS,COLS])

	;; Create base module (consisting of two 2x1s with a 2-pixel split between them)
	mx = xarr(2*ROWS,2*COLS)-ROWS
	my = yarr(2*ROWS,2*COLS)-COLS
	my[*,0:COLS-1] -= 1
	my[*,COLS:2*COLS-1] += 1


	;; Create output arrays
	x = fltarr(8*ROWS,8*COLS)
	y = fltarr(8*ROWS,8*COLS)
	z = fltarr(8*ROWS,8*COLS)
	
	;;
	;;	Create 1st quadrant (4 modules)
	;;
	msx = [0.5, 0.5, 1.5, 1.5]		;; Module shift X (in multiples of module_width)
	msy = [0.5, 1.5, 0.5, 1.5]		;; Module shift Y (in multiples of module_width)
	mrx = [ 1,  1, 1, 1]			;; Coordinate reflection in X
	mry = [ 1,  1, 1, 1]			;; Coordinate reflection in Y
	mth = [ 0, 90, 90, 0]			;; Rotation (in degrees)
	
	for i=0, 3 do begin
		tmx = mrx[i]*mx
		tmy = mry[i]*my
		rotate_module, tmx, tmy, mth[i]
		tmx += msx[i]*module_width
		tmy += msy[i]*module_width
		x[0:2*ROWS-1,2*i*COLS:2*(i+1)*COLS-1] = tmx
		y[0:2*ROWS-1,2*i*COLS:2*(i+1)*COLS-1] = tmy
	endfor

	;;
	;;	Replicate this module across all 4 quadrants
	;;
	qth = [0,90,180,270]
	
	for i=0, 3 do begin
		tmx = x[0:2*ROWS-1, *]
		tmy = y[0:2*ROWS-1, *]
		rotate_module, tmx, tmy, qth[i]
		x[2*i*ROWS:2*(i+1)*ROWS-1,*] = tmx
		y[2*i*ROWS:2*(i+1)*ROWS-1,*] = tmy
	endfor
	

	;;
	;;	Adjust for pixel size
	;; 	(cspad_cryst assumes pixel locations are real physical displacements!)
	;;
	x *= pixsize
	y *= pixsize



	;;
	;; Save it
	;;
	fid = H5F_CREATE(filename) 

	datatype_id = H5T_IDL_CREATE(x) 
	dataspace_id = H5S_CREATE_SIMPLE(size(x,/DIMENSIONS)) 
	dataset_id = H5D_CREATE(fid,'x',datatype_id,dataspace_id) 
	H5D_WRITE,dataset_id,x 
	H5D_CLOSE,dataset_id   
	H5S_CLOSE,dataspace_id 
	H5T_CLOSE,datatype_id 

	datatype_id = H5T_IDL_CREATE(y) 
	dataspace_id = H5S_CREATE_SIMPLE(size(y,/DIMENSIONS)) 
	dataset_id = H5D_CREATE(fid,'y',datatype_id,dataspace_id) 
	H5D_WRITE,dataset_id,y
	H5D_CLOSE,dataset_id   
	H5S_CLOSE,dataspace_id 
	H5T_CLOSE,datatype_id 

	datatype_id = H5T_IDL_CREATE(z) 
	dataspace_id = H5S_CREATE_SIMPLE(size(z,/DIMENSIONS)) 
	dataset_id = H5D_CREATE(fid,'z',datatype_id,dataspace_id) 
	H5D_WRITE,dataset_id,z 
	H5D_CLOSE,dataset_id   
	H5S_CLOSE,dataspace_id 
	H5T_CLOSE,datatype_id 
	
	H5F_CLOSE,fid 


	;;
	;;	Pause at the end
	;;
	stop
end

