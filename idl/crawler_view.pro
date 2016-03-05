;
; crawler_autorun
; Anton Barty, 2013
;
; Monitors a given set of HDF5 directories and updates a text file accordingly 
; Could be implemented in sh, or perl, or python
;

;;
;;	Pick working directory
;;
pro crawler_pickdir
		ifile = '.cheetah-crawler'

		cd,'~'
		
		if file_test(ifile) eq 1 then begin
			dirlist = read_csv(ifile)
			dirlist = dirlist.field1
			str = strjoin(dirlist,'|')
			desc = [ 	'1, base, , column', $
				'0, droplist, '+str+', label_left=Dataset:,  tag=selection', $
				'1, base,, row', $
				'0, button, Go to selected experiment, Quit, Tag=OK', $
				'0, button, Set up new experiment, Quit, Tag=Setup', $
				'0, button, Find a different experiment, Quit, Tag=Other', $
				'2, button, Cancel, Quit, tag=Cancel' $
			]		
			a = cw_form(desc, /column, title='Start cheetah')

			;; Only do this if OK is pressed (!!)
			if a.OK eq 1 then begin	
				dir = dirlist[a.selection]
			endif
			
			if a.Other eq 1 then begin
;				dir=dialog_pickfile(/dir)
				dir=dialog_pickfile(filter='crawler.config',/fix_filter, title='Please select crawler directory')
				dir = file_dirname(dir)
				dirlist = [dir, dirlist]			
			endif

			if a.Setup eq 1 then begin
				dir=dialog_pickfile(title='Select target directory', /dir)
				print, 'Target directory: ', dir
				print, dir
				crawler_autosetup	, dir
				dir += 'cheetah/gui'
				print, 'Directory added to list: ', dir
				dirlist = [dir, dirlist]			
			endif
			
			if a.Cancel eq 1 then begin
				exit
			endif
		endif $
		
		else begin
			dir=dialog_pickfile(filter='crawler.config',/fix_filter, title='Please select the crawler.config file')
			dir = file_dirname(dir)
			dirlist = [dir]
		endelse

		;; Write out the experiment list 
		openw, lun, ifile, /get
		printf, lun, transpose(dirlist)
		close, lun
		free_lun, lun

		;; Go to this directory
		cd,dir[0]
end


;; 
;;	Try to auto-detect a configuration
;; 	- If there is a .cheetah-crawler file in this directory, use it (normally ~/.cheetah-crawler)
;;	- If there is a crawler.config file, use it
;;  - Navigate to a directory with a crawler.config file
;; 	- Only then, launch the configuration tool
;;  Quick and dirty: ultimately should be read from a config file
;;
pro crawler_config, pState
	
	;; First set some sensible defaults
	(*pstate).xtcdir = '../../../xtc'
	(*pstate).h5dir = '../hdf5'
	(*pstate).h5filter = 'r*' 
	(*pstate).geometry = '../calib/geometry/cspad-front-12feb2013.h5'
	(*pstate).process = '../process/process'
	(*pstate).cheetahIni = 'lys.ini'
	(*pstate).cheetahTag = 'lys'
	


	;; Configuration file names
	configFile = 'crawler.config'
	ifile = '.cheetah-crawler'

	;; If there is a .cheetah-crawler file here, go straight into the 'pick experiment' dialog (which changes the cwd)	
	if file_test(ifile) eq 1 then begin
		crawler_pickdir
	endif $
	
	;; If there is no config file here, but there is a .cheetah-crawler in the home directory, launch the 'pick experiment' dialog
	else if file_test(configFile) eq 0 AND file_test('~/'+ifile) eq 1 then begin
		crawler_pickdir
	endif $

	else if file_test(configFile) eq 0 then begin	
		crawler_pickdir
	endif
	
		
	;; Read the crawler.config file if there is a crawler.config but no .cheetah-crawler in the current directory
	if file_test(configFile) eq 1 then begin
		info = read_csv(configFile)
		info = info.field1		
		if n_elements(info) eq 6 then begin
			info = strsplit(info, '=', /extract)
			(*pstate).xtcdir = (info[0])[1]
			(*pstate).h5dir = (info[1])[1]
			(*pstate).h5filter = (info[2])[1]
			(*pstate).geometry = (info[3])[1]
			(*pstate).process = (info[4])[1]
			(*pstate).cheetahIni = (info[5])[1]
		endif 
		if n_elements(info) eq 7 then begin
			info = strsplit(info, '=', /extract)
			(*pstate).xtcdir = (info[0])[1]
			(*pstate).h5dir = (info[1])[1]
			(*pstate).h5filter = (info[2])[1]
			(*pstate).geometry = (info[3])[1]
			(*pstate).process = (info[4])[1]
			(*pstate).cheetahIni = (info[5])[1]
			(*pstate).cheetahTag = (info[6])[1]
		endif 
	endif  $
	
	;; Else throw an error
	else begin
		message, 'There does not seem to be any crawler.config file in this location'
		exit
	endelse
	
	;; Set heading
	spawn,'pwd', result
	print,'Working directory: ', result
	widget_control, (*pstate).base, base_set_title=result
	

	print, 'XTC directory: ', (*pState).xtcdir
	print, 'HDF5 directory: ', (*pState).h5dir
	print, 'HDF5 run filter: ', (*pState).h5filter
	print, 'Process script: ', (*pState).process
	print, 'Geometry file: ', (*pState).geometry
	print, 'Default cheetah.ini: ', (*pstate).cheetahIni
	print, 'Default tag: ', (*pstate).cheetahTag

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
			printf, lun, 'xtcdir=',(*pstate).xtcdir
			printf, lun, 'hdf5dir=',(*pstate).h5dir
			printf, lun, 'hdf5filter=',(*pstate).h5filter
			printf, lun, 'geometry=',(*pstate).geometry
			printf, lun, 'process=',(*pstate).process
			printf, lun, 'cheetahini=',(*pstate).cheetahIni
			printf, lun, 'cheetahtag=',(*pstate).cheetahTag
			close, lun
			free_lun, lun
	endif

