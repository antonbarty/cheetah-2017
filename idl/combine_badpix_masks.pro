pro combine_badpix_masks, save=save

	file = dialog_pickfile(/multi, title='Select masks to merge')
	
	if file[0] eq '' then return
	
	
	
	for i=0, n_elements(file)-1 do begin
		a = read_h5(file[i])
		
		if n_elements(result) eq 0 then $
			result = a $
		else $
			result *= a
	endfor
	

	mask = result



	;; Display it
	loadct, 0
	scrolldisplay, mask, title='Combined bad pixel mask'


	;; Save it
	outdir = file_dirname(file[0])
	if keyword_set(save) then begin
		fout = dialog_pickfile(path=outdir, title='Save file')
		if fout ne '' then $
			write_h5, fout, mask
	endif



end