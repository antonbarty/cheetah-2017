;;
;;	FEL_Browser
;;	Tool for snooping through FEL diffraction images
;;
;;	Anton Barty, 2007-2008
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
		temp = peakcircles[peakx[i]:peakx[i]+19, peaky[i]:peaky[i]+19]
		temp += circle
		peakcircles[peakx[i]:peakx[i]+19, peaky[i]:peaky[i]+19] = temp
	endfor
	
	peakcircles = peakcircles < 1
	
	return, peakcircles	

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


function cheetah_findpeaks, data, pState

	lbg = (*pstate).peaks_localbackground 
	adc_thresh = (*pstate).peaks_ADC
	minpix = (*pstate).peaks_minpix
	maxpix = (*pstate).peaks_maxpix
	peaks_minsnr = (*pstate).peaks_minsnr 
	peaks_minres = (*pstate).peaks_minres
	peaks_maxres = (*pstate).peaks_maxres


	;; Array for peak information
	maxpeaks = 5000
	peakx = fltarr(maxpeaks)
	peaky = fltarr(maxpeaks)
	peaktotal = fltarr(maxpeaks)
	peakpix = fltarr(maxpeaks)
	peakcounter = 0
	
	s = size(data, /dim)

	;; Subtract local background here
	;; That this has already been done if (*pstate).display_localbackground eq 1!!!
	m = 0
	if (*pstate).display_localbackground ne 1 then begin
		m = cheetah_localbackground(data, lbg) 
	endif
	temp = data
	temp -= m	

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


pro cheetah_displayImage, pState, image

		;; Retrieve file info
		file = *(*pState).pfile
		i = (*pState).currentFileNum
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

		data = read_h5(filename, field=(*pstate).h5field)
		title = file_basename(filename)
		data = float(data)
		
		;; Apply local background to display image?
		if (*pstate).display_localbackground eq 1 then begin
				lbg = (*pstate).peaks_localbackground 
				m = cheetah_localbackground(data, lbg) 
				data -= m
		endif
		

		;; Find or load peaks
		if (*pState).circleHDF5Peaks then begin
			peakinfo = read_h5(filename, field='processing/hitfinder/peakinfo')
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

		
		;; Display image
		(*pState).data = data
		;(*pState).image = image
		widget_control, (*pState).base, base_set_title=title
		WSET, (*pState).slideWin
		loadct, (*pstate).colour_table, /silent
		;tvscl, (*pState).image
		tvscl, image

		;; Widget_control
		thisfile = (*pState).currentFileNum+1
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



pro cheetah_event, ev

  	WIDGET_CONTROL, ev.top, GET_UVALUE=pState
  	sState = *pState
	img_size = size(sState.image,/dim)
	
	
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
			i = sState.currentFileNum+1
			if(i ge n_elements(file)) then $
				i = 0				
			filename = file[i]

			;; Display it
			(*pState).currentFile = filename
			(*pState).currentFileNum = i
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
			i = sState.currentFileNum-1
			if(i lt 0) then $
				i = n_elements(file)-1				
			filename = file[i]

			;; Display it
			(*pState).currentFile = filename
			(*pState).currentFileNum = i
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
			(*pState).currentFileNum = i
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
			newfile = file_search(newdir,"LCLS*.h5",/fully_qualify)
			
			if n_elements(newfile) eq 0 then begin
				message,'No files found in directory: '+newdir, /info
				return
			endif
			file = newfile
			
			if(ptr_valid((*pState).pfile)) then $
				ptr_free, (*pState).pfile
			(*pState).pfile = ptr_new(newfile,/no_copy)
			(*pState).dir = newdir
			(*pState).currentFileNum = 0

			cheetah_displayImage, pState
		end
				
		
		;;
		;;	Refresh file list
		;;
		sState.button_updatefiles : begin
			newfile = file_search(sState.dir,"LCLS*.h5",/fully_qualify)
			
			if n_elements(newfile) eq 0 then begin
				message,'No files found in directory: '+sState.dir, /info
				return
			endif
			file = newfile
			
			if(ptr_valid((*pState).pfile)) then $
				ptr_free, (*pState).pfile
			(*pState).pfile = ptr_new(newfile,/no_copy)
			(*pState).currentFileNum = n_elements(file)-1


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
			i = sState.currentFileNum
			filename = file[i]
		end
		sState.menu_findPeaks : begin
			(*pstate).findPeaks = 1-sState.findPeaks
			(*pstate).circleHDF5Peaks = 0
			widget_control, sState.menu_findPeaks, set_button = (*pstate).findPeaks
			widget_control, sState.menu_circleHDF5Peaks, set_button = 0
			file = *sState.pfile
			i = sState.currentFileNum
			filename = file[i]
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
		;;	Profiles
		;;
		sState.menu_profiles : begin
			WSET, sState.slideWin
			profiles, sState.data
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
					(*pState).currentFileNum = i
					cheetah_displayImage, pState
				endfor
			endif
		end


		;;
		;; Change image display parameters
		;;
		sState.menu_display : begin
			desc = [ 	'1, base, , column', $
						'0, float, '+string(sState.image_gamma)+', label_left=Gamma:, width=10, tag=image_gamma', $
						'0, float, '+string(sState.image_boost)+', label_left=Boost:, width=10, tag=image_boost', $
						'0, float, '+string(sState.image_max)+', label_left=Max value:, width=10, tag=image_max', $
						'2, text, '+string(sState.h5field)+', label_left=HDF5 field:, width=50, tag=h5field', $
						'1, base,, row', $
						'0, button, OK, Quit, Tag=OK', $
						'2, button, Cancel, Quit' $
			]		
			a = cw_form(desc, /column, title='Image display')
			
			if a.OK eq 1 then begin		
				(*pstate).image_max = a.image_max
				(*pstate).image_gamma = a.image_gamma
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


		sState.menu_cdiDefaults : begin
			loadct, 4, /silent		
			(*pstate).colour_table = 4
			(*pstate).image_gamma = 0.25
			(*pstate).image_boost = 2
			cheetah_displayImage, pState
		end


		sState.menu_crystDefaults : begin
			loadct, 41, /silent		
			(*pstate).colour_table = 41
			(*pstate).image_gamma = 1
			(*pstate).image_boost = 5
			cheetah_displayImage, pState
		end
		
		

		else: begin
		end
		
	endcase  
	
	catch, /cancel
