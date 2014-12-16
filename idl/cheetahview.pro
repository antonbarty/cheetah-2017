;;
;;	Cheetahview
;;	Tool for snooping through FEL diffraction images
;;
;;	Anton Barty, 2009-2013
;;

;;
;; Draw circles around peaks
;;
function cheetah_peakcircles, filename, image, pState, peakx, peaky

	;; Image of overlaid circles
	d = size(image,/dim)
	peakcircles = fltarr(d[0],d[1])
	
	;; Circle template
	r = dist(20,20)
	r = shift(r,10,10)
	circle = r ge 5 AND r lt 6

	;; Read peak info
	peakx = long(reform(peakx))
	peaky = long(reform(peaky))
	npeaks = n_elements(peakx)
	
	;; No peaks, give up
	if npeaks le 0 then $
		return, peakcircles

	;; Apply pixel map to peak positions (if needed)
	if (*pstate).use_pixmap then begin
		newpeakx = (*pstate).pixmap_x[peakx,peaky]/(*pstate).pixmap_dx
		newpeaky = (*pstate).pixmap_y[peakx,peaky]/(*pstate).pixmap_dx
		peakx = newpeakx
		peaky = newpeaky
	endif


	;; Center peaks
	if (*pstate).centeredPeaks then begin
		peakx += d[0]/2
		peaky += d[1]/2
	endif
	
	;; Make array bounds OK
	peakx = peakx > 15
	peaky = peaky > 15
	peakx = peakx < (d[0]-15)
	peaky = peaky < (d[1]-15)

	;; Corner is offset
	peakx -= 10
	peaky -= 10
	
	;; Create circles
	for i=0L, npeaks-1 do begin
		xx = round(peakx[i])
		yy = round(peaky[i])
		temp = peakcircles[xx:xx+19, yy:yy+19]
		temp += circle
		peakcircles[xx:xx+19, yy:yy+19] = temp
	endfor
	
	peakcircles = peakcircles < 1
	
	return, peakcircles	

end


pro cheetah_resolutionCircles, pState, filename, image
	
	s = size(image,/dim)
	win = (*pState).slideWin
	scroll = (*pState).scroll
	
	detectorZ_mm = read_h5(filename, field='LCLS/detector0-Position')
	wavelength_A = read_h5(filename, field='LCLS/photon_wavelength_A')
	detectorZ_mm = detectorZ_mm[0]
	wavelength_A = wavelength_A[0]
	
	print, 'Detector Z (mm) = ', detectorZ_mm
	print, 'Wavelength (A) = ', wavelength_A
	
	;; For making the circle the old-fashioned way
	np = 360
	ct = 2*!pi*findgen(np)/(np-1)
	cx = cos(ct)
	cy = sin(ct)
	
	WSET, (*pState).slideWin
	for d=2., 10., 1. do begin

		;; Lithographer or Crystallographer convention?
		if (*pState).resolutionRings1 eq 1 then $
			sin_t = wavelength_A / (2 * d) $
		else  $
			sin_t = wavelength_A / d
					
		tan_t = sin_t / sqrt(1-sin_t^2)
		r_mm = detectorZ_mm * tan_t
		r_pix = r_mm / 110e-3
		;print, d, sin_t, tan_t, r_mm, r_pix
		;print, d, r_pix
		;ring = ellipse(s[0]/2, s[1]/2, /device, color='red', major = r_pix, thick=1);, target=scroll)
		;label = text(s[0]/2+r_pix, s[1]/2, string(d, 'Å'), color='red', /data)
		plots, r_pix*cx+s[0]/2, r_pix*cy+s[1]/2, color=90, /device
		
		label = strcompress(string(fix(d),'A'),/remove_all)
		lx = s[0]/2 - r_pix/sqrt(2)
		ly = s[1]/2 - r_pix/sqrt(2)
		xyouts, lx, ly, label, /device
	endfor
	

end


;; Note: this routine is hard coded for CSpad data!!!
function cheetah_localbackground, data, radius

	if radius le 0 then begin
		return, 0
	endif
	
	
	s = size(data, /dim)
	if s[0] eq 1552 AND s[1] eq 1480 then cspad=1
	
	width = 2*radius+1
		
	if cspad then begin
		cspadx = 194
		cspady = 185
		m = fltarr(s[0],s[1])
		
		;; Subtract medians asic-wise
		for i=0, 7 do begin
			for j = 0, 7 do begin
				region = data[i*cspadx:(i+1)*cspadx-1, j*cspady:(j+1)*cspady-1]
				mm = median(region, width)
				m[i*cspadx,j*cspady] = mm
			endfor
		endfor		
		
		;; Ignore the bright ASIC edges (set background=data and these points will become 0)
		for i=0, 7 do begin
			m[*, i*cspady] = data[*,i*cspady]
			m[*, (i+1)*cspady-1] = data[*, (i+1)*cspady-1]
			m[i*cspadx, *] = data[i*cspadx,*]
			m[(i+1)*cspadx-1, *] = data[(i+1)*cspadx-1,*]
		endfor
	endif $
	else begin
		m = median(data, width)
	endelse

	return, m

end

