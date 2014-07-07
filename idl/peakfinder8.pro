;;
;; Peakfinder8
;; IDL version of the peakfinder8 algorithm in Cheetah
;;


function peakfinder8, data, snr, r=r, x=x, y=y, minpix=minpix, maxpix=maxpix, minr=minr, maxr=maxr, iter=iter, minADC=minADC

	;;
	;;	Defaults
	;;
	if NOT KEYWORD_SET(minpix) then minpix = 2
	if NOT KEYWORD_SET(maxpix) then maxpix = 200
	if NOT KEYWORD_SET(iter) then iter = 5
	if NOT KEYWORD_SET(minADC) then minADC = -1.e9
	maxpeaks = 1000


	;;
	;;	Checks
	;;
	if n_elements(data) eq 0 then begin
		print,'function peakfinder8, data, snr, r=r, x=x, y=y'
		stop
	endif

	;;
	;;	Create radial distance array
	;;
		s = size(data,/dim)
		if keyword_set(x) AND keyword_set(y) then begin
			d = sqrt(x*x+y*y)
		endif $
		else if keyword_set(r) then begin
			d = r
			x = findgen(s[0])
			y = findgen(s[1])
			print,'Radius supplied, (x,y) coordinates not, so peak (x,y) will be position in the array'
		endif $
		else begin
			d = dist(s[0],s[1])
			d = shift(d,s[0]/2,s[1]/2)
			x = xarr(s[0],s[1]) - s[0]/2
			y = yarr(s[0],s[1]) - s[1]/2
		endelse
		
	if NOT KEYWORD_SET(minr) then minr	= 0
	if NOT KEYWORD_SET(maxr) then maxr	= max(d)
	
	

	;;
	;;	Sort distances
	;;
		dmin = 0
		dmax = max(d)		
		hd = histogram(d, min=0, max=dmax, reverse_indices=ii, _extra=extra, locations=loc)

	;;
	;;	Determine radial dependent threshold
	;;
		thresh = hd*0.
		thresh[*] = 1e9

		for count = 0, iter do begin
			bg = hd*0.
			sig = hd*0.		
		
			for i = 1, n_elements(hd)-1 do begin
				if ii[i] NE ii[i+1] then begin
					list = ii[ii[i]:ii[i+1]-1]
					w = where(data[list] lt thresh[i])
					if w[0] ne -1 then begin
						bg[i] = mean(data[list[w]])
						sig[i] = stddev(data[list[w]])
						thresh[i] = bg[i] + snr*sig[i]
					endif
				endif
			endfor
			
			;plot, bg
			;oplot, sig
			;oplot, thresh
			;wait, 1

		endfor

		thresh = thresh > minADC

	;;
	;;	Create threshold map
	;;
		peakmask = intarr(s[0],s[1])
		
		for i = 1, n_elements(hd)-1 do begin
			if ii[i] NE ii[i+1] then begin	
				if i lt minr OR i gt maxr then continue	
				if finite(thresh[i]) then begin
					list = ii[ii[i]:ii[i+1]-1]
					w = where(data[list] gt thresh[i])
					if w[0] ne -1 then $
						peakmask[list[w]] = 1
				endif
			endif
		endfor


	;;
	;;	Label regions and determine which bits are peaks
	;;
	peakx = fltarr(maxpeaks)
	peaky = fltarr(maxpeaks)
	peakfs = fltarr(maxpeaks)
	peakss = fltarr(maxpeaks)
	peaktotal = fltarr(maxpeaks)
	peakpix = fltarr(maxpeaks)
	peakcounter = 0


	region = label_region(peakmask, /all, /ulong)
	h = histogram(region, reverse_indices=r)
	indices = lindgen(s[0],s[1])
	
	for i=0L, n_elements(h)-1 do begin
		npix = h[i]
		if npix lt minpix then continue
		if npix gt maxpix then continue

		;Find subscripts of members of region i.
		p = r[r[i]:r[i+1]-1]
		
	   ; Pixels of region i
   		q = data[p]
	
		; xy indices of region i	
		pxy = array_indices(data, p)
		px = x[p]
		py = y[p]
		pfs = reform(pxy[0,*])
		pss= reform(pxy[1,*])
		
		;; Centroid of region i
		centroid_x = total(px*q)/total(q)
		centroid_y = total(py*q)/total(q)
		centroid_fs = total(pfs*q)/total(q)
		centroid_ss = total(pss*q)/total(q)
		ptotal = total(data[p])

		peakx[peakcounter] = centroid_x
		peaky[peakcounter] = centroid_y
		peakfs[peakcounter] = centroid_fs
		peakss[peakcounter] = centroid_ss
		peaktotal[peakcounter] = ptotal
		peakpix[peakcounter] = npix
		peakcounter += 1


	endfor
	
	
	if peakcounter ne 0 then peakcounter -= 1
	peakx = peakx[0:peakcounter]
	peaky = peaky[0:peakcounter]
	peakfs = peakfs[0:peakcounter]
	peakss = peakss[0:peakcounter]
	peaktotal = peaktotal[0:peakcounter]
	peakpix = peakpix[0:peakcounter]
	
	
	out = { $
				x : peakx, $
				y : peaky, $
				fs : peakfs, $
				ss: peakss, $
				total : peaktotal, $
				npix : peakpix $
			}
				

	return, out

	
end
	
