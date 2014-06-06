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
	img = alog10(h > 1)
	
	q = image(img, rgb_table=4)
	c = colorbar(target=q, orientation=1, position=[0.93,0.1,0.96,0.9], range=[min(img),max(img)], title='Log10(count)')


	t = contour(img, n_levels=50, /fill, rgb_table=4)
	c = colorbar(target=t, orientation=1, position=[0.93,0.1,0.96,0.9], range=[min(img),max(img)], title='Log10(count)')
	



end
