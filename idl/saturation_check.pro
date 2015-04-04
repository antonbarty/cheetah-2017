pro saturation_check, file

	if n_elements(file) eq 0 then begin
		file = dialog_pickfile()
		if file eq '' then return
	endif

print, file
	;; File existence check
	if file_test(file) eq 0 then begin
		print,'Error: File does not exist'
		print, file
		return
	endif

	print, 'Reading peak list: ', file
	print, '(This can take a while if the peak list is long)'
	
	
	
	;; Read data (maximum 1e6)
	
	npeaks_max = 1000000
	a = read_csv(file, num_records=npeaks_max, header=h)
	print, 'File read... plotting'
	
	
	
	;; Extract fields
	r =  a.field09
	i = a.field14
	npeaks = n_elements(r)
	print,'Number of peaks: ', npeaks
	
	
	
	;; Simple scatter plot
	p = plot(r, i, linestyle=' ', symbol='.')
	p.xtitle = 'Radius on detector (pixels)'
	p.title =  file_basename(file_dirname(file))

	

	;; 2D histogram
	h = hist_2d(r,i, bin1=max(r)/100, bin2=max(i)/100)
	

	;; 2D image
	img = alog10(h > 1)
	q = image(img, position=[0.0, 0.05, 0.9, 0.95], rgb_table=26)
	c = colorbar(target=q, orientation=1, position=[0.93,0.1,0.96,0.9], range=[min(img),max(img)], title='Log10(count)')



	
	;; Percentages at each radius 
	sh = size(h, /dim)
	ph = h
	ph[*] = 0

	t = total(h)
	t1 = total(h,1)
	t2 = total(h,2)
	

	;; Percentage of peaks that are saturated
	tsat = total(h[*,0.9*sh[1]:*], 2)
	y = tsat/t2
	y *= 100.
	x = min(r) + findgen(n_elements(y)) * ((max(r)-min(r))/n_elements(y))
	p2 = plot(	x, y, xtitle='Radius on detector (pixels)', ytitle='% of  peaks >90th percentile', title='Proportion of saturated peaks')
		
	
	;; Plot of number of peaks at each intensity level
	y = t1 / max(t1)
	x = min(i) + findgen(n_elements(y)) * ((max(i)-min(i))/n_elements(y))
	p1 = plot(	x, y, /ylog, xtitle='Max ADC', ytitle='Number of peaks', title='Maximum peak intensity distribution')
	
	

	;;t = contour(img, n_levels=50, /fill,  background_color=[0,0,0], rgb_table=39)
	;;c = colorbar(target=t, orientation=1, position=[0.93,0.1,0.96,0.9], range=[min(img),max(img)], title='Log10(count)')
	



end
