function histogram_clip, data, thresh_lo, thresh_hi

	temp = data

	if n_elements(data) eq 0 OR  n_elements(thresh_lo) eq 0 then begin
		print,'Usage: histogram_clip, data, threshold'
		print,'0 < threshold < 1'
		stop
	endif
	
	if n_elements(thresh_hi) eq 0 then $
		thresh_hi = thresh_lo


	;h = histogram(data, nbins=1e6, locations=x)
	h = histogram(data, locations=x)
	t = total(h, /cum)/total(h)
	
	wlo = max(where(t le  thresh_lo )) 
	whi = min(where(t ge 1-thresh_hi)) 
	wlo = wlo > 0
	whi = whi < (n_elements(t) -1)
	xlo = x[wlo]
	xhi = x[whi]

	temp = temp > xlo
	temp = temp < xhi
	
	return, temp

end