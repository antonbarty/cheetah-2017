function badpix_from_powder, filename, value=value, save=save, edgemask=edgemask, menu=menu

	if n_elements(filename) eq 0 then begin
		filename = dialog_pickfile(/must_exist)
		if filename eq '' then return, -1
	endif

	if NOT KEYWORD_SET(value) then $
		value = 0.1


	if KEYWORD_SET(menu) then begin
		desc = [ 	'1, base, , column', $
					'2, float, '+string(value)+', label_left=Value: , tag=value', $
					'1, base,, row', $
					'0, button, OK, Quit, Tag=OK', $
					'2, button, Cancel, Quit' $
		]		

		a = cw_form(desc, /column, title='Bad pixels from powder')

		;; Only do this if OK is pressed (!!)
		if a.OK eq 1 then begin		
			value = a.value
		endif 
	endif




	;; Read in data
	print,'Source file: ', filename
	a = read_h5(filename, field='data/non_assembled_detector_corrected')
	n = read_h5(filename, field='data/nframes')	
	

	;; ASIC size (cspad specific)
	s = size(a, /dim)
	ss = s / 8
	
	
	ref = smooth(a,5)
	
	maskv = a lt value*ref
	
	
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
	mask = maskv	OR maske

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