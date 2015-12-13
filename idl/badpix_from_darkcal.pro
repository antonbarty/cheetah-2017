function badpix_from_darkcal, filename, noise=noise, value=value, save=save, edgemask=edgemask, menu=menu

	if n_elements(filename) eq 0 then begin
		filename = dialog_pickfile(/must_exist)
		if filename eq '' then return, -1
	endif

	if NOT KEYWORD_SET(noise) then $
		noise = 5
	if NOT KEYWORD_SET(value) then $
		value = 5


	if KEYWORD_SET(menu) then begin
		desc = [ 	'1, base, , column', $
					'0, label, Identifies bad pixels based on strange behaviour when not illuminated, left', $
					'0, label, ie: pixels in a darkcal which are either noisy or, left', $
					'0, label, have a significantly different offset to other pixels on the same ASIC, left', $
					'0, label, mask_noise = sigma_pix gt noise, left', $
					'0, label, mask_offset = dark gt (median(ASIC)+value*stddev(ASIC)), left', $
					'0, label, mask = mask_noise OR mask_offset, left', $
					'0, float, '+string(noise)+', label_left=Noise: , tag=noise', $
					'2, float, '+string(value)+', label_left=Value: , tag=value', $
					'1, base,, row', $
					'0, button, OK, Quit, Tag=OK', $
					'2, button, Cancel, Quit' $
		]		

		a = cw_form(desc, /column, title='Start cheetah')

		;; Only do this if OK is pressed (!!)
		if a.OK eq 1 then begin		
			noise = a.noise
			value = a.value
		endif 
	endif




	;; Read in data
	print,'Source file: ', filename
	;a = read_h5(filename, field='data/correcteddata')
	;b = read_h5(filename, field='data/correcteddatasigma')
	a = read_h5(filename, field='data/non_assembled_raw')
	b = read_h5(filename, field='data/non_assembled_raw_sigma')
	n = read_h5(filename, field='data/nframes')	
	a /= n[0]
	

	;; ASIC size (cspad specific)
	s = size(a, /dim)
	ss = s / 8
	
	
	
	;; Noisy pixel mask
	;; (pixels with higher standard deviation than some threshold)
	maskn = b gt noise
	
	
	
	;; Pixels with really different dark values in each ASIC
	maskv = bytarr(s[0],s[1])
	for j=0, 7 do begin
		for i=0, 7 do begin
			data = a[i*ss[0]:(i+1)*ss[0]-1,  j*ss[1]:(j+1)*ss[1]-1] 
			
			m = median(data)
			dev = stddev(data)
			
			dmask = data gt (m+value*dev)
			
			maskv[i*ss[0]:(i+1)*ss[0]-1,  j*ss[1]:(j+1)*ss[1]-1]  = dmask			
		endfor
	endfor
	
	
	;; Edges of the ASICs
	maske = bytarr(s[0],s[1])
	if KEYWORD_SET(edgemask) then begin
		for i=0, 7 do begin
			maske[i*ss[0], *] = 1		
			maske[(i+1)*ss[0]-1, *] = 1		
			maske[*, i*ss[1]] = 1		
			maske[*, (i+1)*ss[1]-1] = 1		
		endfor
	endif	
	
	
	;; Compile final mask
	mask = maskn OR maskv	OR maske

	;; Mask is actually the inverse (1=good, 0=bad)
	mask = 1-mask
	
	;; Display it
	loadct, 0
	scrolldisplay, mask, title='Bad pixel mask'

	;; Save it
	if keyword_set(save) then begin
		fout = dialog_pickfile(title='Save file')
		if fout ne '' then $
			write_h5, fout, mask
	endif


	return, mask
end