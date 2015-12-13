;;
;;	Unbonded pixels behave exactly like normal pixels in dark frames
;;  We only see that they are dark when the whole detector is illuminated.
;;	Attempt to find unbonded pixels by identifying pixels which remain at 'dark' tolerances even when the whole detector is illuminated.
;;

function badpix_from_powder, filename, tolerance=tolerance, save=save, edgemask=edgemask, menu=menu

	if n_elements(filename) eq 0 then begin
		filename = dialog_pickfile(/must_exist)
		if filename eq '' then return, -1
	endif

	if NOT KEYWORD_SET(tolerance) then $
		tolerance = 10


	if KEYWORD_SET(menu) then begin
		desc = [ 	'1, base, , column', $
					'0, label, Identifies unbonded and/or unresponsive pixels, left', $
					'0, label, ie: pixels which do not change value when illuminated, left', $
					'0, label, These pixels have values of 0 +/- tolerance after darkcal subtraction, left', $
					'0, label, maskv = abs(data) lt tolerance, left', $
					'2, float, '+string(tolerance)+', label_left=tolerance: , tag=tolerance', $
					'1, base,, row', $
					'0, button, OK, Quit, Tag=OK', $
					'2, button, Cancel, Quit, Tag=cancel' $
		]		

		a = cw_form(desc, /column, title='Bad pixels from powder')

		;; Only do this if OK is pressed (!!)
		if a.OK eq 1 then begin		
			tolerance = a.tolerance
		endif 
		if a.cancel eq 1 then begin
			return, -1
		endif
		
	endif




	;; Read in data
	print,'Source file: ', filename
	data = read_h5(filename, field='data/non_assembled_detector_corrected')
;	n = read_h5(filename, field='data/nframes')	

	s = size(data, /dim)
	ss = s / 8
	

	;; Bad pixels are at dark values even when illuminated
	maskv = abs(data) lt tolerance
	

	;; Static edge masks	
	maske = bytarr(s[0],s[1])
	
	
	;; Edges of the CSPAD ASICs  (cspad specific!)
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