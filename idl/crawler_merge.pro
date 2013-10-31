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

	print, 'crawler_merge: xtc.txt hdf5.txt datasets.txt crystfel.txt'


	;; XTC info
	xtc = read_csv('xtc.txt')
	xtcrun = xtc.field1
	xtcstatus = xtc.field2

	;; Dataset info
	fdataset = file_test('datasets.txt')
	if fdataset ne 0 then begin
		dataset = read_csv('datasets.txt')	
		datarun = dataset.field1
		datasetdir = dataset.field3
		dataset = dataset.field2
	endif

	;; HDF5 info
	h5 = read_csv('hdf5.txt')	
	h5run = h5.field1
	h5status = h5.field2
	h5dir = h5.field3
	h5processed = h5.field4
	h5hits = h5.field5
	h5hitrate = h5.field6
	h5mtime = h5.field7
	
	;; CrystFEL info
	fcrystfel = file_test('crystfel.txt')
	if fcrystfel ne 0 then begin
		cf = read_csv('crystfel.txt')
		cfrun = cf.field1
		cfstatus = cf.field2
		cfindexed = cf.field3
	endif	
	
	;; Populate the output table
	openw, fout, 'crawler.txt', /get
	printf, fout, '#Run, Dataset, XTC, Cheetah, CrystFEL, H5 Directory, Nprocessed, Nhits, Nindex, Hitrate%'
	
	
	for i = 0L, n_elements(xtcrun)-1 do begin	
		
		;; Retrieve dataset ID for this run
		ds = '---'
		if fdataset ne 0 then begin
			wdataset = where(datarun eq xtcrun[i])
			if wdataset[0] ne -1 then begin
				ds = dataset[wdataset] 
			endif
		endif


		;; Retrieve HDF5 directories for this run (used when HDF5 filter allows for multiple results per run)		
		status = '---'
		dir = '---'
		processed = '---'
		hits = '---'
		hitrate = '---'
		whdf5 = where(h5run eq xtcrun[i])
		if whdf5[0] ne -1 then begin
			s = sort(h5mtime[whdf5])
			mtime = 0
			for j=0L, n_elements(whdf5)-1 do begin
				h5index = whdf5[s[j]]
				status = h5status[h5index]
				dir = h5dir[h5index]
				processed = h5processed[h5index]
				hits = h5hits[h5index]
				hitrate = h5hitrate[h5index]
				hitrate = strmid(strcompress(hitrate,/remove_all),0,4)
				if fdataset ne 0 then begin
					if strpos(dir, dataset[wdataset]) ne -1 then $
						break
				endif
			endfor
		endif 
		
		;; Now gather information about Crystfel (or whatever else)
		status2 = '---'
		hits2 = '---'
		if fcrystfel ne 0 then begin
			wcf = where(cfrun eq xtcrun[i])
			if whcf[0] ne -1 then begin
				for j=0L, n_elements(wcf)-1 do begin
					status2 = cfstatus[wcf[j]]
					hits2 = cfindexed[wcf[j]]
				endfor
			endif 
		endif 		
		
		str = strcompress(string(xtcrun[i], ',', ds, ',', xtcstatus[i], ',', status, ',', status2, ',', dir, ',', processed, ',', hits, ',', hits2, ',', hitrate))
		writeu, fout, str
		printf, fout, ' '
		
	endfor
	
	close, fout
	free_lun, fout

end
