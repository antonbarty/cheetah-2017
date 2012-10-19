;;
;;	FEL_Browser
;;	Tool for snooping through FEL diffraction images
;;
;;	Anton Barty, 2007-2008
;;

function fel_randomimage_peakcircles, filename, image, pState

	;; Image of overlaid circles
	d = size(image,/dim)
	peakcircles = fltarr(d[0],d[1])
	
	;; Circle template
	r = dist(20,20)
	r = shift(r,10,10)
	circle = r ge 5 AND r lt 6

	;; Read peak info
	peakinfo = read_h5(filename, field='processing/hitfinder/peakinfo-raw')
	peakx = long(reform(peakinfo(0,*)))
	peaky = long(reform(peakinfo(1,*)))
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


pro fel_randomimage_displayImage, filename, pState, image

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
		
		
		;; Apply pixel map
		if (*pstate).use_pixmap then begin
			image = pixelRemap(data, (*pstate).pixmap_x, (*pstate).pixmap_y, (*pstate).pixmap_dx)
		endif $
		else $
			image = data

		image = (image>0)
		image = image < (*pState).image_max

		if (*pState).circlePeaks then begin
			circles = fel_randomimage_peakcircles(filename, image, pState)
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

		;; Turn off error handler
		catch, /cancel

end


pro fel_randomimage_event, ev

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
			fel_randomimage_displayImage, sState.currentFile, pState, image

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
			fel_randomimage_displayImage, filename, pState

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
			fel_randomimage_displayImage, filename, pState
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
			fel_randomimage_displayImage, filename, pState
			
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

			fel_randomimage_displayImage, file[0], pState
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


			fel_randomimage_displayImage, file[(*pState).currentFileNum], pState

		end
		
		;;
		;;	Circle peaks?
		;;
		sState.menu_circlePeaks : begin
			(*pstate).circlePeaks = 1-sState.circlePeaks
			widget_control, sState.menu_circlePeaks, set_button = (*pstate).circlePeaks
		end
		sState.menu_centeredPeaks : begin
			(*pstate).centeredPeaks = 1-sState.centeredPeaks
			widget_control, sState.menu_centeredPeaks, set_button = (*pstate).centeredPeaks
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
			WSET, sState.slideWin
			tvscl, sState.image
		end
		
		sState.button_refresh : begin
			WSET, sState.slideWin
			tvscl, sState.image
		end


		;;
		;; Change image display parameters
		;;
		sState.menu_display : begin
			desc = [ 	'1, base, , column', $
						'0, float, '+string(sState.image_gamma)+', label_left=Gamma:, width=10, tag=image_gamma', $
						'0, float, '+string(sState.image_boost)+', label_left=Boost:, width=10, tag=image_boost', $
						'0, float, '+string(sState.image_max)+', label_left=Max value:, width=10, tag=image_max', $
						'2, text, '+string(sState.h5field)+', label_left=HDF5 field:, width=10, tag=h5field', $
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

			file = *sState.pfile
			i = sState.currentFileNum
			filename = file[i]
			fel_randomimage_displayImage, filename, pState
		
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

			file = *sState.pfile
			i = sState.currentFileNum
			filename = file[i]
			fel_randomimage_displayImage, filename, pState


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

			file = *sState.pfile
			i = sState.currentFileNum
			filename = file[i]
			fel_randomimage_displayImage, filename, pState

		end




		else: begin
		end
		
	endcase  
	
	catch, /cancel
end




pro fel_randomimage, pixmap=pixmap

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
	modifyct, 41, 'Inverse B-W', invbw[*,0], invbw[*,1], invbw[*,2]

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
	mbanalysis_imagescaling = widget_button(mbanalysis, value='Image display')
	mbanalysis_circlePeaks = widget_button(mbanalysis, value='Circle peaks', /checked)
	mbanalysis_centeredPeaks = widget_button(mbanalysis, value='Peaks relative to image centre', /checked)
	mbanalysis_profiles = widget_button(mbanalysis, value='Profiles')
	mbanalysis_ROI = widget_button(mbanalysis, value='ROI statistics')
	mbanalysis_refresh = widget_button(mbanalysis, value='Refresh image')


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
				  image_gamma : 0.25, $
				  image_boost : 1., $
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
				  circlePeaks : 0, $
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
				  menu_profiles : mbanalysis_profiles, $
				  menu_ROIstats : mbanalysis_ROI, $
				  menu_refresh : mbanalysis_refresh, $
				  menu_circlePeaks : mbanalysis_circlePeaks, $
				  menu_centeredPeaks : mbanalysis_centeredPeaks, $
				  
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
	    tvscl, image
		WIDGET_CONTROL, scroll, SET_DRAW_VIEW=[(s[0]-xview)/2, (s[0]-yview)/2]
		
		if oldwin ne -1 then $
			wset, oldwin

    	XMANAGER, 'fel_randomimage', base, event='fel_randomimage_event', /NO_BLOCK
    	
		thisfile = (*pState).currentFileNum+1
		numfiles = n_elements(*((*pstate).pfile))
		str = strcompress(string('(image',thisfile,' of ',numfiles,')'))
		widget_control, (*pState).status_label, set_value=str


end