;;
;;	Peakfinder 3
;;
function cheetah_peakfinder3, data, pState

	adc_thresh = (*pstate).peaks_ADC
	minpix = (*pstate).peaks_minpix
	maxpix = (*pstate).peaks_maxpix
	peaks_minsnr = (*pstate).peaks_minsnr 
	peaks_minres = (*pstate).peaks_minres
	peaks_maxres = (*pstate).peaks_maxres


	;; Array for peak information
	maxpeaks = 10000
	peakx = fltarr(maxpeaks)
	peaky = fltarr(maxpeaks)
	peaktotal = fltarr(maxpeaks)
	peakpix = fltarr(maxpeaks)
	peakcounter = 0
	
	s = size(data, /dim)
	temp = data


	region = label_region(temp gt adc_thresh, /all, /ulong)
	h = histogram(region, reverse_indices=r)
	indices = lindgen(s[0],s[1])
	
	for i=0L, n_elements(h)-1 do begin
	
		npix = h[i]
		if npix lt minpix then continue
		if npix gt maxpix then continue
		
		if peakcounter ge maxpeaks-1 then begin
			print,'More than allowed number of peaks found: ', maxpeaks
			break
		endif
		
		;Find subscripts of members of region i.
		p = r[r[i]:r[i+1]-1]
		
	   ; Pixels of region i
   		q = data[p]
	
		; xy indices of region i	
		pxy = array_indices(data, p)
		px = reform(pxy[0,*])
		py = reform(pxy[1,*])
		
		;; Centroid of region i
		centroid_x = total(px*q)/total(q)
		centroid_y = total(py*q)/total(q)
		ptotal = total(temp[p])
		

		;; Reject based on peak radius
		newpeakx = (*pstate).pixmap_x[centroid_x,centroid_y]/(*pstate).pixmap_dx
		newpeaky = (*pstate).pixmap_y[centroid_x,centroid_y]/(*pstate).pixmap_dx
		newpeakr = sqrt(newpeakx*newpeakx + newpeaky*newpeaky)
		if newpeakr gt peaks_maxres then continue
		if newpeakr lt peaks_minres then continue
		
		;; Reject based on signal:noise (not including peak)
		if peaks_minsnr ne 0 then begin
			region_xl = max([0, centroid_x-2*lbg])
			region_xh = min([s[0]-1, centroid_x+2*lbg])
			region_yl = max([0, centroid_y-2*lbg])
			region_yh = min([s[1]-1, centroid_y+2*lbg])
			;sd = stddev(temp[region_xl:region_xh, region_yl:region_yh] < adc_thresh)
			in = indices[region_xl:region_xh, region_yl:region_yh]
			in = in[where(temp[in] lt adc_thresh)]
			sd = stddev(temp[in])
			ipeak = max(temp[p])
			snr = ipeak/sd
		 	if snr lt peaks_minsnr then continue
		endif	
		
		peakx[peakcounter] = centroid_x
		peaky[peakcounter] = centroid_y
		peaktotal[peakcounter] = ptotal
		peakpix[peakcounter] = npix
		peakcounter += 1

	endfor
	
	if peakcounter ne 0 then peakcounter -= 1
	peakx = peakx[0:peakcounter]
	peaky = peaky[0:peakcounter]
	peaktotal = peaktotal[0:peakcounter]
	peakpix = peakpix[0:peakcounter]
	
	out = transpose([[peakx],[peaky],[peaktotal],[peakpix]])

	return, out

end

;;
;;	Peakfinder 8
;;
function cheetah_peakfinder8, data, pstate

	adc_thresh = (*pstate).peaks_ADC
	minpix = (*pstate).peaks_minpix
	maxpix = (*pstate).peaks_maxpix
	peaks_minsnr = (*pstate).peaks_minsnr 
	peaks_minres = (*pstate).peaks_minres
	peaks_maxres = (*pstate).peaks_maxres

	;; Pixelmap
	x = (*pstate).pixmap_x/(*pstate).pixmap_dx
	y = (*pstate).pixmap_y/(*pstate).pixmap_dx

	;; Call external peakfinder8
	peaks = peakfinder8(data, peaks_minsnr, x=x, y=y, minpix=minpix, maxpix=maxpix, minr=peaks_minres, maxr=peaks_maxres, iter=3, minADC=adc_thresh)
	
	;; Transform output format
	out = transpose([[peaks.fs],[peaks.ss],[peaks.total],[peaks.npix]])

		;out = { $
		;		x : peakx, $
		;		y : peaky, $
		;		fs : peakfs, $
		;		ss: peakss, $
		;		total : peaktotal, $
		;		npix : peakpix $
		;	}

	return, out
	
end


;;
;;	Find peaks
;;
function cheetah_findpeaks, data, pState


	;; Subtract local background if (*pstate).display_localbackground eq 1
	lbg = (*pstate).peaks_localbackground 
	m = 0
	if (*pstate).peaks_localbackground ne 0 then begin
		m = cheetah_localbackground(data, lbg) 
	endif
	temp = data
	temp -= m	


	;; Which algorithm should we use
	algorithm = (*pstate).peaks_algorithm

	case algorithm of
		0 : result = cheetah_peakfinder8(temp, pState)
		1 : result = cheetah_peakfinder3(temp, pState)
	endcase
	
	return, result 

end


;;
;;	Read data from file
;; 	(more complex now that we have multiple file types)
;;
function cheetah_readdata, filename, frame, pState, title=title, file_type=file_type, $
													image=image, peaks=peaks
	
		i = frame
		
		;; This bit is to allow the flexibility to call with and without pState being defined
		;; (currently needed to rean an initial frame in order to set up the initial graphics window)
		if ptr_valid(pstate) then begin	
			file = *(*pState).pfile
			index = (*pstate).index
			file_type = (*pState).file_type
			h5field = (*pstate).h5field
			filename = file[i]
			title = file_basename(filename)
			(*pState).currentFile = filename
		endif
	
	
		;;
		;; Single h5 file per frame (1st version of Cheetah)
		;;
		if file_type eq 'little_h5' then begin

			if n_elements(h5field) eq 0 then $
				h5field = 'data/data'
				
			if keyword_set(image) then begin
				data = read_h5(filename, field=h5field)
				return, data
			endif
		
			if keyword_set(peaks) then begin
				peakinfo = read_h5(filename, field='processing/hitfinder/peakinfo-raw')
				return, peakinfo
			endif $
		
			else begin
				print,'Oops, unsupported option for little h5 files'
			endelse
		
		endif
		
		;;
		;; CXIDB format files
		;;
		if file_type eq 'cxi' then begin

			if n_elements(h5field) eq 0 then $
				h5field = 'data/data'
			
			if keyword_set(image) then begin
				data = read_cxi_data(filename, i, field = 'entry_1/instrument_1/detector_1/data') 
				return, data
			endif 

			if keyword_set(peaks) then begin
				px = read_cxi_data(filename, i, field = 'entry_1/result_1/peakXPosRaw') 
				py = read_cxi_data(filename, i, field = 'entry_1/result_1/peakYPosRaw') 
				w = where(px ne 0 AND py ne 0)
				peakinfo = fltarr(4,n_elements(w))
				peakinfo[0,*] = px[w]
				peakinfo[1,*] = py[w]
				return, peakinfo
			endif $
		
			else begin
				print,'Unsupported option for CXI files'
			endelse
		
		endif

		
		;; We have an error if we get to here
		print,'It looks like some condition was not trapped in cheetahview_readdata()'
		stop


end


