;
; crawler_crystfel
; Anton Barty, 2013
;
; Monitors a given set of HDF5 directories and updates a text file accordingly 
; Could be implemented in sh, or perl, or python
;


pro crawler_crystfel, CFdir

	if n_elements(CFdir) eq 0 then $
		CFdir = '/reg/d/psdm/cxi/cxi69113/scratch/indexing'


	;; Find a list of started CrystFEL jobs
	print, 'crawler_crystfel: ', CFdir
	CFfile = file_search(CFdir,'*.lst')


	;; No files = blank status file
	if CFfile[0] eq '' then begin
		openw, fout, 'crystfel.txt', /get	
		printf, fout, '# Run, status, indexed'
		close, fout
		free_lun, fout
		return
	endif
	
	
	
	n = n_elements(CFfile)
	run = strmid(file_basename(CFfile),1,4)
	status = strarr(n)
	indexed = lonarr(n)
	size = lon64arr(n)
	
	
	for i=0L, n-1 do begin
		
		status[i] = 'Submitted'	
			
	
	
		catch, Error_status 
		if Error_status ne 0 then begin
			message = 'Execution error: ' + !error_state.msg
			print, message
			status[i] = 'Dont panic'
			catch, /cancel
			continue
		endif 
		

		;; Has the stream file been created yet?
		streamfile = strmid(CFfile[i], 0, strlen(CFfile[i])-4)+'.stream'
		if file_test(streamfile) eq 1 then begin
			status[i] = 'Running'
		endif
		
		;; Has the stream file been modified recently?
		info = file_info(streamfile)
		mtime = info.mtime
		now = systime(/sec)
		if (now - mtime) gt 60 then begin
			status[i] = 'Done'
		endif 
		

		;; Find out how many crystals from the log file
		;; (easier than grep'ing a many GB file)
		logfile = strmid(CFfile[i], 0, strlen(CFfile[i])-4)+'.log'
		if file_test(logfile) eq 1 then begin
			command = 'tail -n 100 ' + logfile + ' | grep indexable | tail -n 1 '
			spawn, command, result			
			result = result[0]
			p = strpos(result,'indexable')
			indexed[i] = long(strmid(result, 0, p))
		endif
		


	endfor
	



	;; Populate the output table
	openw, fout, 'crystfel.txt', /get
	
	printf, fout, '# Run, status, indexed'
	
	for i = 0L, n_elements(CFfile)-1 do begin	

		str = strcompress(string(run[i], ',', status[i], ',', indexed[i]))
		printf, fout, str

	endfor
	
	close, fout
	free_lun, fout
	close, /all

end
