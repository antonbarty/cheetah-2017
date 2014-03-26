pro saturation_check, file, threshold=threshold, stop=stop

	if n_elements(file) eq 0 then begin
		file = dialog_pickfile()		
		if file[0] eq '' then $
			return
	endif

	if NOT keyword_set(threshold) then $
		threshold = 9000

	print, 'Reading csv file: ', file
	a = read_csv(file, header=header)

	col_peak_r = 8
	col_peak_Q = 9
	col_peak_A = 10
	col_max = 13
	print, 'Peak r = ', header[col_peak_r]
	print, 'Peak q = ', header[col_peak_q]
	print, 'Peak A = ', header[col_peak_A]
	print, 'Peak max = ', header[col_max]
	
	peakFrame = float(a.field01)
	peakR = float(a.field09)
	peakQ = float(a.field10)
	peakA = float(a.field11)
	peakMax = float(a.field14)

	w = where(finite(peakR))
	peakFrame = peakFrame[w]
	peakR = peakR[w]
	peakQ = peakQ[w]
	peakA = peakA[w]
	peakMax = peakMax[w]
		
	
	;;
	;;	Basic stats
	;;
	print,'Number of peaks: ', n_elements(peakMax)
	print,'Number of saturated peaks: ', n_elements(where(peakMax gt threshold))
	
	;;
	;; Distribution of peak maxima
	;;
	h = histogram(peakMax, binsize=50, min=0, max=10000, locations=hx)
	p7 = plot(hx, h/1000, xtitle='Max ADU', ytitle='Frequency (x1000)', title=title)


	;;
	;;	Number of saturated peaks in resolution shells
	;;
	h = histogram(peakR, locations=xx, /nan, min=0, max=1200, binsize=20,  reverse_indices=ii)
	sat = h
	sat[*] = 0

	for i=0L, n_elements(h)-1 do begin
		if ii[i] NE ii[i+1] then $
			sat[i] = n_elements( where( peakMax[ii[ii[i]:ii[i+1]-1]] gt threshold ))
	endfor
	satp = 100.*float(sat)/h
		
	b = barplot(xx, satp, xtitle='Radius (pixels)', ytitle = 'Percentage of saturated peaks', title=title)
	t = text(0.15, 0.8, strcompress(string('Saturation threshold = ', threshold)))
	

	
	;;
	;;	Number of saturated peaks per pattern
	;;
	u = peakFrame[ uniq(peakFrame, sort(peakFrame)) ]
	np = n_elements(u)
	p = fltarr(np)
	ps = fltarr(np)
	for i=0, np -1 do begin
		w = where(peakFrame eq u[i])
		p[i] = n_elements(peakMax[w])
		ps[i] = n_elements(where(peakMax[w] gt threshold))
	endfor
	pp = ps/p
	pp *= 100

	w = where(finite(satp))
	satp = satp[w]
	xx = xx[w]
	
	title = file_dirname(file)
	title = file_basename(title)
	
	;p1 = plot(peakFrame, peakMax, linestyle=' ', symbol='.', xtitle='Event #', ytitle='Maximum ADC value', title=title)

	;p2 = plot(peakR, peakMax, linestyle=' ', symbol='.', xtitle='q (pixels)', ytitle='Maximum ADC value', title=title)

	;p3 = plot(u, pp, linestyle=' ', symbol='+', xtitle='Event #', ytitle='Saturated peaks per frame (%)', title=title)
	;t = text(0.15, 0.8, strcompress(string('Saturation = ', threshold)))

	;p4 = plot(u, ps, linestyle=' ', symbol='+', xtitle='Event #', ytitle='Saturated peaks per frame (#)', title=title)
	;t = text(0.15, 0.8, strcompress(string('Saturation = ', threshold)))


	;; Cumulative histograms
	h = histogram(pp, locations=px, min=0, binsize=1)
	ch = total(h,/cum)/total(h)

	p5 = barplot(px, 100*h/total(h), xtitle='Saturated peaks per frame (%)', ytitle='Proportion of frames (%)', title=title)
	t = text(0.15, 0.8, strcompress(string('Saturation = ', threshold)))

	p6 = plot(px, 100*ch, xtitle='Saturation tolerance (peaks per frame, %)', ytitle='Useable frames (%)', title=title)
	t = text(0.15, 0.8, strcompress(string('Saturation = ', threshold)))


	;;
	;;	2D histograms
	;;
	r  = hist_2d(peakR, peakMax, bin1=20, bin2=100, min1=0, min2=0)
	d = size(r, /dim)
	rx =  10*findgen(d[0])
	ry =  50*findgen(d[1])
	;isurface, r, rx, ry, xtitle='Radius (pix)', ytitle = 'Max ADU', ztitle='Count', rgb_table=33, /insert_colorbar, shading=1, texture_image=r, title=title
	;isurface, r, rx, ry, xtitle='Radius (pix)', ytitle = 'Max ADU', ztitle='Count', rgb_table=33, shading=1, title=title

	if keyword_set(stop) then $
		stop
end