end


;;
;;	Display a single HDF5 file (eg: virtual powder)
;;
pro crawler_displayfile, filename, field=field, gamma=gamma, geometry=geometry, hist=hist
	
	if filename eq '' then begin
		print,'File does not exist'
		return
	endif
	
	if NOT KEYWORD_SET(field) then $
		field = 'data/data'
	if NOT KEYWORD_SET(geometry) then $
		geometry=''
	
	print,'Displaying: ', filename
	data = read_h5(filename, field=field)
	
	;; Histogram saturate (but revert if this makes max=min)
	if keyword_set(hist) then begin
		temp = data > 0
		temp = histogram_clip(temp, 0.002)	
		if max(temp) ne min(temp) then $
			data=temp
	endif 
	
	if keyword_set(gamma) then $
		data = data ^ gamma

	;loadct, 4, /silent
	loadct, 41, /silent
	scrolldisplay, data, title=file_basename(filename), geometry=geometry

end

;;
;;	Snippet to update the datasets file
;;
pro crawler_updateDatasetLog, pstate
	sState = *pState

	;; Get table data info
	widget_control, sState.table, get_value = table_data
	run = reform(table_data[0,*])
	dataset = reform(table_data[sState.table_datasetcol,*])
	dirname = reform(table_data[sState.table_dircol,*])

	;; Sort it
	s = sort(fix(run))
	run = run[s]
	dataset = dataset[s]
	dirname = dirname[s]
	
	;; Update datasets.txt files		
	openw, lun, 'datasets.txt', /get
	printf, lun, '# Run, DatasetID, Directory'		
	for i=0L, n_elements(dataset)-1 do begin
		str = strcompress(string(run[i], ', ', dataset[i], ', ', dirname[i]))
		printf, lun, str
	endfor
	close, lun
	free_lun, lun

	;openw, lun, 'directories.txt', /get
	;printf, lun, '# Run, Directory'		
	;for i=0L, n_elements(dataset)-1 do begin
	;	str = strcompress(string(run[i], ', ', dirname[i]))
	;	printf, lun, str
	;endfor
	;close, lun
	;free_lun, lun

end



;;
;;	Dialog to start Cheetah
;;
pro crawler_startCheetah, pState, run, menu=menu

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
	tag = sState.cheetahTag
	
	
	
	;; Menu when performing this interactively
	;;					'2, text, '+ini+', ', $
	if keyword_set(menu) then begin
		;;	Find .ini files
		list_in = file_search('../process/*.ini')
		list_in = file_basename(list_in)
		list_str = 	strjoin(list_in,'|')
		list_w = where(list_in eq ini)
		list_w = list_w[0]
		if list_w eq -1 then list_w = 0

		desc = [ 	'1, base, , column', $
					'0, label, Start run: '+startrun+', left', $
					'0, label, End run: '+endrun+', left', $
					'0, label, Command: '+cheetah+', left', $
					'0, text, '+tag+', label_left=Directory tag:, width=50, tag=ctag', $
					'2, droplist, '+list_str+', label_left=cheetah.ini file:, set_value='+string(list_w)+', tag=wini', $
					'1, base,, row', $
					'0, button, OK, Quit, Tag=OK', $
					'2, button, Cancel, Quit' $
		]		
		a = cw_form(desc, /column, title='Start cheetah')
		
		if a.OK ne 1 then $
			return

		;; Only do this if OK is pressed (!!)
		;;ini = a.cini
		ini = list_in[a.wini]
		tag = a.ctag
		(*pstate).cheetahIni = ini
		(*pstate).cheetahTag = tag
	endif

	print, 'Selected .ini file: ', ini
		
		
	;; Strip whitespace from tag
	tag = strcompress(tag, /remove_all)	
	print, tag
	
	for i=0L, nruns do begin
		cmnd = strcompress(string(cheetah, ' ', run[i], ' ', ini, ' ', tag))
		dir = string(format='(%"r%04i-%s")', run[i], tag) 
		print, cmnd
		spawn, cmnd
		
		;; Swap the Cheetah status label to 'Submitted'
		w = where(table_runs eq run[i])
		if w[0] ne -1 then begin
			widget_control, sState.table, use_table_select = [sState.table_datasetcol, w[0], sState.table_datasetcol, w[0]], set_value = [tag]
			widget_control, sState.table, use_table_select = [sState.table_statuscol, w[0], sState.table_statuscol, w[0]], set_value = ['Submitted']
			widget_control, sState.table, use_table_select = [sState.table_dircol, w[0], sState.table_dircol, w[0]], set_value = [dir]
		endif
		crawler_updateDatasetLog, pstate
	endfor