;;
;;	Display an image
;;
pro cheetah_displayImage, pState, image

		;; Retrieve file info
		file = *(*pState).pfile
		i = (*pState).currentFrameNum
		filename = file[i]
		(*pState).currentFile = filename


		catch, error

		;; Politely handle file reading errors		
		if error ne 0 then begin
			print,'Error reading and processing: ', filename
			print,!error_state.msg
			catch, /cancel
			return
		endif

		
		data = cheetah_readdata(filename, i, pState, title=title, /image)
		;;data = read_h5(filename, field=(*pstate).h5field)
		;title = file_basename(filename)
		data = float(data)

		
		;; Apply local background to display image?
		if (*pstate).display_localbackground eq 1 then begin
				lbg = (*pstate).peaks_localbackground 
				m = cheetah_localbackground(data, lbg) 
				data -= m
		endif
		
		;; Apply histogram clipping (cap top and bottom fraction)
		if (*pState).image_histclip ne 0 then begin
			hist_thresh = (*pState).image_histclip
			data = histogram_clip(data, hist_thresh, hist_thresh)
		endif


		;; Find or load peaks
		if (*pState).circleHDF5Peaks then begin
			peakinfo = cheetah_readdata(filename, i, pState, /peaks)
			;peakinfo = read_h5(filename, field='processing/hitfinder/peakinfo-raw')
		endif
		if (*pState).findPeaks then begin
			peakinfo = cheetah_findpeaks(data, pState)
		endif

		;; Apply pixel map
		if (*pstate).use_pixmap then begin
			image = pixelRemap(data, (*pstate).pixmap_x, (*pstate).pixmap_y, (*pstate).pixmap_dx)
		endif $
		else $
			image = data

		;; Rescale image
		image = (image>0)
		image = image < (*pState).image_max

		if (*pState).circleHDF5Peaks OR (*pState).findPeaks then begin
			peakx = long(reform(peakinfo(0,*)))
			peaky = long(reform(peakinfo(1,*)))
			in = where(peakx ne 0 AND peaky ne 0)
			peakx = peakx[in]
			peaky = peaky[in]
			print,'Peaks found: ', n_elements(peakx)

			circles = cheetah_peakcircles(filename, image, pState, peakx, peaky)
			m = max(image) 			
			image += (m*circles)
			image = image < m
		endif

		
		;; Display corrections
		image = image < (max(image)/((*pState).image_boost))
		image = (image > 0)^((*pState).image_gamma)


		;; Resize image depending on zoom factor

		z = (*pstate).image_zoom 
		if z ne 1.0 then begin
			sz = size(image,/dim)
			image = congrid(image, z*sz[0], z*sz[1])		
		endif
		(*pstate).image_size = size(image,/dim)
		
		
		;; If the image is much smaller than the draw region, put it in the center of the image space
		g = widget_info((*pState).scroll, /geometry)
		s = size(image, /dim)
		;;help, g
		if( (g.draw_xsize - s[0] ) gt 100  and  (g.draw_ysize - s[1] ) gt 100) then begin
			tv_img = fltarr(g.draw_xsize, g.draw_ysize)
			tv_img[(g.draw_xsize - s[0])/2, (g.draw_ysize - s[1])/2] = image
		endif $
		else begin
			tv_img = image
		endelse


		
		;; Display image
		(*pState).data = data
		widget_control, (*pState).base, base_set_title=title
		WSET, (*pState).slideWin
		loadct, (*pstate).colour_table, /silent
		tvscl, tv_img

		;; Resolution rings
		if (*pState).resolutionRings1 eq 1 or (*pState).resolutionRings2 eq 1 then begin
			cheetah_resolutionCircles, pState, filename, image
		
		endif


		;; Widget_control
		thisfile = (*pState).currentFrameNum+1
		numfiles = n_elements(*((*pstate).pfile))
		str = strcompress(string('(image',thisfile,' of ',numfiles,')'))
		widget_control, (*pState).status_label, set_value=str


		;; Save peak list
		if (*pState).findPeaks then begin
			if (*pstate).savePeaks then begin
				cheetah_overwritePeaks, filename, peakinfo
			endif
		endif


		;; Turn off error handler
		catch, /cancel



end

;;
;;	Save optimised IDL found peak list back into HDF5 file
;;
pro cheetah_overwritePeaks, filename, peakinfo

	print, 'Saving found peaks back into HDF5 file '
	s = size(peakinfo, /dim)
	if n_elements(s) ne 2 then return
	
	file_id = H5F_OPEN(filename, /write) 
	
	;; Determine whether peakinfo-refined field exists.
	;; If the field exists, we need to use H5D_OPEN
	;; If the field does not exist, H5D_OPEN will crash and we need to use H5D_CREATE_SIMPLE instead
	;; Finding out this information is a little tricky (!)
	
	;h5_fields = h5_parse(file_id, 'processing/hitfinder', file=filename)
	;names = tag_names(h5_fields)

	;;peaklink = h5g_get_linkval(file_id,'processing/hitfinder/peakinfo')
	;;if peaklink eq '/processing/hitfinder/peakinfo-raw' then field_present=0 else field_present=1

	h5_fields = h5_parse(filename)
	names = tag_names(h5_fields.processing.hitfinder)
	w = where(names eq 'PEAKINFO_REFINED')
	if w[0] eq -1 then field_present=0 else field_present=1


	;; If field already defined - figure out how big it is and resize the array accordingly
	if field_present then begin
		print, 'peakinfo_refined is already defined:'
	 	dataset_id = H5D_OPEN(file_id, 'processing/hitfinder/peakinfo-refined') 
		raw_dataset_id = H5D_GET_SPACE(dataset_id)
		raw_dims = H5S_GET_SIMPLE_EXTENT_DIMS(raw_dataset_id)
		snew = size(peakinfo,/dim)
		msg = strcompress(string('Existing peak list is ',raw_dims[1],' long, New peak list is ', snew[1], ' long'))
		print, msg
		;print,'Peakinfo_refined is: ', raw_dims
		;print,'New peak list is: ', size(peakinfo,/dim)
	
		if s[1] le raw_dims[1] then begin
			print,'Array will be zero padded'
			temp = dblarr(raw_dims)
			temp[0,0] = peakinfo
		endif $
		else begin
			print,strcompress(string('Peak list truncated to H5 container size: ( best ', raw_dims[1],' peaks saved )'))
			val = reform(peakinfo[3,*])
			srt = reverse(sort(val))
			peakinfo[*,*] = peakinfo[*,srt]
			temp = double(peakinfo[*,0:raw_dims[1]-1])
		endelse

		H5D_WRITE,dataset_id, temp, file_space_id=raw_dataset_id  		
		H5S_CLOSE, raw_dataset_id
		H5D_CLOSE,dataset_id   
		
	endif $
	
	else begin
			print,'Creating new hdf5 field: processing/hitfinder/peakinfo-refined to hold up to 1000 peaks'
			temp = dblarr(4,1000)
			temp[0,0] = peakinfo
			datatype_id = H5T_IDL_CREATE(temp) 
			dataspace_id = H5S_CREATE_SIMPLE(size(temp,/DIMENSIONS), max_dimensions=[4,-1]) 
			dataset_id = H5D_CREATE(file_id,'processing/hitfinder/peakinfo-refined', datatype_id, dataspace_id,  CHUNK_DIMENSIONS=[4,50] ) 
			H5D_WRITE, dataset_id, temp
			H5D_CLOSE,dataset_id   
			H5S_CLOSE,dataspace_id 
			H5T_CLOSE,datatype_id 
	endelse

	;; Fix up symbolic links
	H5G_UNLINK, file_id, 'processing/hitfinder/peakinfo'
	H5G_LINK, file_id, 'processing/hitfinder/peakinfo-refined', 'processing/hitfinder/peakinfo'	

	H5F_CLOSE, file_id 

