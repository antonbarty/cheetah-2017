pro hitrate, dir, buffer=buffer

	if n_elements(dir) eq 0 then begin
		print, 'Usage:'
		print, 'resolution, directory'
	endif

	if NOT KEYWORD_SET(buffer) then buffer=0

	;;
	;; Hit rate
	;;
	infile = dir + '/frames.txt'
	base = file_basename(dir)
	print,'Reading file: ', infile

	a = read_csv(infile, comment='#')
	file = a.field01
	hit = float(a.field04)
	npeaks = float(a.field10)
	nframes = n_elements(file)
	nhits = n_elements(where(hit eq 1))
	
	
	print,'Processing file: ', infile
	secavg = 5
	binsize = 120*secavg
	
	;; Point averages
	;l = long(n_elements(hit)/binsize)
	;hitrate = float(hit[0:l*binsize-1])
	;hitrate = rebin(hitrate,l)	
	;hitrate *= 100
	;time = 5*findgen(l)/60
	;p3 = plot(time,hitrate, symbol='D',  linestyle=' ', thick=3, yrange=[0,max(hitrate)], buffer=buffer)

	;; Line
	hitrate = smooth(hit,binsize,/edge)
	hitrate *= 100
	time = findgen(nframes)/(120.*60)
	p3 = plot(time,hitrate, thick=3, color='blue', yrange=[0,1.1*max(hitrate)], buffer=buffer)
	
	
	;; Fiddle with the plot
	p3.yrange = [0,1.1*max(hitrate)]
	p3.xtitle = 'Time (minutes)'
	p3.ytitle = 'Hit rate (%)'
	p3.title = dir + strcompress(string(': Hit rate (',secavg,' sec avg )'))
	
	;p3.sym_size=0.8
	;p3.sym_filled=1
	;p3.sym_fill_color='blue'

	str1 = strcompress(string(nhits, ' hits'))
	str2 = strcompress(string(nframes,' events'))
	str3 = strcompress(string('(',100.*nhits/nframes, '%)'))
	t = text(0.05*max(time), 0.95*max(hitrate), /data, str1)
	t = text(0.05*max(time), 0.9*max(hitrate), /data, str2)
	t = text(0.05*max(time), 0.85*max(hitrate), /data, str3)
	
	
	outfile = dir + '/' + base + '-' + 'hitrate.png'
	;;outfile = dir + '/hitrate.png'
	print,'Saving graphic to: ',outfile
	p3.save,outfile

	
end