end

;;
;;	Auto-run Cheetah when new runs become available
;;
pro crawler_autostart, pState
	sState = *pState 


	;; Return if Autorun is not selected
	if (*pstate).mode_autorun eq 0 then $
		return

	;; Autorun only works in run-mode list (when runs are in columns, does not work in summary modes!)
	if (sState.table_viewmode ne 0) then $
		return

	print,'Autorun:'

	;; Collect status
	table_data = *(sState.table_pdata)
	run = reform(table_data[0,*])
	xtc_status = reform(table_data[2,*])
	cheetah_status = reform(table_data[sState.table_statuscol,*])
	rundir = reform(table_data[sState.table_dircol,*])
	h5dir = sState.h5dir

	
	;; Figure out which runs need processing
	last_processed = max(where(cheetah_status ne '---'))
	print,'Last run processed: ', run[last_processed]
	
	last_ready_xtc = max(where(xtc_status eq 'Ready'))
	print,'Last ready XTC file: ', run[last_ready_xtc]


	;; If there are runs to process...
	if last_ready_xtc gt last_processed then begin
		runs_to_process = run[last_processed+1:last_ready_xtc]
		print,'Runs to be processed: ', runs_to_process
		
		;; Loop through runs to process
		for i=0L, n_elements(runs_to_process)-1 do begin
			this_run = runs_to_process[i]
			dir = string(format='(%"r%04i")', this_run)
			;;dir = string(format='(%"r%04i-%s")', run[i], tag) 
			
			;; Does the directory exist? If yes then processing must have been started already
			dir_exists = file_test(h5dir+'/'+dir+'-*', /dir)
			if dir_exists then begin
				print, 'Run ', this_run, ' - output directory exists (skipping)'
				continue
			endif
			
			;; Start Cheetah
			print,'Starting Cheetah for run ', this_run
			crawler_startCheetah, pState, this_run 
			
		endfor			
	endif


end




;;
;;	Dialog to start general user-defined post-processing
;;
pro crawler_runCrystFEL, pState, run
  	sState = *pState
	table_data = *(sState.table_pdata)
	table_run = reform(table_data[0,*])
	table_rundir = reform(table_data[sState.table_dircol, *])

	help, run
	print, run

	nruns = n_elements(run)-1
	startrun  = run[0]
	endrun = run[nruns]
	command = '../process/run_crystfel.sh'
	ini = sState.crystfelIni


	
	desc = [ 	'1, base, , column', $
				'0, label, Start run: '+startrun+', left', $
				'0, label, End run: '+endrun+', left', $
				'0, label, Queue handler: '+command+', left', $
				'2, text, '+ini+', label_left=CrystFEL script:, width=50, tag=ini', $
				'1, base,, row', $
				'0, button, OK, Quit, Tag=OK', $
				'2, button, Cancel, Quit' $
	]		
	a = cw_form(desc, /column, title='Start CrystFEL')
	
	;; Only do this if OK is pressed (!!)
	if a.OK eq 1 then begin		
		ini = a.ini
		(*pstate).crystfelIni = ini
		
		for i=0, nruns do begin
			w = where(table_run eq run[i])
			if w[0] eq -1 then continue
			
			dir = table_rundir[w]
			path = strcompress(sState.h5dir+'/'+dir, /remove_all)
			label = file_basename(path)

			qtag = string(format='(%"indx%04i")', run[i])  
			cmnd = command + ' ' + ini + ' ' + path + ' ' + label + ' ' + qtag
			;cmnd = strcompress(string(command, ' ', path, ' ', ini, ' ', label, ' ', qtag))
			;print, cmnd
			spawn, cmnd, /sh

			
			;; Change the CrystFEL status label to 'Submitted'
			widget_control, sState.table, use_table_select = [sState.table_crystfelcol, w[0], sState.table_crystfelcol, w[0]], set_value = ['Submitted']
		endfor
	endif
end


