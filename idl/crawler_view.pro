;
; crawler_autorun
; Anton Barty, 2013
;
; Monitors a given set of HDF5 directories and updates a text file accordingly 
; Could be implemented in sh, or perl, or python
;


;; 
;;	Set up a few locations and things
;;  Quick and dirty: ultimately should be read from a config file
;;
pro crawler_config, pState
	
	;; First set some sensible defaults
	(*pstate).xtcdir = '../../xtc'
	(*pstate).h5dir = '../hdf5'
	(*pstate).h5filter = 'r*-ab' 
	(*pstate).geometry = '../config/pixelmap/2may13-v2.h5'
	(*pstate).process = '../anton/process/process'
	(*pstate).cheetahIni = '2dx.ini'


	;; Now try to read from the config file
	configFile = 'crawler.config'
	if file_test(configFile) eq 1 then begin
		info = read_csv(configFile)
		info = info.field1		
		if n_elements(info) eq 6 then begin
			(*pstate).xtcdir = info[0]
			(*pstate).h5dir = info[1]
			(*pstate).h5filter = info[2]
			(*pstate).geometry = info[3]
			(*pstate).process = info[4]
			(*pstate).cheetahIni = info[5]
		endif $
		
		else begin
			crawler_configMenu, pState
		endelse		
	endif $
	
	else begin
		crawler_configMenu, pState
	endelse
	

	print, 'XTC directory: ', (*pState).xtcdir
	print, 'HDF5 directory: ', (*pState).h5dir
	print, 'HDF5 run filter: ', (*pState).h5filter
	print, 'Process script: ', (*pState).process
	print, 'Geometry file: ', (*pState).geometry
	print, 'Cheetah .ini: ', (*pstate).cheetahIni

end


pro crawler_configMenu, pState

	desc = [ 	'1, base, , column', $
					'0, text, '+(*pstate).xtcdir+', label_left=XTC directory:, width=100, tag=xtcdir', $
					'0, text, '+(*pstate).h5dir+', label_left=HDF5 directory:, width=100, tag=h5dir', $
					'0, text, '+(*pstate).h5filter+', label_left=HDF5 filter:, width=100, tag=h5filter', $
					'0, text, '+(*pstate).geometry+', label_left=Geometry file:, width=100, tag=geometry', $
					'0, text, '+(*pstate).process+', label_left=Process script:, width=100, tag=process', $
					'0, text, '+(*pstate).cheetahIni+', label_left=Default cheetah.ini:, width=100, tag=cheetahIni', $
					'1, base,, row', $
					'0, button, OK, Quit, Tag=OK', $
					'2, button, Cancel, Quit' $
	]		
	a = cw_form(desc, /column, title='Start cheetah')
	
	;; Only do this if OK is pressed (!!)
	if a.OK eq 1 then begin		
			(*pstate).xtcdir = a.xtcdir
			(*pstate).h5dir = a.h5dir
			(*pstate).h5filter = a.h5filter
			(*pstate).geometry = a.geometry
			(*pstate).process = a.process
			(*pstate).cheetahIni = a.cheetahIni
			
			;; Save back out to file
			openw, lun, 'crawler.config', /get
			printf, lun, (*pstate).xtcdir
			printf, lun, (*pstate).h5dir
			printf, lun, (*pstate).h5filter
			printf, lun, (*pstate).geometry
			printf, lun, (*pstate).process
			printf, lun, (*pstate).cheetahIni
			close, lun
			free_lun, lun
	endif

end


;;
;;	Display a single HDF5 file (eg: virtual powder)
;;
pro crawler_displayfile, filename
	
	if filename eq '' then begin
		print,'File does not exist'
		return
	endif
	
	print,'Displaying: ', filename
	data = read_h5(filename)
	img = histogram_clip(data, 0.005)
	img = img > 0
	loadct, 4
	scrolldisplay, img, title=file_basename(filename)

end

