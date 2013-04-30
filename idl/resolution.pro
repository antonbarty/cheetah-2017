pro resolution, dir, buffer=buffer

	if n_elements(dir) eq 0 then begin
		print, 'Usage:'
		print, 'hitrate, directory'
	endif

	if NOT KEYWORD_SET(buffer) then buffer=0

	;; 
	;;	Resolution histograms come from cleaned.txt
	;;
	
	infile = dir + '/cleaned.txt'
	base = file_basename(dir)

	a = read_csv(infile, comment='#')
	file = a.field1
	npeaks = float(a.field3)
	resolution = float(a.field6)
	density = float(a.field7)
	
	nhits = n_elements(file)
	print,'Number of hits', nhits
	
	;; Resolution histogram
	hr = histogram(resolution, min=0, max=1000, binsize=10, locations=x)
	
	;; Plot and save
	p1 = barplot(x,hr, buffer=buffer)
	p1.xtitle = 'Resolution (pixels)'
	p1.ytitle = 'Frequency'
	p1.title = dir + ': Resolution histogram'
	
	hitstr = strcompress(string(nhits, ' hits'))
	t = text(50, 0.9*max(hr), /data, hitstr)
	
	outfile = dir + '/' + base + '-' + 'resolutionhist.png'
	print,'Saving graphic to: ',outfile
	p1.save,outfile
	
end