;;
;;	Dialog to label a dataset
;;
pro crawler_labelDataset, pState, run

  	sState = *pState
	table_data = *(sState.table_pdata)
	table_runs = reform(table_data[0,*])

	nruns = n_elements(run)-1
	startrun  = run[0]
	endrun = run[nruns]
	
	ini = sState.cheetahIni
	tag = sState.cheetahTag

	desc = [ 	'1, base, , column', $
				'0, label, Start run: '+startrun+', left', $
				'0, label, End run: '+endrun+', left', $
				'2, text, '+tag+', label_left=Dataset label:, width=50, tag=tag', $
				'1, base,, row', $
				'0, button, OK, Quit, Tag=OK', $
				'2, button, Cancel, Quit' $
	]		
	a = cw_form(desc, /column, title='Start cheetah')
	
	;; Only do this if OK is pressed (!!)
	if a.OK eq 1 then begin		
		tag = a.tag

		;; Swap labels to the new tag	 and rename directory
		;; Warning:  renaming the directory may terminate jobs still in progress
		for i=0L, nruns do begin
			w = where(table_runs eq run[i])
			if w[0] ne -1 then begin 
				widget_control, sState.table, use_table_select = [sState.table_datasetcol, w[0], sState.table_datasetcol, w[0]], set_value = [tag]

				dir_old = ''
				widget_control, sState.table, use_table_select = [sState.table_dircol, w[0], sState.table_dircol, w[0]], get_value = dir_old		
				dir_old = dir_old[0]
							
				if dir_old[0] ne '---'  then begin
					dir_new = string(format='(%"r%04i-%s")', run[i], tag) 
					if dir_old ne dir_new then begin
						print, dir_old, ' --> ', dir_new
						cmd = 'mv ../hdf5/'+dir_old+'  ../hdf5/'+dir_new
						print, cmd
						spawn, cmd
						widget_control, sState.table, use_table_select = [sState.table_dircol, w[0], sState.table_dircol, w[0]], set_value = [dir_new]		
					endif
				endif
			endif
		endfor


		;; Update the datasets file
		crawler_updateDatasetLog, pstate
		
	endif
end

;;
;;	Dialog to start general user-defined post-processing
;;
pro crawler_postprocess, pState, run
  	sState = *pState
	table_data = *(sState.table_pdata)
	table_run = reform(table_data[0,*])
	table_rundir = reform(table_data[sState.table_dircol, *])

	help, run
	print, run

	nruns = n_elements(run)-1
	startrun  = run[0]
	endrun = run[nruns]
	command = sState.postprocess_command
	
	desc = [ 	'1, base, , column', $
				'0, label, Start run: '+startrun+', left', $
				'0, label, End run: '+endrun+', left', $
				'0, label, Command format: <command> <rundir>, left', $
				;'0, label, Command format: bsub -q psfehq <command> <rundir>, left', $
				'2, text, '+command+', label_left=Command:, width=50, tag=command', $
				'1, base,, row', $
				'0, button, OK, Quit, Tag=OK', $
				'2, button, Cancel, Quit' $
	]		
	a = cw_form(desc, /column, title='Start cheetah')
	
	;; Only do this if OK is pressed (!!)
	if a.OK eq 1 then begin		
		command = a.command
		(*pstate).postprocess_command = a.command
		
		for i=0, nruns do begin
			w = where(table_run eq run[i])
			if w[0] eq -1 then continue
			dir = table_rundir[w]
			path = strcompress(sState.h5dir+'/'+dir, /remove_all)
						
			;cmnd = strcompress(string('bsub -q psfehq ', command, ' ', run[i]))
			;cmnd = strcompress(string(command, ' ', path))
			cmnd = command +  ' ' + path
			print, cmnd
			spawn, cmnd, /sh
			
			;; Change the CrystFEL status label to 'Submitted'
			widget_control, sState.table, use_table_select = [sState.table_crystfelcol, w[0], sState.table_crystfelcol, w[0]], set_value = ['Submitted']
		endfor
	endif
end