;;
;;	Dialog to start Cheetah
;;
pro crawler_startCheetah, pState, run

  	sState = *pState
	table_data = *(sState.table_pdata)
	table_runs = reform(table_data[0,*])

	help, run
	print, run

	nruns = n_elements(run)-1
	startrun  = run[0]
	endrun = run[nruns]
	cheetah = sState.process
	ini = sState.cheetahIni
	
	desc = [ 	'1, base, , column', $
				'0, label, Start run: '+startrun+', left', $
				'0, label, End run: '+endrun+', left', $
				'0, label, Command: '+cheetah+', left', $
				'2, text, '+ini+', label_left=cheetah.ini file:, width=50, tag=ini', $
				'1, base,, row', $
				'0, button, OK, Quit, Tag=OK', $
				'2, button, Cancel, Quit' $
	]		
	a = cw_form(desc, /column, title='Start cheetah')
	
	;; Only do this if OK is pressed (!!)
	if a.OK eq 1 then begin		
		(*pstate).cheetahIni = a.ini
		
		for i=0, nruns do begin
			cmnd = strcompress(string(cheetah, ' ', run[i], ' ', ini))
			print, cmnd
			spawn, cmnd
			
			;; This is simply for eye candy - swap the Cheetah status label to 'Submitted'
			w = where(table_runs eq run[i])
			if w[0] ne -1 then $
				widget_control, sState.table, use_table_select = [3, w[0], 3, w[0]], set_value = ['Submitted']
		endfor

	endif

end



;;
;;	Update and populate the table
;;
pro crawler_updateTable, pState

	print,'Refreshing table'

  	sState = *pState
	table = sState.table
	
	;screensize = get_screen_size()
	;xview = screensize[0] - 140
	;yview = screensize[1] - 140


	;; Read crawler file
	if file_test('crawler.txt') eq 0 then begin
		print,'Crawler file does not exist'
		return
	endif
	data = read_csv('crawler.txt',header=h)

	ntags = n_tags(data)
	names = tag_names(data)
	
	ncols = ntags
	nrows = n_elements(data.(0))
	table_data= strarr(ncols, nrows)

	for i=0, ncols-1 do begin
		a = string(data.(i))
		a = strcompress(a)
		table_data[i,*] = a
	endfor


	;; Update the table	
	widget_control, table, table_xsize = ncols
	widget_control, table, table_ysize = nrows
	widget_control, table, column_labels=h
	widget_control, table, row_labels = ''
	widget_control, table, set_value = table_data
	widget_control, table, set_table_select = [-1, nrows-1, -1, nrows-1]


	;; Column widths
	colwidth = strlen(h)+3
	cwidth = colwidth * !d.x_ch_size + 6
	widget_control, table, column_widths = cwidth



	;; Remember table data for later
	if ptr_valid((*pState).table_pdata) then $
		ptr_free, (*pState).table_pdata
	(*pState).table_pdata = ptr_new(table_data,/no_copy)


end

;;
;; This routine ends up being called quite often...
;;
function crawler_whichRun, pState, runname=runname, dirname=dirname, pathname=pathname, multiple=multiple
  	sState = *pState

	selection = widget_info(sState.table, /table_select)
	rowstart = selection[1]
	rowend = selection[3]
	if not keyword_set(multiple) then begin
		if rowstart ne rowend then begin
			rowend = rowstart
			widget_control, sState.table, set_table_select=[-1, rowstart, -1, rowend]
		endif
	endif
	row = rowstart + indgen(rowend-rowstart+1)
	
	print, 'Selected rows: ', rowstart, rowend
	table_data = *(sState.table_pdata)
	run = reform(table_data[0,rowstart:rowend])
	rundir = reform(table_data[4,rowstart:rowend])
	
	
	;; Old way when we only knew the run number
	;h5filter = sState.h5filter
	;part = strsplit(h5filter, '*', /extract)
	;dir = string(format='(%"%s%04i%s")', part[0], fix(run), part[1])
	;path = sState.h5dir+'/'+dir
	;print, path
	
	;; New way now that we record run dirs
	dir = rundir
	path = sState.h5dir+'/'+dir
	
	;; Which format output do we need?
	result = row
	if keyword_set(runname) then begin
		result = run
	endif $
	else if keyword_set(dirname) then begin
		result = dir
	endif $
	else if keyword_set(pathname) then begin
		result = path
	endif 
	
	print, result
	return, result

end

