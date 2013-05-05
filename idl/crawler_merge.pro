;
; crawler_merge
; Anton Barty, 2013
;
; Merges data from the various crawler files into one .txt file for display
; Could be implemented in sh, or perl, or python
;
; Run in this order
;	crawler_xtc
;	crawler_hdf5
;	crawler_dataset
;	crawler_merge
;


pro crawler_merge

	print, 'crawler_merge: xtc.txt hdf5.txt datasets.txt'


	;; XTC info
	xtc = read_csv('xtc.txt')
	xtcrun = xtc.field1
	xtcstatus = xtc.field2

	;; Dataset info
	dataset = read_csv('datasets.txt')	
	datarun = dataset.field1
	dataset = dataset.field2


	;; HDF5 info
	h5 = read_csv('hdf5.txt')	
	h5run = h5.field1
	h5status = h5.field2
	h5dir = h5.field3
	h5processed = h5.field4
	h5hits = h5.field5
	h5hitrate = h5.field6
	

	
	;; Populate the output table
	openw, fout, 'crawler.txt', /get
	printf, fout, '#Run, DatasetID, XTC, Cheetah, Directory, Nprocessed, Nhits, Hitrate%'
	
	
	for i = 0L, n_elements(xtcrun)-1 do begin	
		
		;; Retrieve dataset ID for this run
		wdataset = where(datarun eq xtcrun[i])
		ds = dataset[wdataset]

		;; Retrieve HDF5 directories for this run (used when HDF5 filter allows for multiple results per run)
		whdf5 = where(h5run eq xtcrun[i])
		
		;; Loop through all HDF5 directories that exist and are associated with this run
		if whdf5[0] ne -1 then begin
			for j=0L, n_elements(whdf5)-1 do begin
				status = h5status[whdf5[j]]
				dir = h5dir[whdf5[j]]
				processed = h5processed[whdf5[j]]
				hits = h5hits[whdf5[j]]
				hitrate = h5hitrate[whdf5[j]]
				;;hitrate = (fix(100*hitrate))/100.
				hitrate = strmid(strcompress(hitrate,/remove_all),0,4)
			endif else begin
				status = '---'
				dir = '---'
				processed = '---'
				hits = '---'
				hitrate = '---'
			endelse	

			str = strcompress(string(xtcrun[i], ',', dataset[wdataset], ',', xtcstatus[i], ',', status, ',', dir, ',', processed, ',', hits, ',', hitrate))
			printf, fout, str
		endfor
		
	endfor
	
	close, fout
	free_lun, fout

end
