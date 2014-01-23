;
; crawler_xtc
; Anton Barty, 2013
;
; Monitors a given XTC directory for latest saved files and updates a text file accordingly 
; Could be implemented in sh, or perl, or python
;


pro crawler_xtc, xtcdir

	if n_elements(xtcdir) eq 0 then $
		xtcdir = '/reg/d/psdm/cxi/cxi69113/xtc/'


	;; Find the available XTC files
	filter = xtcdir+'*.xtc*'
	print, 'crawler_xtc: ', filter 

	xtc = file_search(xtcdir+'*.xtc*')
	xtc = file_basename(xtc)

	;; No files = blank XTC status file
	if xtc[0] eq '' then begin
		openw, fout, 'xtc.txt', /get	
		printf, fout, '# Run, status'
		close, fout
		free_lun, fout
		return
	endif

	rstart = strpos(xtc[0], 'r')
	run = long(strmid(xtc, rstart+1, 4))
	
	;; Sort the list
	run = run[sort(run)]

	;; Find the unique run numbers
	u = uniq(run)
	run = run[u]
	
	;; Find the XTC files still copying
	xtc_pending = file_search(xtcdir+'*.xtc.inprogress')
	xtc_pending = file_basename(xtc_pending)
	run_pending = long(strmid(xtc_pending, rstart+1, 4))


	;; Populate the output table
	openw, fout, 'xtc.txt', /get
	
	printf, fout, '# Run, status'
	
	for i = 0L, n_elements(run)-1 do begin	
		tag = 'Ready'
		w = where(run_pending eq run[i])
		if w[0] ne -1 then tag = 'Copying'
		
		str = strcompress(string(run[i], ', ', tag))
		printf, fout, str
	endfor
	
	close, fout
	free_lun, fout

end
