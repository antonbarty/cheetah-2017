function pixelremap, data, pix_x, pix_y, pix_dx, missing=missing, crop=crop


	;; Default pixel size
	if n_elements(pix_dx) eq 0 then $
		pix_dx = 1
	;; Missing data set to 0 unless otherwise specified
	if NOT KEYWORD_SET(missing) then $
		missing = 0
	


	;; Array bounds check
	if(n_elements(data) ne n_elements(pix_x) OR n_elements(data) ne n_elements(pix_y)) then begin
		print,'pixelremap: array sizes do not match'
		print, 'size(data) = ', size(data,/dim)
		print, 'size(pix_x) = ', size(pix_x,/dim)
		print, 'size(pix_y) = ', size(pix_y,/dim)
		print, 'Returning original data'
		return, data
	endif
	

	;; Correct for pixel size
	temp_x  = pix_x/pix_dx
	temp_y  = pix_y/pix_dx

	
	;; Size of remapped image
	max_x = max(abs(temp_x))
	max_y = max(abs(temp_y))
	nx = 2*max_x+2
	ny = 2*max_y+2

	;; Put image centre in centre
	temp_x += nx/2
	temp_y += ny/2


	;; Remap image (quick)
	image = fltarr(nx, ny)
	image[*] = missing
	image[temp_x, temp_y] = data
	
	
	;; Crop image...
	if keyword_set(crop) then begin
		pxlo = min(pix_x) + nx/2
		pxhi = max(pix_x) + nx/2
		pylo = min(pix_y) + ny/2
		pyhi = max(pix_y) + ny/2
		pxlo = max([pxlo,0])
		pxhi = min([pxhi,nx-1])
		pylo = max([pylo,0])
		pyhi = min([pyhi,ny-1])
		image = image[pxlo:pxhi,pylo:pyhi]
	endif
	
	return, image
end
