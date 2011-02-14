;; Geometry according to Garth
;;
;;
;; Quad 0
;;   +---+ +---+ +---------+
;;   |10 | | 8 | | 12   13 |
;;   |   | |   | +---------+
;;   |11 | | 9 | +---------+
;;   |   | |   | | 14   15 |
;;   +---+ +---+ +---------+
;;   +---------+ +---+ +---+
;;   | 4    5  | |   | |   |
;;   +---------+ | 1 | | 3 |
;;   +---------+ |   | |   |
;;   | 6    7  | | 0 | | 2 |
;;   +---------+ +---+ +---+
;; 
;; Quad 1
;;   +---+ +---+ +---------+
;;   | 6 | | 4 | | 11   10 |
;;   |   | |   | +---------+
;;   | 7 | | 5 | +---------+
;;   |   | |   | |  9    8 |
;;   +---+ +---+ +---------+
;;   +---------+ +---+ +---+
;;   | 0    1  | |   | |   |
;;   +---------+ |14 | |12 |
;;   +---------+ |   | |   |
;;   | 2    3  | |15 | |13 |
;;   +---------+ +---+ +---+
;; 
;; Quad 2
;;   +---+ +---+ +---------+
;;   | 2 | | 0 | |  7   6  |
;;   |   | |   | +---------+
;;   | 3 | | 1 | +---------+
;;   |   | |   | |  5   4  |
;;   +---+ +---+ +---------+
;;   +---------+ +---+ +---+
;;   | 15  14  | |   | |   |
;;   +---------+ | 9 | |11 |
;;   +---------+ |   | |   |
;;   | 13  12  | | 8 | |10 |
;;   +---------+ +---+ +---+
;; 
;; Quad 3
;;   +---+ +---+ +---------+
;;   |13 | |15 | |  3   2  |
;;   |   | |   | +---------+
;;   |12 | |14 | +---------+
;;   |   | |   | |  1   0  |
;;   +---+ +---+ +---------+
;;   +---------+ +---+ +---+
;;   | 8    9  | |   | |   |
;;   +---------+ | 5 | | 7 |
;;   +---------+ |   | |   |
;;   | 10  11  | | 4 | | 6 |
;;   +---------+ +---+ +---+
;;
;;
;; Quadrant positions (as viewed from upstream)
;; 
;;     Q0     Q1
;; 
;;         x
;; 
;;     Q3     Q2
;;
;;
;;
;;	To create this pixel map we need to do the following:
;;
;;	1. Determine module 0 geometry
;;	Data appears in the XTC data as follows
;;         +---------+
;;         | 2    3  |
;;         +---------+
;;         +---------+
;;         | 0    1  |
;;         +---------+
;;	with (0,0) index in bottom left corner, stream of bytes form successive rows
;;	ie: x is most rapidly varying, y is least rapidly varying
;;
;;	We know module 0 should look like this
;;          +---+ +---+
;;          |   | |   |
;;          | 1 | | 3 |
;;          |   | |   |
;;          | 0 | | 2 |
;;          +---+ +---+
;;
;;	Giving cartesian axes for pixel coordinates (+x right, +y up) and giving
;;	each pixel in the data stream a cartesian (x,y) coordinate we need to:
;;		1. Reflect X
;;		2. Rotate 90 degrees CW
;;
;;	2. Module layout
;;		Module 0 = is in the lower right coner of the 1st quad (viewed from the beam)
;;		Module 1 = Module 0 rotated 90 CW
;;		Module 2 = Module 0 rotated 180 CW
;;		Module 3 = Module 0 rotated 90 CW
;;
;;	3. Quad layout:
;;		Quad 0 = is in the upper left (viewed from the beam)
;;		Quad 1 = Quad 0 rotated 90 CW
;;		Quad 2 = Quad 0 rotated 180 CW
;;		Quad 3 = Quad 0 rotated 270 CW
;;
;;	Pixel pitch is 110µm



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
	pixsize = 110e-6
	module_width = 2*max([ROWS,COLS])

	;; Create base module (consisting of two 2x1s with a 2-pixel split between them)
	mx = xarr(2*ROWS,2*COLS)-ROWS
	my = yarr(2*ROWS,2*COLS)-COLS
	;;my[*,0:COLS-1] -= 1
	;;my[*,COLS:2*COLS-1] += 1


	;; Create output arrays
	x = fltarr(8*ROWS,8*COLS)
	y = fltarr(8*ROWS,8*COLS)
	z = fltarr(8*ROWS,8*COLS)
	
	;;
	;;	Create 1st quadrant (4 modules)
	;;
	msx = [-0.5, -1.5, -1.5, -0.5]		;; Module shift X (in multiples of module_width)
	msy = [ 0.5,  0.5,  1.5,  1.5]		;; Module shift Y (in multiples of module_width)
	mrx = [ -1, -1, -1, -1]				;; Coordinate reflection in X
	mry = [  1,  1,  1,  1]				;; Coordinate reflection in Y
	mth = [-90, 180, 90, 180]			;; Rotation (in degrees)
	
	for i=0, 3 do begin
		print,'Placing module ',i
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
	qth = [0,-90,-180,-270]
	x0 = x[0:2*ROWS-1, *]
	y0 = y[0:2*ROWS-1, *]
	
	for i=0, 3 do begin
		print,'Replicating to quadrant ',i
		tmx = x0
		tmy = y0
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
	print,'Writing data to file: ',filename
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