end




pro cheetahview, pixmap=pixmap

	;;	Select data directory
	dir = dialog_pickfile(/directory, title='Select data directory')
	file = file_search(dir,"LCLS*.h5",/fully_qualify)
	savedir=dir
	
	if n_elements(file) eq 0 OR file[0] eq '' then begin
		message,'No files found in directory: '+dir, /info
		return
	endif


	;; Using a pixel map?
	;pixmap_file = dialog_pickfile(filter='*.h5', title='Select detector geometry file')
	;if pixmap_file eq '' then begin
	;	pixmap = 0
	;	pixmap_x = 0
	;	pixmap_y = 0
	;	pixmap_dx = 0
	;endif $
	;else begin
	;	pixmap = 1
	;	pixmap_x = read_h5(pixmap_file,field='x')
	;	pixmap_y = read_h5(pixmap_file,field='y')
	;	pixmap_dx = 110e-6		;; cspad
	;endelse

	
	;; Sample data
	filename = file[0]
	data = read_h5(filename)
	
	;; Set default pixel map (none)
	s = size(data, /dim)
	pixmap = 0
	pixmap_x = fltarr(s[0],s[1])
	pixmap_y = fltarr(s[0],s[1])
	pixmap_dx = 110e-6


	if keyword_set(pixmap) then $
		data = pixelRemap(data, pixmap_x, pixmap_y, pixmap_dx)

	
	image = (data>0)
	image = image < 1000

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
	mbanalysis_centeredPeaks = widget_button(mbcryst, value='Peaks relative to image centre', /checked)
	mbanalysis_displayLocalBackground = widget_button(mbcryst, value='Display with local background subtraction', /checked)


	mbview = widget_button(bar, value='View')
	mbanalysis_imagescaling = widget_button(mbview, value='Image display settings')
	mbanalysis_zoom50 = widget_button(mbview, value='Zoom 50%', sensitive=0, /separator)
	mbanalysis_zoom100 = widget_button(mbview, value='Zoom 100%', sensitive=0)
	mbanalysis_zoom150 = widget_button(mbview, value='Zoom 150%', sensitive=0)
	mbanalysis_zoom200 = widget_button(mbview, value='Zoom 200%', sensitive=0)


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
				  currentFileNum : 0L, $
				  h5field : "data/data", $
				  image_gamma : 1.0, $
				  image_boost : 5., $
				  image_max : 32000., $
				  use_pixmap : pixmap, $
				  pixmap_x : pixmap_x, $
				  pixmap_y : pixmap_y, $
				  pixmap_dx : pixmap_dx, $
				  scroll : scroll, $
				  pfile : ptr_new(), $
				  dir : dir, $
				  autoShuffle : 0, $
				  autoNext : 0, $
				  circleHDF5Peaks : 0, $
				  findPeaks : 0, $
				  savePeaks : 0, $
				  centeredPeaks : 0, $
				  savedir: savedir, $
				  slideWin: SLIDE_WINDOW, $
				  xvisible: xview, $
				  yvisible: yview, $
				  padding : draw_padding, $
				  base : base, $
				  
				  Colours : mbcolours, $
				  ColourList : mbcolours_1, $
				  ColourListID : mbcolours_IDs, $
				  colour_table : 4, $

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
				  

				  peaks_localbackground : 2, $
				  peaks_ADC : 300, $
				  peaks_minpix : 3, $
				  peaks_maxpix : 20, $
				  peaks_minsnr : 0., $
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
 		loadct, 4, /silent
	    tvscl, image
		WIDGET_CONTROL, scroll, SET_DRAW_VIEW=[(s[0]-xview)/2, (s[0]-yview)/2]
		
		if oldwin ne -1 then $
			wset, oldwin

    	XMANAGER, 'cheetah', base, event='cheetah_event', /NO_BLOCK
    	
		thisfile = (*pState).currentFileNum+1
		numfiles = n_elements(*((*pstate).pfile))
		str = strcompress(string('(image',thisfile,' of ',numfiles,')'))
		widget_control, (*pState).status_label, set_value=str


end