end

;;
;; Update file list
;; (a little more complex now with different file types)
;;
function cheetahview_updatefilelist, dir

	file = ['']
	
	;; Find .cxi files
	file = file_search(dir,"*.cxi",/fully_qualify)
	if n_elements(file) ne 0 AND file[0] ne '' then begin
		file_type = 'cxi'
		print,strcompress(string('CXI file: ', file_basename(file)))
		
		nframes = read_cheetah_cxi(file, /get_nframes)
		print, strcompress(string('Number of frames: ', nframes))

		file = replicate(file[0], nframes)
		index = indgen(nframes)
		
	endif $

	;; Find single frane .h5 files
	else begin
		file = file_search(dir,"LCLS*.h5",/fully_qualify)	
	 	if n_elements(file) eq 0 OR file[0] eq '' then begin
		 	message,'No files found in directory: '+dir, /info
	 	endif
		file_type = 'little_h5'
		index = intarr(n_elements(file))
		print,strcompress(string(n_elements(file),' files found'))
	endelse

	;; Return result and indexes
	result = {	file_type : file_type, $
					file : file, $
					index : index $
				}
	return, result
end





;;
;;	Resize the window
;;
pro cheetah_resizewindow, pstate
  	sState = *pState
	s = (*pstate).image_size
	screensize = get_screen_size()
	screensize -= 140
	new_xsize = min([screensize[0],s[0]])
	new_ysize = min([screensize[1],s[1]])
	WIDGET_CONTROL, sState.scroll, xsize=new_xsize, ysize=new_ysize, draw_xsize=s[0],draw_ysize=s[1]
end