;;
;;	Update and populate the table
;;
function crawler_readTable, mode, header=header

	;; Read crawler file
	data = read_csv('crawler.txt',header=header)
	ntags = n_tags(data)
	names = tag_names(data)
	
	ncols = ntags
	nrows = n_elements(data.(0))
	
	case mode of
		;; Normal column view
		0 : begin
			table_data= strarr(ncols, nrows)
			for i=0, ncols-1 do begin
				a = string(data.(i))
				a = strcompress(a)
				table_data[i,*] = a
			endfor
			end
		
		1 : begin
			table_data= strarr(ncols, nrows)
			dataset = data.(1)
			dataset_sort = sort(dataset)
			for i=0, ncols-1 do begin
				a = string(data.(i))
				a = strcompress(a)
				table_data[i,*] = a[dataset_sort]
			endfor
			end

		;; Data set view
		2 : begin
			!EXCEPT=0
			dataset = data.(1)
			dataset_s = sort(dataset)
			dataset_sorted = dataset[dataset_s]
			dataset_u = uniq(dataset_sorted)
			dataset_name = dataset_sorted[dataset_u]
			dataset_n = n_elements(dataset_name)
			
			table_data = strarr(ncols, dataset_n)
			table_data[*] = '---'
			
			;; Fix non-existent data (otherwise prints a nasty message)
			w6 = where(data.(6) eq '---')
			w7 = where(data.(7) eq '---')
			if w6[0] ne -1 then data.(6)[w6] = '0'
			if w7[0] ne -1 then data.(7)[w7] = '0'
			
			for i=0, dataset_n-1 do begin
				w = where(dataset eq dataset_name[i])
				np = total(long(data.(6)[w]))
				nh = total(long(data.(7)[w]))
				perc = 100.*nh/np
				if not finite(np) then np = '---'
				if not finite(nh) then nh = '---'
				if not finite(perc) then perc = '---'
				table_data(0,i) = n_elements(w)
				table_data(1,i) = dataset_name[i]
				table_data(6,i) = string(long(np))
				table_data(7,i) = string(long(nh))
				table_data(9,i) = perc
				junk = check_math()
			endfor
			table_data = strcompress(table_data)
			end

			
	endcase


	return, table_data
end

