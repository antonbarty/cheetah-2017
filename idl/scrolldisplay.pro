;;
;;	FEL_Browser
;;	Tool for snooping through FEL diffraction images
;;
;;	Anton Barty, 2007-2008
;;



pro scrolldisplay_event, ev

  	WIDGET_CONTROL, ev.top, GET_UVALUE=sState
	img_size = size(sState.image,/dim)
	
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
				tvscl, sState.image
			endif
	
			if ev.type eq 4 then begin ; Expose event
				WSET, sState.slideWin
				tvscl, sState.image
			endif
		end

		;;
		;;	Save image
		;;
		sState.menu_save : begin

			filename = dialog_pickfile(filter='*.png', /write)
			;filename = dialog_pickfile(filter='*.tif', /write)
			if (filename eq '') then $
				return
			idl_write_png,filename, sState.image
			;idl_write_tiff,filename, sState.image
		end


		;;
		;;	Save data
		;;
		sState.menu_savedata : begin
			filename = dialog_pickfile(filter='*.h5', /write)
			if (filename eq '') then $
				return
			write_h5,filename, sState.data
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

		
		else: begin
		end
		
	endcase  
	
end







pro scrolldisplay, image, data, title=title, geometry=geometry

	if(n_elements(TITLE) eq 0) THEN TITLE='Image'
	oldwin = !d.window

	
	;; 
	;;	Data array?
	;;
		if (n_elements(data) eq 0) OR (n_elements(data) ne n_elements(image)) then begin
			data = image
		endif


	;;
	;;	Apply geometry?
	;;
	if keyword_set(geometry) then begin
		if geometry ne '' then begin
			 pixmap_x = read_h5(geometry,field='x')
			 pixmap_y = read_h5(geometry,field='y')
			 pixmap_dx = 110e-6		;; cspad
			 image = pixelRemap(image, pixmap_x, pixmap_y, pixmap_dx)
		endif
	endif



	;;
	;;	Base widget
	;;
		base = WIDGET_BASE(title=title, GROUP=GROUP, mbar=bar, /ROW, /TLB_SIZE_EVENTS)
		WIDGET_CONTROL, /MANAGED, base

	;;
	;;	Size to make image
	;;
		s = size(image, /dim)
		screensize = get_screen_size()
		screensize -= 100
		xview = min([screensize[0],s[0]])
		yview = min([screensize[1],s[1]])

	;;
	;;	Create save menu
	;;
		mbfile = widget_button(bar, value='File')
		mbfile_si = widget_button(mbfile, value='Save image')
		mbfile_sd = widget_button(mbfile, value='Save data')

		mbanalysis = widget_button(bar, value='Analysis')
		mbanalysis_profiles = widget_button(mbanalysis, value='Profiles')
		mbanalysis_ROI = widget_button(mbanalysis, value='ROI statistics')
		mbanalysis_refresh = widget_button(mbanalysis, value='Refresh image')


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
		sState = {image: image, $
				  data : data, $
				  scroll : scroll, $
				  slideWin: SLIDE_WINDOW, $
				  xvisible: xview, $
				  yvisible: yview, $
				  padding : draw_padding, $
				  menu_save : mbfile_si, $
				  menu_savedata : mbfile_sd, $
				  menu_profiles : mbanalysis_profiles, $
				  menu_ROIstats : mbanalysis_ROI, $
				  menu_refresh : mbanalysis_refresh $
				 }
		WIDGET_CONTROL, base, SET_UVALUE=sState

 	;;
 	;;	Set up window
 	;;
 		wset, slide_window
	    tvscl, image
		WIDGET_CONTROL, scroll, SET_DRAW_VIEW=[(s[0]-xview)/2, (s[0]-yview)/2]
		
		if oldwin ne -1 then $
			wset, oldwin

    	XMANAGER, 'scrolldisplay', base, event='scrolldisplay_event', /NO_BLOCK

end