;;
;;	=================================
;;       Main event processing switch statement
;;	=================================
;;
pro cheetah_event, ev

  	WIDGET_CONTROL, ev.top, GET_UVALUE=pState
  	sState = *pState
	img_size = (*pstate).image_size
	
	
	;; Establish polite error handler to catch crashes
	;; (only if not in debug mode)
	if 1 then begin
		catch, Error_status 
		if Error_status ne 0 then begin
			message = 'Execution error: ' + !error_state.msg
			r = dialog_message(message,title='Error',/center,/error)
			catch, /cancel
			return
		endif 
	endif

	
	case ev.id of 
		
		;;
		;;	Change in screensize
		;;
		ev.top : begin
			screensize = get_screen_size()
			x_padding = sState.padding.x
			y_padding = sState.padding.y
			min_xsize = 200
			min_ysize = 200

			new_xsize = (ev.x gt x_padding) ? (ev.x - x_padding) : x_padding
			new_ysize = (ev.y gt y_padding) ? (ev.y - y_padding) : y_padding
			new_xsize = (new_xsize lt min_xsize) ? min_xsize : new_xsize
			new_ysize = (new_ysize lt min_ysize) ? min_ysize : new_ysize
			new_xsize = (new_xsize gt img_size[0]) ? img_size[0] : new_xsize
			new_ysize = (new_ysize gt img_size[1]) ? img_size[1] : new_ysize
			new_xsize = (new_xsize gt screensize[0]-50) ? screensize[0]-50 : new_xsize
			new_ysize = (new_ysize gt screensize[1]-50) ? screensize[1]-50 : new_ysize
	
			WIDGET_CONTROL, sState.scroll, xsize=new_xsize, ysize=new_ysize
		end
		
		;;
		;;	Scrolling
		;;
		sState.scroll : begin
			if ev.type eq 3 then begin  ; Scroll event
				WSET, sState.slideWin
				loadct, 4, /silent
				tvscl, sState.image
			endif
	
			if ev.type eq 4 then begin ; Expose event
				WSET, sState.slideWin
				loadct, 4, /silent
				tvscl, sState.image
			endif
		end

		;;
		;;	Save image
		;;
		sState.menu_save : begin
			cheetah_displayImage, pState, image

			outfile = file_basename(sState.currentFile)
			outfile = strmid(outfile, 0, strlen(outfile)-3)+'.png'
			outfile = file_basename(outfile)
			filename = dialog_pickfile(file=outfile, path=sState.savedir, filter='*.png', /write)
			if (filename eq '') then $
				return
				
			idl_write_png,filename, image
			newdir = file_dirname(filename)
			(*pState).savedir = newdir
		end


		;;
		;;	Save data
		;;
		sState.menu_savedata : begin
			outfile = file_basename(sState.currentFile)
			outfile = strmid(outfile, 0, strlen(outfile)-3)+'.h5'
			filename = dialog_pickfile(file=outfile, path=sState.savedir, filter='*.h5', /write)
			if (filename eq '') then $
				return

			write_h5,filename, sState.data
			newdir = file_dirname(filename)
			(*pState).savedir = newdir
		end

		;;
		;; Display the next image
		;;
		sState.button_next : begin
			;; Choose the next file
			file = *sState.pfile
			i = sState.currentFrameNum+1
			if(i ge n_elements(file)) then $
				i = 0				
			filename = file[i]

			;; Display it
			(*pState).currentFile = filename
			(*pState).currentFrameNum = i
			cheetah_displayImage, pState

			;; Again?			
			if (*pstate).autoNext eq 1 then $
				widget_control, sState.button_next,  timer=1

		end


		;;
		;; Display the last image
		;;
		sState.button_previous : begin
			;; Choose the next file
			file = *sState.pfile
			i = sState.currentFrameNum-1
			if(i lt 0) then $
				i = n_elements(file)-1				
			filename = file[i]

			;; Display it
			(*pState).currentFile = filename
			(*pState).currentFrameNum = i
			cheetah_displayImage, pState
		end


		;;
		;;	Display a single random image
		;;
		sState.button_random : begin

			;; Choose a random file
			file = *sState.pfile
			i = randomu(systime(1))*n_elements(file)
			filename = file[i]

			;; Display it
			(*pState).currentFile = filename
			(*pState).currentFrameNum = i
			cheetah_displayImage, pState
			
			;; Again?			
			if (*pstate).autoShuffle eq 1 then $
				widget_control, sState.button_random,  timer=1

		end

		sState.menu_random : begin
	    	if sState.autoNext eq 0 then $
	    		return

      		widget_control, sState.button_auto, set_value='Stop'
      		(*pstate).autoNext = 1

      		widget_control, sState.button_random, timer=0.1      		
	      	widget_control, sState.menu_random,  timer=2
		end


		;;
		;; Auto shuffle through images
		;;
		sState.button_auto : begin
			if (*pstate).autoNext eq 1 then begin
	      		widget_control, sState.button_auto, set_value='Auto'
	      		(*pstate).autoNext = 0
			endif $
			else begin
	      		widget_control, sState.button_auto, set_value='Stop'
	      		(*pstate).autoNext = 1
				widget_control, sState.button_next,  timer=0.1
			endelse
		end
		
		sState.button_shuffle : begin
			if (*pstate).autoShuffle eq 1 then begin
	      		widget_control, sState.button_shuffle, set_value='Shuffle'
	      		(*pstate).autoShuffle = 0
			endif $
			else begin
	      		widget_control, sState.button_shuffle, set_value='Stop'
	      		(*pstate).autoShuffle = 1
				widget_control, sState.button_random,  timer=0.1
			endelse
		end


		;;
		;;	Select a new directory
		;;
		sState.menu_newdir : begin
			newdir = dialog_pickfile(/directory, path=sState.dir)
			
			file_type = 'None'
			
			result = cheetahview_updatefilelist(sState.dir)
			file_type = result.file_type
			newfile = result.file
			index = result.index
			help, newfile
			;;newfile = file_search(newdir,"LCLS*.h5",/fully_qualify)
			
			if n_elements(newfile) eq 0 then begin
				message,'No files found in directory: '+newdir, /info
				return
			endif
			file = newfile
			
			if(ptr_valid((*pState).pfile)) then $
				ptr_free, (*pState).pfile
			(*pState).pfile = ptr_new(newfile,/no_copy)
			(*pState).dir = newdir
			(*pstate).file_type = file_type
			(*pState).currentFrameNum = 0
			(*pstate).index = index

			cheetah_displayImage, pState
		end
				
		
		;;
		;;	Refresh file list
		;;
		sState.button_updatefiles : begin
		
			file_type = 'None'
			result = cheetahview_updatefilelist(sState.dir)
			file_type = result.file_type
			newfile = result.file
			index = result.index
			;newfile = file_search(sState.dir,"LCLS*.h5",/fully_qualify)
			
			if n_elements(newfile) eq 0 then begin
				message,'No files found in directory: '+sState.dir, /info
				return
			endif
			file = newfile
			
			if(ptr_valid((*pState).pfile)) then $
				ptr_free, (*pState).pfile
			(*pState).pfile = ptr_new(newfile,/no_copy)
			(*pState).currentFrameNum = n_elements(file)-1
			(*pstate).file_type = file_type
			(*pstate).index = index

			cheetah_displayImage, pState

		end
		
		;;
		;;	Circle peaks?
		;;
		sState.menu_circleHDF5Peaks : begin
			(*pstate).circleHDF5Peaks = 1-sState.circleHDF5Peaks
			(*pstate).findPeaks = 0
			(*pstate).savePeaks = 0
			widget_control, sState.menu_circleHDF5Peaks, set_button = (*pstate).circleHDF5Peaks
			widget_control, sState.menu_findPeaks, set_button = 0
			widget_control, sState.menu_savePeaks, set_button = 0
			file = *sState.pfile
			i = sState.currentFrameNum
			filename = file[i]
			cheetah_displayImage, pState
		end
		sState.menu_findPeaks : begin
			(*pstate).findPeaks = 1-sState.findPeaks
			(*pstate).circleHDF5Peaks = 0
			widget_control, sState.menu_findPeaks, set_button = (*pstate).findPeaks
			widget_control, sState.menu_circleHDF5Peaks, set_button = 0
			file = *sState.pfile
			i = sState.currentFrameNum
			filename = file[i]
			cheetah_displayImage, pState
		end

		sState.menu_savePeaks : begin
			(*pstate).savePeaks = 1-sState.savePeaks
			widget_control, sState.menu_savePeaks, set_button = (*pstate).savePeaks
		end
		
		;; Display with local background
		sState.menu_localBackground : begin
			(*pstate).display_localbackground = 1-sState.display_localbackground
			widget_control, sState.menu_localBackground, set_button = (*pstate).display_localbackground	
			cheetah_displayImage, pState
		end
		
		;;
		;;	Resolution (in two conventions)
		;;
		sState.menu_resolution1 : begin
			(*pstate).resolutionRings1 = 1-sState.resolutionRings1
			(*pstate).resolutionRings2 = 0
			widget_control, sState.menu_resolution1, set_button = (*pstate).resolutionRings1
			widget_control, sState.menu_resolution2, set_button = 0
			cheetah_displayImage, pState
		end
		sState.menu_resolution2 : begin
			(*pstate).resolutionRings1 = 0
			(*pstate).resolutionRings2 = 1-sState.resolutionRings2
			widget_control, sState.menu_resolution1, set_button = 0
			widget_control, sState.menu_resolution2, set_button = (*pstate).resolutionRings2
			cheetah_displayImage, pState
		end
		
		
		;;
		;;	Front or back detectors
		;;
		sState.menu_datadata : begin
			(*pstate).h5field = 'data/data'
			cheetah_displayImage, pState
		end
		sState.menu_detector0 : begin
			(*pstate).h5field = 'data/rawdata0'
			cheetah_displayImage, pState
		end
		sState.menu_detector1 : begin
			(*pstate).h5field = 'data/rawdata1'
			cheetah_displayImage, pState
		end



		;;
		;;	Profiles
		;;
		sState.menu_profiles : begin
			WSET, sState.slideWin
			profiles, sState.image
		end


		;;
		;;	ROI statistics
		;;
		sState.menu_ROIstats : begin
			WSET, sState.slideWin
			region = cw_defroi(sState.scroll)
			print,'>-----------------------<'
			print,'Mean = ', mean(sState.data[region])
			print,'Median = ', median(sState.data[region])
			print,'Maximum = ', max(sState.data[region])
			print,'Minimum = ', min(sState.data[region])
			print,'Standard deviation = ', stddev(sState.data[region])
			print,'>-----------------------<'
		end


		;;
		;;	Refresh image (clears ROI)
		;;
		sState.menu_refresh : begin
			cheetah_displayImage, pState
		end
		
		sState.button_refresh : begin
			cheetah_displayImage, pState
		end

		;;
		;; Process all images
		;;
		sState.menu_processall : begin
			res = dialog_message('Do you really want to do this?  This command will automatically step through each image once and could take some time to complete.', /cancel, /default_cancel)

			if res eq 'OK' then begin
				file = *sState.pfile
				nfiles = n_elements(file)
				for i=0L, nfiles-1 do begin
					(*pState).currentFrameNum = i
					cheetah_displayImage, pState
				endfor
			endif
		end


		;;
		;; Change image display parameters
		;;
		sState.menu_display : begin
			desc = [ 	'1, base, , column', $
						'0, float, '+string(sState.image_gamma)+', label_left=Gamma:, width=20, tag=image_gamma', $
						'0, float, '+string(sState.image_boost)+', label_left=Boost:, width=20, tag=image_boost', $
						'0, float, '+string(sState.image_histclip)+', label_left=Histogram clip:, width=20, tag=image_histclip', $
						'0, float, '+string(sState.image_max)+', label_left=Max value:, width=20, tag=image_max', $
						'2, text, '+string(sState.h5field)+', label_left=HDF5 field:, width=50, tag=h5field', $
						'1, base,, row', $
						'0, button, OK, Quit, Tag=OK', $
						'2, button, Cancel, Quit' $
			]		
			a = cw_form(desc, /column, title='Image display')
			
			if a.OK eq 1 then begin		
				(*pstate).image_max = a.image_max
				(*pstate).image_gamma = a.image_gamma
				(*pstate).image_histclip = a.image_histclip
				(*pstate).image_boost = a.image_boost
				(*pstate).h5field = a.h5field
			endif

			cheetah_displayImage, pState
		
		end



		;;
		;; Change peak finding parameters
		;;
		sState.menu_peakfinding : begin
			desc = [ 	'1, base, , column', $
						'0, float, '+string(sState.peaks_localbackground)+', label_left=LocalBackgroundRadius:, width=10, tag=peaks_localbackground', $
						'0, droplist, peakfinder 8|peakfinder 3, label_left=Algorithm (hitfinderAlgorithm):,  tag=algorithm', $
						'0, float, '+string(sState.peaks_ADC)+', label_left=Intensity threshold (hitfinderADC):, width=10, tag=peaks_ADC', $
						'0, float, '+string(sState.peaks_minpix)+', label_left=Minimum pixels per peak (hitfinderMinPixCount):, width=10, tag=peaks_minpix', $
						'0, float, '+string(sState.peaks_maxpix)+', label_left=Maximum pixels per peak (hitfinderMaxPixCount):, width=10, tag=peaks_maxpix', $
						'0, float, '+string(sState.peaks_minsnr)+', label_left=Minimum signal-to-noise ratio (I/sigma) (hitfinderMinSNR):, width=10, tag=peaks_minsnr', $
						'0, float, '+string(sState.peaks_minres)+', label_left=Inner region radius (hitfinderMinRes):, width=10, tag=peaks_minres', $
						'2, float, '+string(sState.peaks_maxres)+', label_left=Outer region radius (hitfinderMaxRes):, width=10, tag=peaks_maxres', $
						'1, base,, row', $
						'0, button, OK, Quit, Tag=OK', $
						'2, button, Cancel, Quit' $
			]
			a = cw_form(desc, /column, title='Peakfinder settings')
			
			if a.OK eq 1 then begin		
				(*pstate).peaks_localbackground = a.peaks_localbackground
				(*pstate).peaks_algorithm = a.algorithm
				(*pstate).peaks_ADC = a.peaks_ADC
				(*pstate).peaks_minpix = a.peaks_minpix
				(*pstate).peaks_maxpix = a.peaks_maxpix
				(*pstate).peaks_minsnr = a.peaks_minsnr
				(*pstate).peaks_minres = a.peaks_minres
				(*pstate).peaks_maxres = a.peaks_maxres
			endif

			cheetah_displayImage, pState
		end
		

		;; Load geometry
		sState.menu_geom : begin
			pixmap_file = dialog_pickfile(filter='*.h5', title='Select detector geometry file')
			if pixmap_file eq '' then $
				return
				
			(*pstate).pixmap_x = read_h5(pixmap_file,field='x')
			(*pstate).pixmap_y = read_h5(pixmap_file,field='y')
			(*pstate).pixmap_dx = 110e-6		;; cspad
			(*pstate).use_pixmap = 1

			(*pstate).centeredPeaks = 1
			widget_control, sState.menu_centeredPeaks, set_button = (*pstate).centeredPeaks

			image = (*pstate).data
			image = pixelRemap(image, (*pstate).pixmap_x, (*pstate).pixmap_y, (*pstate).pixmap_dx)
			s = size(image, /dim)
			screensize = get_screen_size()
			screensize -= 140
			new_xsize = min([screensize[0],s[0]])
			new_ysize = min([screensize[1],s[1]])
			WIDGET_CONTROL, sState.scroll, xsize=new_xsize, ysize=new_ysize, draw_xsize=s[0],draw_ysize=s[1]

			cheetah_displayImage, pState
		end
		
		;; Colour map
		sState.colourList : begin
			if ev.value eq 0 then $
				xloadct $
			else begin
				ct = ev.value-1
				(*pstate).colour_table = ct
				loadct, ct, /silent
				widget_control, ((*pstate).ColourListID)[ev.value], SET_BUTTON=1
			endelse

			cheetah_displayImage, pState
		end


		sState.menu_centeredPeaks : begin
			(*pstate).centeredPeaks = 1-(*pstate).centeredPeaks
			widget_control, sState.menu_centeredPeaks, set_button = (*pstate).centeredPeaks
		end
		

		sState.menu_cdiDefaults : begin
			loadct, 4, /silent		
			(*pstate).colour_table = 4
			(*pstate).image_gamma = 0.25
			(*pstate).image_boost = 1
			(*pstate).image_histclip = 0.0001
			cheetah_displayImage, pState
		end


		sState.menu_crystDefaults : begin
			loadct, 41, /silent		
			(*pstate).colour_table = 41
			(*pstate).image_gamma = 1
			(*pstate).image_boost = 1
			(*pstate).image_histclip = 0.0001
			(*pstate).circleHDF5Peaks = 1
			(*pstate).findPeaks = 0
			(*pstate).savePeaks = 0
			widget_control, sState.menu_circleHDF5Peaks, set_button = (*pstate).circleHDF5Peaks
			widget_control, sState.menu_findPeaks, set_button = 0
			widget_control, sState.menu_savePeaks, set_button = 0

			cheetah_displayImage, pState
		end
		
		
		;;
		;;	Image zoom
		;;
		sState.menu_localzoom : begin
			zoom
		end		

		sState.menu_zoom50 : begin
			(*pstate).image_zoom = .5
			cheetah_displayImage, pState
			cheetah_resizewindow, pstate
			cheetah_displayImage, pState
		end		

		sState.menu_zoom100 : begin
			(*pstate).image_zoom = 1.0
			cheetah_displayImage, pState
			cheetah_resizewindow, pstate
			cheetah_displayImage, pState
		end		

		sState.menu_zoom150 : begin
			(*pstate).image_zoom = 1.5
			cheetah_displayImage, pState
			cheetah_resizewindow, pstate
			cheetah_displayImage, pState
		end		

		sState.menu_zoom200 : begin
			(*pstate).image_zoom = 2.0
			cheetah_displayImage, pState
			cheetah_resizewindow, pstate
			cheetah_displayImage, pState
		end		
		

		else: begin
		end
		
	endcase  
	
	catch, /cancel
