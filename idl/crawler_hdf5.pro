;
; crawler_hdf5
; Anton Barty, 2013
;
; Monitors a given set of HDF5 directories and updates a text file accordingly 
; Could be implemented in sh, or perl, or python
;


pro crawler_hdf5, hdf5dir, pattern

	if n_elements(hdf5dir) eq 0 then $
		hdf5dir = '/reg/d/psdm/cxi/cxi69113/scratch/hdf5'
	if n_elements(pattern) eq 0 then $
		pattern = 'r*-ab'


	;; Find the available directories
	searchdir = hdf5dir+'/'+pattern
	print, 'crawler_hdf5: ', searchdir 
	h5dir = file_search(searchdir)


	;; No files = blank hdf5 status file
	if h5dir[0] eq '' then begin
		openw, fout, 'hdf5.txt', /get	
		printf, fout, '# Run, status, directory, processed, hits, hitrate%, mtime'
		close, fout
		free_lun, fout
		return
	endif
	
	
	ndir = n_elements(h5dir)
	run = strmid(file_basename(h5dir),1,4)
	h5dir_short = file_basename(h5dir)
	status = strarr(ndir)
	processed = lonarr(ndir)
	hits = lonarr(ndir)
	hitrate = fltarr(ndir)	
	mt = lon64arr(ndir)
	
	for i=0L, ndir-1 do begin
		
		s = 'Submitted'		
	
		catch, Error_status 
		if Error_status ne 0 then begin
			message = 'Execution error: ' + !error_state.msg
			print, message
			status[i] = 'Dont panic'
			catch, /cancel
			continue
		endif 

		;; Newer versions of Cheetah will output a brief status.txt file
		;; Sometimes this will tank when the file has not been completely written - fix later, solve now with catch errors
		sfile = h5dir[i]+'/status.txt'
		if file_test(sfile) then begin
			;; Are there 3 lines (the original format)?
			nlines = file_lines(sfile)
			if nlines eq 3 then begin
				data1 = string('')
				data2 = long(0)
				data3 = long(0)
				openr, lun, sfile, /get
				readf, lun, data1
				readf, lun, data2
				readf, lun, data3
				close, lun
				free_lun, lun
			endif $
			;; Or 6 lines (the 2nd generation format)?
			else if nlines eq 6 then begin
				data = read_csv(sfile)
				data = data.field1
				data1 = strsplit(data[3], ':', /extract)				
				data1 = data1[1]
				data2 = strsplit(data[4], ':', /extract)				
				data2 = long(data2[1])
				data3 = strsplit(data[5], ':', /extract)				
				data3 = long(data3[1])
			endif $
			;; Not the right number of lines means some sort of race condition occurred
			else begin
				status[i] = 'Dont panic'
				continue
			endelse

			;; Populate the array
			status[i] = data1
			processed[i] = data2
			hits[i] = data3
			
			;; Is the file stale??
			staletime = 20
			info = file_info(sfile)
			mtime = info.mtime
			now = systime(/sec)
			if (now - mtime) gt (staletime*60) then begin
				if strpos(status[i], 'Not finished') ne -1 then begin
					status[i] = 'Stalled?'
				endif
			endif
			info = file_info(sfile)
			mt[i] = info.mtime
		endif $
		
		;; Older versions won't 
		else begin
			;; Clean exit?
			;; Directory exists means job has at least been sent to the queue
			;; Otherwise, look at log.txt to check for 'clean exit'
			s = 'Submitted'
			if file_test(h5dir[i]+ '/log.txt') then begin 
				info = file_info(h5dir[i]+ '/log.txt')
				mt[i] = info.mtime
				command = 'tail -n 2 ' +h5dir[i]+ '/log.txt | head -n 1'
				spawn, command, r
				w2 = strpos(r, 'Cheetah clean exit')
				if r eq '' then $
					s = 'Not started'  $
				else if w2 ne -1 then $
					s = 'Finished' $
				else $
					s = 'Not finished' 
			endif
			status[i] = s
		
		
			;; Number of processed frames
			processed[i] = 0
			if file_test(h5dir[i]+ '/frames.txt') then begin
				command = 'wc -l ' +h5dir[i]+ '/frames.txt'
				spawn, command, r
				w1 = strpos(r, 'No such file or directory')
				if w1 ne -1 then $
					processed[i] = 0 $
				else begin
					s = strsplit(r, ' ', /extract)
					p = s[0]
					processed[i] = long(p)-1 
				endelse
			endif


			;; Number of hits
			hits[i] = 0 
			if file_test(h5dir[i]+ '/cleaned.txt') then begin
				command = 'wc -l ' +h5dir[i]+ '/cleaned.txt'
				spawn, command, r
				w1 = strpos(r, 'No such file or directory')
				if w1 ne -1 then $
					hits[i] = 0 $
				else begin
					s = strsplit(r, ' ', /extract)
					c = s[0]
					hits[i] = long(c)-1
				endelse
			endif
		endelse
	
		
		;; Hit rate
		if processed[i] ge 1 then $
			hitrate[i] = 100.*float(hits[i])/float(processed[i]) $
		else $
			hitrate[i] = 0 
			
		catch, /cancel

	endfor
	



	;; Populate the output table
	openw, fout, 'hdf5.txt', /get
	
	printf, fout, '# Run, status, directory, processed, hits, hitrate%, mtime'
	
	for i = 0L, n_elements(run)-1 do begin	

		str = strcompress(string(run[i], ',', status[i], ',', h5dir_short[i], ',', processed[i], ',', hits[i], ',', hitrate[i], ',', mt[i] ))
		printf, fout, str

	endfor
	
	close, fout
	free_lun, fout
	close, /all

end
