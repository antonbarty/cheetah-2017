function waxsnorm, stack

	
	;; Total up stack for reference values
	qlo = 400
	qhi = 800
	avg = median(stack,dim=2)
	ref = mean(avg[qlo:qhi])
	ref = 1
	
	;; Information
	str = strcompress(string('Normalising stack rows to signal in range q = [ ', qlo,' , ',qhi,' ] pixels'))
	print, str
	
	;; New stack
	s = size(stack, /dim)
	newstack = fltarr(s[0],s[1])

	;; Loop through frames
	for i=0L, s[1]-1 do begin
		row = reform(stack[*,i])
		t = mean(row[qlo:qhi])
		newrow = row/t
		newstack[*,i] = newrow
	endfor
	

	return, newstack

end
