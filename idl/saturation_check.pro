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
	
	
	
	;; Read data (maximum 500,000)
	
	npeaks_max = 500000
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
	h = hist_2d(r,i, bin1=max(r)/100, bin2=max(i)/100, min1=0, max1=max(r), min2=0, max2=max(i))
	

	;; 2D image
	img = alog10(h > 1)
	q = image(img, position=[0.0, 0.05, 0.9, 0.95], rgb_table=26)
	c = colorbar(target=q, orientation=1, position=[0.93,0.1,0.96,0.9], range=[min(img),max(img)], title='Log10(count)')
	q.title =  file_basename(file_dirname(file))



	
	;; Projections of the histogram
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
	x = max(r) * findgen(n_elements(y))/n_elements(y)
	p2 = plot(	x, y, xrange=[0,1000])
	p2.xtitle = 'Radius on detector (pixels)'
	p2.ytitle='Percent of saturated peaks (I_max>90%)'
	p2.title =  file_basename(file_dirname(file))


	;; Percentage of peaks that are too weak
	tlow= total(h[*,0:0.15*sh[1]], 2)
	y = tlow/t2
	y *= 100.
	p3 = plot(	x, y, xrange=[0,1000])
	p3.xtitle = 'Radius on detector (pixels)'
	p3.ytitle='Percent of very weak peaks (I_max<15%)'
	p3.title =  file_basename(file_dirname(file))
		
	
	;; Plot of number of peaks at each intensity level
	y = t1 / max(t1)
	x = max(r) * findgen(n_elements(y))/n_elements(y)
	p1 = plot(	x, y, /ylog)
	p1.xtitle='Max ADC'
	p1.ytitle='Relative number of peaks with max(I)'
	p1.title =  file_basename(file_dirname(file))
	
	

	;; Save them
	print,'Saving plots...'
	label = file_basename(file_dirname(file))
	label = strmid(label, 0, 5)
	dir = file_dirname(file)
	p.save, dir + '/' + label + '-satScatter.png'
	p1.save, dir + '/' + label + '-satMax.png'
	p2.save, dir + '/' + label + '-satRatio.png'
	p3.save, dir + '/' + label + '-satLow.png'
	q.save, dir + '/' + label + '-sat2d.png'
	



end
