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
		printf, fout, '# Run, status, processed, hits, hitrate%'
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
	
	for i=0L, ndir-1 do begin
		
		s = 'Queued'		
	
		;; Newer versions of Cheetah will output a brief status.txt file
		if file_test(h5dir[i]+'/status.txt') then begin
			data1 = string('')
			data2 = long(0)
			data3 = long(0)
			openr, lun, h5dir[i]+'/status.txt', /get
			readf, lun, data1
			readf, lun, data2
			readf, lun, data3
			close, lun
			free_lun, lun
			
			status[i] = data1
			processed[i] = data2
			hits[i] = data3
		endif $
		
		;; Older versions won't 
		else begin
			;; Clean exit?
			;; Directory exists means job has at least been sent to the queue
			;; Otherwise, look at log.txt to check for 'clean exit'
			s = 'Queued'
			if file_test(h5dir[i]+ '/log.txt') then begin 
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
			
	
	endfor
	



	;; Populate the output table
	openw, fout, 'hdf5.txt', /get
	
	printf, fout, '# Run, status, directory, processed, hits, hitrate%'
	
	for i = 0L, n_elements(run)-1 do begin	

		str = strcompress(string(run[i], ',', status[i], ',', h5dir_short[i], ',', processed[i], ',', hits[i], ',', hitrate[i]))
		printf, fout, str

	endfor
	
	close, fout
	free_lun, fout

end
