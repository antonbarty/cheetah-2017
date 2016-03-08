pro plotct, ct

	;; Get CT names
	loadct, get_names = ct_name

	;; Load CT and get RGB values
	loadct, ct
	tvlct, r,g,b, /get
	
	
	;; Plot it
	x = findgen(n_elements(r))/n_elements(r)
	p = plot(x, r, color='r')
	pr = p
	pg = plot(x, g, color='g', /over)
	pb = plot(x, b, color='b', /over)

	p.title = string(ct) + ' : ' + ct_name[ct]
	p.yrange = [0, 260]
	p.xrange=[0.0, 1.0]


	;; Print it
	arr = [ [x], [r], [g], [b], [deriv(r)], [deriv(g)], [deriv(b)]]
	print, strcompress(string(transpose(arr)))

end