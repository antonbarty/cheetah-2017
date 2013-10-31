;
; crawler_dataset
; Anton Barty, 2013
;
; Adds new XTC data runs to end of datasets.txt whilst preserving contents
; Could be implemented in sh, or perl, or python
;


pro crawler_dataset, datasetfile

	xtc = read_csv('xtc.txt')
	xtcrun = xtc.field1

	print, 'crawler_dataset: datasets.txt'

	if n_elements(datasetfile) eq 0 then $
		datasetfile = 'datasets.txt'

	f = file_test(datasetfile)
	if f ne 0 then begin
		dataset = read_csv(datasetfile)	
		datarun = dataset.field1
		dataset = dataset.field2
	endif $
	else begin
		datarun = xtcrun
		dataset = replicate('---', n_elements(datarun))
	endelse
		
	
	;; Populate the output table
	openw, fout, datasetfile, /get
	printf, fout, '#Run, DatasetInfo, Directory'
	
	for i = 0L, n_elements(xtcrun)-1 do begin	
		
		w = where(datarun eq xtcrun[i])
		
		if w[0] eq -1 then $
			tag = '---' $
		else $
			tag = dataset[w]

		str = strcompress(string(xtcrun[i], ',', tag))
		printf, fout, str
	endfor
	
	close, fout
	free_lun, fout

end
