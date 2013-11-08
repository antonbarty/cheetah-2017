function waxs1d, stackfile, find=find, normalise=normalise

	;; Find the file if needed
	if keyword_set(find) then begin
		file = file_search(stackfile, '*detector0-class0-stack*.h5')
		stackfile = file[0]
	endif
	
	
	;; Read the stack
	stack = read_h5(stackfile)
	
		
	;; Roughly normalise each line across a certain Q range
	if keyword_set(normalise) then begin
		stack = waxsnorm(stack)
	endif
	
	
	;; Average	 through the stack, in such a way as to reject outliers
	w = median(stack, dim=2)
	
	
	;; Write back out to csv file
	np = n_elements(w)
	index = indgen(np)
	csvout = transpose([[index],[w]])
	
	outfile = stackfile
	outfile = strmid(stackfile,0,strlen(stackfile)-10)
	outfile += '.csv'
	
	print,'Writing lineout to: ', outfile
	write_csv, outfile, index, w, header=['#q_pixels','Signal']


	;; Return data array
	return, w

end