end


;;
;;	=================================
;;       Main GUI setup code
;;	=================================
;;
pro cheetahview, geometry=geometry, dir=dir

	;;	Select data directory
	if not keyword_set(dir) then begin
		dir = dialog_pickfile(/directory, title='Select data directory')
		if dir eq '' then return
	endif
	

	;; Find files and file type
	file_type = 'None'
	result = cheetahview_updatefilelist(dir)
	file_type = result.file_type
	file = result.file
	index = result.index
	
	if n_elements(file) eq 0 OR file[0] eq '' then begin
		message,'No files found in directory: '+dir, /info
		return
	endif

	savedir=dir
	

	;; If geometry file is specified...
	if keyword_set(geometry) then begin
		pixmap = 1
		pixmap_x = read_h5(geometry,field='x')
		pixmap_y = read_h5(geometry,field='y')
		pixmap_dx = 110e-6		;; cspad
		centeredPeaks = 1
	endif $
	else begin
		;; Set default pixel map (none)
		s = size(data, /dim)
		pixmap = 0
		pixmap_x = fltarr(s[0],s[1])
		pixmap_y = fltarr(s[0],s[1])
		pixmap_dx = 110e-6
		centeredPeaks = 0
	endelse


	;; Sample data
	filename = file[0]
;	data = read_h5(filename)
	data = cheetah_readdata(filename, 0, file_type=file_type,/image)
	if n_elements(data) eq 0 then begin
		 s = size(pixmap_x, /dim)
		 data = xarr(s[0],s[1])
	endif
	
		
	image = (data>0)
	image = image < 10000

	if keyword_set(geometry) then $
		image = pixelRemap(image, pixmap_x, pixmap_y, pixmap_dx)


	title = file_basename(file[0])
	oldwin = !d.window

	loadct, 4, /silent
	device, decomposed=0

	;;	Base widget
	base = WIDGET_BASE(title=title, GROUP=GROUP, mbar=bar, /COLUMN, /TLB_SIZE_EVENTS)
	WIDGET_CONTROL, /MANAGED, base

	;;	Size to make image
	s = size(image, /dim)
	screensize = get_screen_size()
	screensize -= 140
	xview = min([screensize[0],s[0]])
	yview = min([screensize[1],s[1]])

	;; Create inverse-BW colour table (X-ray film style)
	loadct, 0, rgb_table=bw, /silent
	invbw = reverse(bw, 1)

	;; Load colour table names
	loadct, 0, /silent
	loadct, get_names=table_names
	colour_list = string(indgen(n_elements(table_names)))+' '+table_names
	colour_list = '0\'+colour_list
	colour_list = ['0\Xloadct',colour_list]
	colour_list = strcompress(colour_list)
	colour_list[n_elements(colour_list)-1] = '2'+ strmid(colour_list[n_elements(colour_list)-1],1)


	;;	Create save menu
	mbfile = widget_button(bar, value='File')
	mbfile_geom = widget_button(mbfile, value='Load detector geometry')
	mbfile_si = widget_button(mbfile, value='Save image')
	mbfile_sd = widget_button(mbfile, value='Save raw data')
	mbfile_sda = widget_button(mbfile, value='Save assembled data')
	mbfile_random = widget_button(mbfile, value='Auto random image')
	mbfile_newdir = widget_button(mbfile, value='Change directory')
	mbfile_updatefilelist = widget_button(mbfile, value='Refresh file list')

	;; Colour scales
	mbcolours = widget_button(bar, value='Colours')
	mbcolours_1 = CW_PDMENU(mbcolours, colour_list, /MBAR, IDs=mbcolours_IDs)
		
	;;	Create analysis menu
	mbanalysis = widget_button(bar, value='Analysis')
	mbanalysis_profiles = widget_button(mbanalysis, value='Profiles')
	mbanalysis_ROI = widget_button(mbanalysis, value='ROI statistics')
	mbanalysis_refresh = widget_button(mbanalysis, value='Refresh image')
	mbanalysis_processall = widget_button(mbanalysis, value='Process all images')


	;; Create particles menu
	mbcdi = widget_button(bar, value='CDI')
	mbanalysis_cdidefaults = widget_button(mbcdi, value='Default cdi display settings')

	;;	Create crystals menu
	mbcryst = widget_button(bar, value='Crystals')
	mbanalysis_crystdefaults = widget_button(mbcryst, value='Default crystal display settings')
	mbanalysis_circleHDF5Peaks = widget_button(mbcryst, value='Circle HDF5 peaks', /checked)
	mbanalysis_findPeaks = widget_button(mbcryst, value='Circle IDL found peaks', /checked)
	mbanalysis_peakfinding = widget_button(mbcryst, value='Peak finding settings')
	mbanalysis_savePeaks = widget_button(mbcryst, value='Save IDL found peaks to H5 file', /checked)
	mbanalysis_centeredPeaks = widget_button(mbcryst, value='Peaks relative to image centre', /checked, sensitive=0)
	mbanalysis_displayLocalBackground = widget_button(mbcryst, value='Display with local background subtraction', /checked)
	widget_control, mbanalysis_centeredPeaks, set_button=centeredPeaks

	mbview = widget_button(bar, value='View')
	mbanalysis_imagescaling = widget_button(mbview, value='Image display settings')
	mbanalysis_resolution2 = widget_button(mbview, value='Resolution rings (Crystallographer, wl = d sin(theta))', /checked)
	mbanalysis_resolution1 = widget_button(mbview, value='Resolution rings (Lithographer, wl = 2d sin(theta))', /checked)
	mbanalysis_datadata = widget_button(mbview, value='data/data', sensitive=1, /separator)
	mbanalysis_detector0 = widget_button(mbview, value='data/rawdata0', sensitive=1)
	mbanalysis_detector1 = widget_button(mbview, value='data/rawdata1', sensitive=1)
	mbanalysis_zoom50 = widget_button(mbview, value='Zoom 50%', sensitive=1, /separator)
	mbanalysis_zoom100 = widget_button(mbview, value='Zoom 100%', sensitive=1)
	mbanalysis_zoom150 = widget_button(mbview, value='Zoom 150%', sensitive=1)
	mbanalysis_zoom200 = widget_button(mbview, value='Zoom 200%', sensitive=1)
	mbanalysis_localzoom = widget_button(mbview, value='Cursor zoom in new window')
	widget_control, mbanalysis_resolution1, set_button=0
	widget_control, mbanalysis_resolution2, set_button=0


	;; Create action buttons
	base2 = widget_base(base, /ROW)
	;;button_newdir = widget_button(base2, value='Change directory')
	button_updatefiles = widget_button(base2, value='Refresh files')
	button_updateimage = widget_button(base2, value='Refresh image')
	button_previous = widget_button(base2, value='Previous')
	button_next = widget_button(base2, value='Next')
	button_auto = widget_button(base2, value='Play')
	button_random = widget_button(base2, value='Random')
	button_shuffle = widget_button(base2, value='Shuffle')
	status_label = widget_label(base2,value='No files selected', /align_left,/dynamic_resize)


	;;
	;;	Create scroling window
	;;
	scroll = widget_draw(base, xsize=s[0], ysize=s[1], /scroll,$
		x_scroll_size=xview, y_scroll_size=yview, uvalue='SLIDE_IMAGE', $
		expose_events=doEvents, viewport_events=doEvents, /button_events, /motion_events)
	WIDGET_CONTROL, /REAL, base
	WIDGET_CONTROL, get_value=SLIDE_WINDOW, scroll

	;;
	;;	Information on geometry
	;;
		base_geometry = WIDGET_INFO(base, /geometry)
		draw_geometry = WIDGET_INFO(scroll, /geometry)
		draw_xpadding = base_geometry.scr_xsize - draw_geometry.scr_xsize
		draw_ypadding = base_geometry.scr_ysize - draw_geometry.scr_ysize
		draw_padding = {x : draw_xpadding, y:draw_ypadding}


	;;
	;;	Info structure
		sState = {data : data, $
				  image: image, $
				  currentFile : filename, $
				  currentFrameNum : 0L, $
				  h5field : "data/data", $
				  file_type : file_type, $
				  image_gamma : 1.0, $
				  image_boost : 1., $
				  image_max : 16000., $
				  image_zoom : 1.0, $
				  image_histclip : 0.0001, $
				  image_size: size(image,/dim), $
				  use_pixmap : pixmap, $
				  pixmap_x : pixmap_x, $
				  pixmap_y : pixmap_y, $
				  pixmap_dx : pixmap_dx, $
				  scroll : scroll, $
				  pfile : ptr_new(), $
				  index : index, $
				  dir : dir, $
				  autoShuffle : 0, $
				  autoNext : 0, $
				  circleHDF5Peaks : 0, $
				  resolutionRings1 : 0, $
				  resolutionRings2 : 0, $
				  findPeaks : 0, $
				  savePeaks : 0, $
				  centeredPeaks : centeredPeaks, $
				  savedir: savedir, $
				  slideWin: SLIDE_WINDOW, $
				  xvisible: xview, $
				  yvisible: yview, $
				  padding : draw_padding, $
				  base : base, $
				  
				  Colours : mbcolours, $
				  ColourList : mbcolours_1, $
				  ColourListID : mbcolours_IDs, $
				  colour_table : 41, $

				  menu_geom : mbfile_geom, $
				  menu_save : mbfile_si, $
				  menu_savedata : mbfile_sd, $
				  menu_savedataassembled : mbfile_sda, $
				  menu_random : mbfile_random, $
				  menu_newdir : mbfile_newdir, $ 
				  menu_updtefiles : mbfile_updatefilelist, $
				  menu_display: mbanalysis_imagescaling, $
				  menu_peakfinding : mbanalysis_peakfinding, $
				  menu_profiles : mbanalysis_profiles, $
				  menu_ROIstats : mbanalysis_ROI, $
				  menu_refresh : mbanalysis_refresh, $
				  menu_circleHDF5Peaks : mbanalysis_circleHDF5Peaks, $
				  menu_centeredPeaks : mbanalysis_centeredPeaks, $
				  menu_findPeaks :  mbanalysis_findPeaks, $
				  menu_localBackground : mbanalysis_displayLocalBackground, $
				  menu_savePeaks : mbanalysis_savePeaks, $
				  menu_cdiDefaults : mbanalysis_cdidefaults, $
				  menu_crystDefaults : mbanalysis_crystdefaults, $
				  menu_processall : mbanalysis_processall, $
				  menu_localzoom : 	mbanalysis_localzoom, $
				  menu_resolution1 :  mbanalysis_resolution1, $
				  menu_resolution2 :  mbanalysis_resolution2, $
				  menu_zoom50 : 	mbanalysis_zoom50, $
				  menu_zoom100 : mbanalysis_zoom100, $
				  menu_zoom150 : mbanalysis_zoom150, $
				  menu_zoom200 : mbanalysis_zoom200, $
				  menu_datadata : mbanalysis_datadata, $
				  menu_detector0 : mbanalysis_detector0, $
				  menu_detector1 : mbanalysis_detector1, $

				  peaks_localbackground : 0, $
				  peaks_algorithm : 0, $
				  peaks_ADC : 50, $
				  peaks_minpix : 2, $
				  peaks_maxpix : 20, $
				  peaks_minsnr : 8., $
				  peaks_minres : 0, $
				  peaks_maxres : 1300, $

				  display_localbackground : 0, $
				  
				  button_updatefiles : button_updatefiles, $
				  button_refresh : button_updateimage, $
				  button_previous : button_previous, $
				  button_next : button_next, $
				  button_random : button_random, $
				  button_auto : button_auto, $
				  button_shuffle : button_shuffle, $
				  status_label : status_label $
				 }
		sState.pfile = ptr_new(file)
		pstate = ptr_new(sState)
		WIDGET_CONTROL, base, SET_UVALUE=pState

 	;;
 	;;	Set up window
 	;;
 		device, retain=3, decomposed=0
 	
 		wset, slide_window
 		loadct, 41, /silent
 		;loadct, 4, /silent
	    ;tvscl, image
		WIDGET_CONTROL, scroll, SET_DRAW_VIEW=[(s[0]-xview)/2, (s[0]-yview)/2]
		
		if oldwin ne -1 then $
			wset, oldwin

    	XMANAGER, 'cheetah', base, event='cheetah_event', /NO_BLOCK
    	
		thisfile = (*pState).currentFrameNum+1
		numfiles = n_elements(*((*pstate).pfile))
		str = strcompress(string('(image',thisfile,' of ',numfiles,')'))
		widget_control, (*pState).status_label, set_value=str
		cheetah_displayImage, pState

end