pro crawler_updateTable, pState

	print,'Refreshing table'
  	sState = *pState
	table = sState.table

	;; Does the file exist?
	if file_test('crawler.txt') eq 0 then begin
		print,'Crawler file does not exist'
		return
	endif

	;; Get screen size
	screensize = get_screen_size()
	xview = screensize[0] - 140
	yview = screensize[1] - 140

	;; Read table data
	table_data = crawler_readTable(sState.table_viewmode, header=h)
	s = size(table_data,/dim)
	ncols = s[0]
	if n_elements(s) gt 1 then nrows = s[1] else nrows=1

	;; Update the table	
	widget_control, table, table_xsize = ncols
	widget_control, table, table_ysize = nrows
	widget_control, table, column_labels=h
	widget_control, table, row_labels = ''
	widget_control, table, set_value = table_data
	
	;; Select the last element (can be a pain in the neck!)
	;widget_control, table, set_table_select = [-1, nrows-1, -1, nrows-1]


	;; Column widths
	colwidth = strlen(h)+3
	cwidth = colwidth * !d.x_ch_size + 6
	widget_control, table, column_widths = cwidth

	;; Window width (silly idea?)
	widget_control, table, scr_xsize=total(cwidth)+30


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
	rundir = reform(table_data[sState.table_dircol,rowstart:rowend])
	
	
	;; Old way of determining the directory name when we only knew the run number
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
			crawler_autostart, pState

			;; Schedule the next click
			if sState.table_autorefresh ne 0 then begin
					widget_control, sState.button_refresh,  timer=sState.table_autorefresh
			endif
		end
				
		;; 
		;;	Change view type
		;;
		sState.button_viewtype : begin
		
			case sState.table_viewmode of
				0 : (*pState).table_viewmode = 1
				1 : (*pState).table_viewmode = 2
				2 : (*pState).table_viewmode = 0
				else : (*pState).table_viewmode = 0
			endcase				
		
			case (*pState).table_viewmode of
				0 : widget_control, sState.button_viewtype,  set_value = 'Dataset view'
				1 : widget_control, sState.button_viewtype,  set_value = 'Summary'
				2 : widget_control, sState.button_viewtype,  set_value = 'Run view'
				else : widget_control, sState.button_viewtype,  set_value = 'Dataset view'
			endcase				

			crawler_updateTable, pState
		end

		;;
		;;	Clear previous HDF5 errors
		;;
		;;h5_close

		;;
		;;	Action buttons
		;;
		;; Launch Cheetah
		sState.button_cheetah : begin
			run = crawler_whichRun(pstate, /run, /multiple)
			crawler_startCheetah, pState, run, /menu
		end			

		sState.button_postprocess : begin
			run = crawler_whichRun(pstate, /run, /multiple)
			crawler_postprocess, pState, run
		end			

		sState.button_crystfel : begin
			run = crawler_whichRun(pstate, /run, /multiple)
			crawler_runCrystFEL, pState, run
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
			;crawler_displayfile, f[0]
			crawler_displayfile, f[0], field='data/correcteddata', geometry=sState.geometry, /hist
		end

		sState.mbfile_unlock : begin
			widget_control, sState.mbfile_configure, sensitive=1
			widget_control, sState.mbfile_crawl, sensitive=1
			widget_control, sState.mbcheetah_run, sensitive=1
			widget_control, sState.mbcheetah_label, sensitive=1
			widget_control, sState.mbcheetah_autorun, sensitive=1
			widget_control, sState.button_postprocess, sensitive=1
			widget_control, sState.button_cheetah, sensitive=1
			widget_control, sState.button_crystfel, sensitive=1
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
			if sState.table_autorefresh ne 0 then begin
					widget_control, sState.button_refresh,  timer=sState.table_autorefresh
			endif
		end
		sState.mbcheetah_run : begin
			run = crawler_whichRun(pstate, /run, /multiple)
			crawler_startCheetah, pState, run, /menu
		end

		sState.mbcheetah_label : begin
			run = crawler_whichRun(pstate, /run, /multiple)
			crawler_labelDataset, pState, run
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

		sState.mbview_cxipowder_class1_detcorr : begin
			dir = crawler_whichRun(pstate, /path)
			f = file_search(dir,'*detector0-class1-sum.h5')
			crawler_displayfile, f[0], field='data/non_assembled_detector_corrected', geometry=sState.geometry, /hist, gamma=1
		end
		sState.mbview_cxipowder_class1_detphotcorr : begin
			dir = crawler_whichRun(pstate, /path)
			f = file_search(dir,'*detector0-class1-sum.h5')
			crawler_displayfile, f[0], field='data/non_assembled_detector_and_photon_corrected', geometry=sState.geometry, /hist, gamma=1
		end
		sState.mbview_cxipowder_class0_detcorr : begin
			dir = crawler_whichRun(pstate, /path)
			f = file_search(dir,'*detector0-class0-sum.h5')
			crawler_displayfile, f[0], field='data/non_assembled_detector_corrected', geometry=sState.geometry, /hist, gamma=1
		end
		sState.mbview_cxipowder_class0_detphotcorr : begin
			dir = crawler_whichRun(pstate, /path)
			f = file_search(dir,'*detector0-class0-sum.h5')
			crawler_displayfile, f[0], field='data/non_assembled_detector_and_photon_corrected', geometry=sState.geometry, /hist, gamma=1
		end

		sState.mbview_cxipowder_diff : begin
			dir = crawler_whichRun(pstate, /path)
			f0 = file_search(dir,'*detector0-class0-sum.h5')
			f1 = file_search(dir,'*detector0-class1-sum.h5')
			d0 = read_h5(f0[0], field='data/non_assembled_detector_corrected')
			d1 = read_h5(f1[0], field='data/non_assembled_detector_corrected')
			diff = d1 - (total(d1)/total(d0))*d0
			fout = strmid(f1[0], 0, strlen(f1[0])-6) + 'class0-diff.h5'
			write_h5,fout, diff
			crawler_displayfile, fout, field='data/data', geometry=sState.geometry, /hist, gamma=1
		end
		
		
		
		sState.mbview_powder : begin
			dir = crawler_whichRun(pstate, /path)
			f = file_search(dir,'*detector0-class1-sum.h5')
			crawler_displayfile, f[0], field='data/correcteddata', geometry=sState.geometry, /hist, gamma=1
		end
		sState.mbview_powderdark : begin
			dir = crawler_whichRun(pstate, /path)
			f = file_search(dir,'*detector0-class0-sum.h5')
			crawler_displayfile, f[0], field='data/correcteddata', geometry=sState.geometry, /hist, gamma=1
		end
		sState.mbview_peakpowder : begin
			dir = crawler_whichRun(pstate, /path)
			f = file_search(dir,'*detector0-class1-sum.h5')
			crawler_displayfile, f[0], field='data/peakpowder', geometry=sState.geometry, /hist, gamma=1
		end
		sState.mbview_peakpowderdark : begin
			dir = crawler_whichRun(pstate, /path)
			f = file_search(dir,'*detector0-class0-sum.h5')
			crawler_displayfile, f[0], field='data/peakpowder', geometry=sState.geometry, /hist, gamma=1
		end
		sState.mbview_bsub : begin
			dir = crawler_whichRun(pstate, /path)
			f = file_search(dir,'bsub.log')
			xdisplayfile, f[0]
		end
		sState.mbview_clog : begin
			dir = crawler_whichRun(pstate, /path)
			f = file_search(dir,'log.txt')
			xdisplayfile, f[0]
		end
		sState.mbview_cstatus : begin
			dir = crawler_whichRun(pstate, /path)
			f = file_search(dir,'status.txt')
			xdisplayfile, f[0]
		end
		
		sState.mbview_waxs : begin
			runs = crawler_whichRun(pstate, /path, /multiple)
			waxsview, runs
		end

		sState.mbview_badpix : begin
			dir = crawler_whichRun(pstate, /path)
			f = file_search(dir,'*detector0-class0-sum.h5')
			print, f
			mask = badpix_from_darkcal(f, /save, /edge, /menu)
		end

		sState.mbview_badpix2 : begin
			dir = crawler_whichRun(pstate, /path)
			f = file_search(dir,'*detector0-class0-sum.h5')
			print, f
			mask = badpix_from_powder(f, /save, /edge, /menu)
		end

		sState.mbview_combinemasks : begin
			combine_badpix_masks, /save
		end

		
		sState.mbview_satcheck : begin
			dir = crawler_whichRun(pstate, /path)
			file = file_search(dir,'peaks.txt')
			;;print, file
			saturation_check, file
		end
		
		sState.mbview_peakogram : begin
			dir = crawler_whichRun(pstate, /path)
			file = file_search(dir,'peaks.txt')
			cmd = 'peakogram -i '+file
			print, cmd
			spawn, cmd, unit=unit
			;wait, 10
			;free_lun, unit
		end
		
		
		sState.mbview_gainmap : begin
			cd, current=dir
			dir = file_dirname(dir)+'/calib'
			make_cspad_gain_map, sState.geometry, 0, 300, 6.87526, /menu
		end


		sState.mbview_translategainmap : begin
			cd, current=dir
			dir = file_dirname(dir)+'/calib'
			translate_cspad_gain_map, gainval=6.87526, geometry=sState.geometry, /menu
		end


		sState.mbfile_autorefresh : begin
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


		;; Auto-run menu item
		sState.mbcheetah_autorun : begin
			;; Start with a menu item to let people know what will happen, and give a chance to cancel
			if  (*pstate).mode_autorun eq 0 then begin
				desc = [ 	'1, base, , column', $
							'0, label, Automatically start Cheetah when new runs appear using last used .ini file., left', $
							'0, label, (current .ini file would be '+sState.cheetahIni+'), left', $
							'0, label, (current sample tag would be '+sState.cheetahTag+'), left', $
							'2, label, Uncheck menu item to stop, left', $
							'1, base,, row', $
							'0, button, OK, Quit, Tag=OK', $
							'2, button, Cancel, Quit' $
				]		
				a = cw_form(desc, /column, title='Autorun Cheetah')
				if a.OK eq 1 then begin		
					(*pstate).mode_autorun = 1 - (*pstate).mode_autorun
					widget_control, sState.mbcheetah_autorun, set_button=(*pstate).mode_autorun
					print,'Autorun enabled, uncheck menu item to cancel'
				endif
			endif $
			else begin
				(*pstate).mode_autorun = 1 - (*pstate).mode_autorun
				widget_control, sState.mbcheetah_autorun, set_button=(*pstate).mode_autorun
				print,'Autorun disabled'
			endelse
			
		end



		
		else : begin 
			help, ev
		end
			
		
	endcase

	catch, /cancel