;;
;;	Crawler event processor
;;
pro crawler_event, ev
  	WIDGET_CONTROL, ev.top, GET_UVALUE=pState
  	sState = *pState

	;;
	;; Main event processing case statement
	;;
	case ev.id of 
		
		;;
		;;	Change in screensize
		;;
		ev.top : begin
			screensize = get_screen_size()
			xview = screensize[0] - 140
			yview = screensize[1] - 140
			min_xsize = sState.table_scrwidth
			min_ysize = 100

			new_xsize = ev.x
			new_ysize = ev.y			
			new_xsize = (new_xsize lt min_xsize) ? min_xsize : new_xsize
			new_ysize = (new_ysize lt min_ysize) ? min_ysize : new_ysize
			new_xsize = (new_xsize gt xview) ? xview : new_xsize
			new_ysize = (new_ysize gt yview) ? yview : new_ysize
	
			;;widget_control, sState.table, scr_xsize = new_xsize, scr_ysize=ev.y-100
			widget_control, sState.base, scr_ysize = ev.y
			widget_control, sState.table, scr_xsize = new_xsize
			widget_control, sState.table, scr_ysize = ev.y - 100

		end
		
		;;
		;;	Refresh button
		;;
		sState.button_refresh : begin
			crawler_updateTable, pState
			if sState.table_autorefresh ne 0 then begin
					;print,'Auto refreshing table'
					widget_control, sState.button_refresh,  timer=sState.table_autorefresh
			endif
		end
				
				
		sState.button_viewtype : begin
		
		end

		;;
		;;	Launch Cheetah
		;;
		sState.button_cheetah : begin
			run = crawler_whichRun(pstate, /run, /multiple)
			crawler_startCheetah, pState, run
		end			

		;; View hits
		sState.button_hits : begin
			dir = crawler_whichRun(pstate, /path)
			print, 'Launching cheetahview, dir=', dir, ', geometry= ', sState.geometry
			cheetahview, dir=dir, geometry=sState.geometry
		end

		;; View hitrate graph
		sState.button_hitrate : begin
			dir = crawler_whichRun(pstate, /path)
			hitrate, dir		
		end


		;; View resolution
		sState.button_resolution : begin
			dir = crawler_whichRun(pstate, /path)
			resolution, dir
		end


		;; View powder patterns
		sState.button_powder : begin
			dir = crawler_whichRun(pstate, /path)
			f = file_search(dir,'*detector0-class1-sum.h5')
			crawler_displayfile, f[0]
		end


		;;
		;; Menu bar items duplicate buttons
		;;
		sState.mbfile_refresh : begin
			crawler_updateTable, pState
		end
		sState.mbfile_configure : begin
			crawler_configMenu, pState
		end
		sState.mbfile_crawl : begin
			crawler_autorun, xtcdir=(*pState).xtcdir, hdf5dir=(*pState).h5dir, hdf5filter=(*pState).h5filter
		end
		sState.mbcheetah_run : begin
			run = crawler_whichRun(pstate, /run, /multiple)
			crawler_startCheetah, pState, run
		end
		sState.mbview_images : begin
			dir = crawler_whichRun(pstate, /path)
			print, 'Launching cheetahview, dir=', dir, ', geometry= ', sState.geometry
			cheetahview, dir=dir, geometry=sState.geometry
		end
		sState.mbview_hitrate : begin
			dir = crawler_whichRun(pstate, /path)
			hitrate, dir				
		end
		sState.mbview_resolution : begin 
			dir = crawler_whichRun(pstate, /path)
			resolution, dir
		end
		sState.mbview_powder : begin
			dir = crawler_whichRun(pstate, /path)
			f = file_search(dir,'*detector0-class1-sum.h5')
			crawler_displayfile, f[0]
		end
		sState.mbview_powderdark : begin
			dir = crawler_whichRun(pstate, /path)
			f = file_search(dir,'*detector0-class0-sum.h5')
			crawler_displayfile, f[0]
		end


		sState.mbfile_autorefresh : begin
			;;help, sState.table_autorefresh
			desc = [ 	'1, base, , column', $
						'2, FLOAT, '+ string(sState.table_autorefresh)+', label_left=Auto refresh delay (sec):, width=10, tag=delay', $
						'1, base,, row', $
						'0, button, OK, Quit, Tag=OK', $
						'2, button, Cancel, Quit' $
			]		
			a = cw_form(desc, /column, title='Start cheetah')
	
			;; Only do this if OK is pressed (!!)
			if a.OK eq 1 then begin		
				(*pstate).table_autorefresh = a.delay
			endif
		end



		
		else : begin 
			help, ev
		end
			
		
	endcase

end



