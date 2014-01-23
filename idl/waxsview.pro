pro waxsview, directory, offset=offset

	nd = n_elements(directory)
	
	if nd eq 0 then begin
		print,'No directories were specified'
		return
	endif

	if not keyword_set(offset) then $
		offset = 1

	
	print,'Selected WAXS directories : '
	print, transpose(directory)



	ymax = max([2, nd*offset+1])


	;; Skeleton plot
	xmax = 1200
	y = fltarr(xmax)
	p = plot(y,/nodata, dimensions=[800,1000])
	
	p.yrange=[-0.05,ymax]
	p.xtitle = 'Radius (pixels)'
	p.ytitle = 'WAXS trace'
	p.title = 'Radial lineouts for different runs'

	
	;; Loop over the selected files
	for i=0, nd-1 do begin	
		y = waxs1d(directory[i], /find, /norm)
		
		yoffset = i*offset
		yy = y + yoffset
		ny = n_elements(yy)
		
		label = file_basename(directory[i])

		p1 = plot(yy, /over)
		t = text(xmax+10, yy[ny-1], label, /data, font_size=10) 		
		
	endfor



end