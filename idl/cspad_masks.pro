;;
;;	Unbonded pixels behave exactly like normal pixels in dark frames
;;  We only see that they are dark when the whole detector is illuminated.
;;	Attempt to find unbonded pixels by identifying pixels which remain at 'dark' tolerances even when the whole detector is illuminated.
;;

pro cspad_masks, edge=edge, unbonded=unbonded, plus=plus

	;; Raw CSPDAD size
	m = bytarr(1552, 1480)

	s = size(m, /dim)
	ss = s / 8
	

	
	;; Edges of the CSPAD ASICs  (cspad specific!)
	maske = m
	if KEYWORD_SET(edge) then begin
		for i=0, 7 do begin
			maske[i*ss[0], *] = 1		
			maske[(i+1)*ss[0]-1, *] = 1		
			maske[*, i*ss[1]] = 1		
			maske[*, (i+1)*ss[1]-1] = 1		
		endfor
	endif	
	
	
	masku1 = m
	if keyword_set(unbonded) then begin
		for i=0, 7 do begin
			for j=0, 7 do begin
				cx = ss[0]*j
				cy = ss[1]*i
				
				for d=0, ss[1], 10 do begin
					masku1[cx+d, cy+d] = 1
					
				endfor					
			endfor
		endfor
	endif


	masku2 = m
	if keyword_set(plus) then begin
		for i=0, 7 do begin
			for j=0, 7 do begin
				cx = ss[0]*j
				cy = ss[1]*i
				
				for d=0, ss[1], 10 do begin
					masku1[cx+d, cy+d] = 1
					masku1[cx+d-1, cy+d] = 1
					masku1[cx+d+1, cy+d] = 1
					masku1[cx+d, cy+d+1] = 1
					masku1[cx+d, cy+d-1] = 1
					
				endfor					
			endfor
		endfor
	endif
	
	
	;; Compile final mask
	mask = masku1	OR maske

	;; Mask is actually the inverse (1=good, 0=bad)
	mask = 1-mask
	mask = byte(mask)
	
	;; Display it
	loadct, 0
	scrolldisplay, mask, title='Bad pixel mask'


	;; Save it
	fout = dialog_pickfile(title='Save file')
	if fout ne '' then $
		write_h5, fout, mask
end