end



pro crawler_view

	;;
	;;	Device options must be set
	;;
	device, decomposed=0
	device, retain=3

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
	mbfile_configure = widget_button(mbfile, value='Configure', sensitive=0)
	mbfile_unlock = widget_button(mbfile, value='Unlock command operations')
	mbfile_crawl = widget_button(mbfile, value='Start crawler', sensitive=0)
	mbfile_refresh = widget_button(mbfile, value='Refresh table')
	mbfile_autorefresh = widget_button(mbfile, value='Auto refresh table')
	mbfile_quit = widget_button(mbfile, value='Quit')

	mbfile = widget_button(bar, value='Cheetah')
	mbcheetah_run = widget_button(mbfile, value='Process selected runs', sensitive=0)
	mbcheetah_label = widget_button(mbfile, value='Label or relabel dataset', sensitive=0)
	mbcheetah_autorun = widget_button(mbfile, value='Autorun when new data is ready', sensitive=0, /checked)
	

	mbtool = widget_button(bar, value='Tools')
	mbview_badpix = widget_button(mbtool, value='Make bad pixel mask from darkcal')
	mbview_badpix2 = widget_button(mbtool, value='Make bad pixel mask from brightfield')
	mbview_combinemasks = widget_button(mbtool, value='Combine masks')
	mbview_satplot1 = widget_button(mbtool, value='Plot peak maximum vs radius')
	mbview_satcheck = widget_button(mbtool, value='Saturation check')
	mbview_peakogram = widget_button(mbtool, value='Peakogram')
	mbview_gainmap = widget_button(mbtool, value='Create CSPAD gain map')
	mbview_translategainmap = widget_button(mbtool, value='Translate CSPAD gain map')




	mbfile = widget_button(bar, value='View')
	mbview_bsub = widget_button(mbfile, value='bsub log file')
	mbview_clog = widget_button(mbfile, value='cheetah log file')
	mbview_cstatus = widget_button(mbfile, value='cheetah status file')
	mbview_images = widget_button(mbfile, value='HDF5 files')
	mbview_hitrate = widget_button(mbfile, value='Hit rate plot')
	mbview_resolution = widget_button(mbfile, value='Resolution plot')
	mbview_cxipowder_class1_detphotcorr = widget_button(mbfile, value='Virtual powder hits (class 1, background subtracted)')
	mbview_cxipowder_class1_detcorr = widget_button(mbfile, value='Virtual powder hits (class 1, detector corrected only)')
	mbview_cxipowder_class0_detphotcorr = widget_button(mbfile, value='Virtual powder blanks (class 0, background subtracted)')
	mbview_cxipowder_class0_detcorr = widget_button(mbfile, value='Virtual powder blanks (class 0, detector corrected only)')
	mbview_cxipowder_diff = widget_button(mbfile, value='Virtual powder difference (class1-class0, detector corrected)')
	mbview_peakpowder = widget_button(mbfile, value='Peakfinder virtual powder hits (class 1)')
	mbview_peakpowderdark = widget_button(mbfile, value='Peakfinder virtual powder blanks (class 0)')
	mbview_powder = widget_button(mbfile, value='Virtual powder hits (data/correcteddata)')
	mbview_powderdark = widget_button(mbfile, value='Virtual powder blanks (data/correcteddata)')
	mbview_waxs = widget_button(mbfile, value='View WAXS traces')


	;;
	;;	Quick action buttons
	;;
	base2 = widget_base(base, /row)
	button_refresh = widget_button(base2, value='Refresh')
	button_viewtype = widget_button(base2, value='Dataset view')
	button_cheetah = widget_button(base2, value='Run Cheetah', sensitive=0)
	button_crystfel = widget_button(base2, value='Run CrystFEL', sensitive=0)
	button_postprocess = widget_button(base2, value='Postprocess')
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
			
			table_datasetcol : 1, $
			table_statuscol : 3, $
			table_crystfelcol : 4, $
			table_dircol : 5, $
			table_viewmode : 0, $
			
			mbfile_configure : mbfile_configure, $
			mbfile_crawl : mbfile_crawl, $
			mbfile_refresh : mbfile_refresh, $
			mbfile_unlock : mbfile_unlock , $ 
			mbfile_autorefresh : mbfile_autorefresh, $ 
			mbfile_quit : mbfile_quit, $
			mbcheetah_run : mbcheetah_run, $
			mbcheetah_label : mbcheetah_label, $
			mbcheetah_autorun : mbcheetah_autorun, $
			
			mbview_satcheck : mbview_satcheck, $
			mbview_peakogram : mbview_peakogram, $
			mbview_gainmap : mbview_gainmap, $
			mbview_translategainmap : mbview_translategainmap, $

			mbview_images : mbview_images, $
			mbview_hitrate : mbview_hitrate, $
			mbview_resolution : mbview_resolution, $
			mbview_powder : mbview_powder, $
			mbview_powderdark : mbview_powderdark, $
			mbview_peakpowder : mbview_peakpowder, $
			mbview_peakpowderdark : mbview_peakpowderdark, $
			mbview_cxipowder_class1_detcorr : mbview_cxipowder_class1_detcorr, $
			mbview_cxipowder_class1_detphotcorr : mbview_cxipowder_class1_detphotcorr, $
			mbview_cxipowder_class0_detcorr : mbview_cxipowder_class0_detcorr, $
			mbview_cxipowder_class0_detphotcorr : mbview_cxipowder_class0_detphotcorr, $
			mbview_cxipowder_diff : mbview_cxipowder_diff, $
			
			mbview_badpix : mbview_badpix, $
			mbview_badpix2 : mbview_badpix2, $
			mbview_combinemasks : mbview_combinemasks, $
			
			mbview_waxs : mbview_waxs, $
			
			mbview_bsub : mbview_bsub, $
			mbview_clog : mbview_clog, $
			mbview_cstatus : mbview_cstatus, $
			
			button_refresh : button_refresh, $
			button_viewtype : button_viewtype, $
			button_cheetah : button_cheetah, $
			button_crystfel : button_crystfel, $
			button_postprocess : button_postprocess, $
			button_hits : button_hits, $
			button_hitrate : button_hitrate, $
			button_resolution : button_resolution, $
			button_powder : button_powder, $
			
			mode_autorun : 0, $

			xtcdir : 'Not set', $
			h5dir : 'Not set', $
			h5filter : 'Not set', $
			process : 'Not set', $
			geometry : 'Not set', $
			cheetahIni : 'Not set', $
			cheetahTag : 'Not set', $
			crystfelIni : '../process/lys.crystfel', $
			postprocess_command : '../process/postprocess.sh' $
	}
	
	;; Establish polite error handler to catch crashes
	;; (only if not in debug mode)
	;if 1 then begin
	;	catch, Error_status 
	;	if Error_status ne 0 then begin
	;		message = 'Execution error: ' + !error_state.msg
	;		r = dialog_message(message,title='Error',/center,/error)
	;		catch, /cancel
	;		return
	;	endif 
	;endif


	pstate = ptr_new(sState)
	WIDGET_CONTROL, base, SET_UVALUE=pState
	crawler_config, pState
	crawler_updateTable, pState

   	XMANAGER, 'Test', base, event='crawler_event', /NO_BLOCK

	;catch, /cancel
	
end
	