pro crawler_view

	;;
	;;	Estimate the screen size
	;;
	screensize = get_screen_size()
	xview = screensize[0] - 140
	yview = screensize[1] - 140


	if file_test('crawler.txt') eq 1 then begin
		data = read_csv('crawler.txt',header=h)
		colwidth = strlen(h)+3
		cwidth = colwidth * !d.x_ch_size + 6
	endif $
	else begin
		data = strarr(5,5)
		h = strarr(5)
		colwidth = strlen(h)+3
		cwidth = colwidth * !d.x_ch_size + 6
	endelse
		

	base = widget_base(/column, group=group, mbar=bar, /tlb_size_events)
	widget_control, base, /managed

	;;
	;; Menu bar items
	;;
	mbfile = widget_button(bar, value='File')
	mbfile_configure = widget_button(mbfile, value='Configure')
	mbfile_crawl = widget_button(mbfile, value='Start crawler')
	mbfile_refresh = widget_button(mbfile, value='Refresh table')
	mbfile_autorefresh = widget_button(mbfile, value='Auto refresh table')
	mbfile_quit = widget_button(mbfile, value='Quit')

	mbfile = widget_button(bar, value='Cheetah')
	mbcheetah_run = widget_button(mbfile, value='Process selected runs')

	mbfile = widget_button(bar, value='View')
	mbview_images = widget_button(mbfile, value='View HDF5 files')
	mbview_hitrate = widget_button(mbfile, value='View hit rate plot')
	mbview_resolution = widget_button(mbfile, value='View resolution plot')
	mbview_powder = widget_button(mbfile, value='View virtual powder')
	mbview_powderdark = widget_button(mbfile, value='View virtual powder (dark)')


	;;
	;;	Quick action buttons
	;;
	base2 = widget_base(base, /row)
	button_refresh = widget_button(base2, value='Refresh')
	button_viewtype = widget_button(base2, value='Dataset view', sensitive=0)
	button_cheetah = widget_button(base2, value='Run Cheetah')
	button_hits = widget_button(base2, value='View hits')
	button_hitrate = widget_button(base2, value='Hitrate')
	button_resolution = widget_button(base2, value='Resolution')
	button_powder = widget_button(base2, value='Virtual powder')



	;;
	;;	Format the table
	;;
	table = widget_table(base, /resizeable_columns, /no_row_headers)
	scrwidth = total(cwidth)+30
	widget_control, table, column_widths = cwidth

	

	;;
	;;	Information on screen geometry
	;;
	base_geometry = WIDGET_INFO(base, /geometry)
	table_geometry = WIDGET_INFO(table, /geometry)
	xpadding = base_geometry.scr_xsize - table_geometry.scr_xsize
	ypadding = base_geometry.scr_ysize - table_geometry.scr_ysize
	draw_padding = {x : xpadding, y : ypadding}

	;;
	;;	Realise widget
	;;
	widget_control, base, base_set_title='Cheetah crawler view'
	widget_control, base, scr_ysize = yview
	widget_control, base, /realize

	;; 
	;;	Update table size
	;;
	widget_control, table, scr_xsize=total(cwidth)+30
	widget_control, table, scr_ysize=yview - 100


	;;
	;;	Info structure
	;;
	sState = { $
			base : base, $
			table : table, $
			table_pdata : ptr_new(),  $
			table_datasetview : 0, $
			table_scrwidth : scrwidth, $
			table_autorefresh : 60., $
			
			mbfile_configure : mbfile_configure, $
			mbfile_crawl : mbfile_crawl, $
			mbfile_refresh : mbfile_refresh, $
			mbfile_autorefresh : mbfile_autorefresh, $ 
			mbfile_quit : mbfile_quit, $
			mbcheetah_run : mbcheetah_run, $
			
			mbview_images : mbview_images, $
			mbview_hitrate : mbview_hitrate, $
			mbview_resolution : mbview_resolution, $
			mbview_powder : mbview_powder, $
			mbview_powderdark : mbview_powderdark, $
			
			button_refresh : button_refresh, $
			button_viewtype : button_viewtype, $
			button_cheetah : button_cheetah, $
			button_hits : button_hits, $
			button_hitrate : button_hitrate, $
			button_resolution : button_resolution, $
			button_powder : button_powder, $
			
			xtcdir : 'Not set', $
			h5dir : 'Not set', $
			h5filter : 'Not set', $
			process : 'Not set', $
			geometry : 'Not set', $
			cheetahIni : 'Not set' $
	}
	
	pstate = ptr_new(sState)
	WIDGET_CONTROL, base, SET_UVALUE=pState
	crawler_config, pState
	crawler_updateTable, pState

   	XMANAGER, 'Test', base, event='crawler_event', /NO_BLOCK

	
	
end
	
