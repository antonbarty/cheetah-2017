pro make_cspad_gain_map, geometry, rlow, rhi, gainval, menu=menu


	;;
	;;	Default values
	;;
	if n_elements(geometry) eq 0 then begin
		print,'Please select a detector geometry'
		geometry = dialog_pickfile(title='Select a geometry file', filter='*.h5')
		if geometry[0] eq '' then return
	endif
	
	if n_elements(rlow) eq 0 then rlow=0
	if n_elements(rhi) eq 0 then rhi=300
	if n_elements(gainval) eq 0 then gainval= 6.87526
	
	
	
	;;
	;;	Dialog box
	;;

	if KEYWORD_SET(menu) then begin
		desc = [ 	'1, base, , column', $
					'0, text, '+string(geometry)+', label_left=Geometry file (.h5 pixelmap): , width=80, tag=geometry', $		
					'0, float, '+string(rlow)+', label_left=Inner low gain radius (pixels):, width=10, tag=rlow', $
					'0, float, '+string(rhi)+', label_left=Outer low gain radius (pixels):, width=10, tag=rhi', $
					'2, float, '+string(gainval)+', label_left=Gain factor:, width=20, tag=gainval', $
					'1, base,, row', $
					'0, button, OK, Quit, Tag=OK', $
					'2, button, Cancel, Quit' $
		]		

		a = cw_form(desc, /column, title='Parameters for gain map')

		;; Only do this if OK is pressed (!!)
		if a.OK eq 1 then begin		
			geometry = a.geometry
			rlow = a.rlow
			rhi = a.rhi
			gainval = a.gainval
		endif $
		else begin
			return
		endelse
	endif



	;;
	;; Read in geometry specification
	;;
	print,'Reading geometry'
	geom = read_geometry(geometry)
	s = size(geom.x,/dim)
	r = sqrt(geom.x^2 + geom.y^2) / 110e-6

	;; Size of one ASIC (pixelmap is 8x8 array of ASICs)
	ss = s/8			




	print,'Creating binary gain map'
	;; 0 or 1 for low or high gain respectively
	gain = intarr(s[0],s[1])
	gain[*] = 1
	gain(where(r ge rlow AND r lt rhi)) = 0


	print,'Creating map of gain values for gain correction in Cheetah'
	;; These are the actual gain values
	gainmap = fltarr(s[0],s[1])
	gainmap(where(gain eq 0)) = gainval
	gainmap(where(gain eq 1)) = 1


	;; Display the gain map (high value = high gain)
	img = 1/gainmap
	img = pixelremap(img, geom.x, geom.y, 110e-6)
	scrolldisplay, img

	

	;; DAQ layout is one column of all ASICs stacked on top of each other 
	;; Call this array the "DAQ stack"
	print, 'Reformatting binary gain map for upload to cspad detector'
	gain2 = intarr(ss[0],8*(8*ss[1]))


	;; Dimensions of each ASIC
	asic_nx = ss[0]
	asic_ny = ss[1]


	;; Loop over quadrants
	for quad =0, 3 do begin

		;; start position of this quadrant in the DAQ stack
		qstart = quad*8*(2*asic_ny)

		;; Loop over 2x1 modules within this quadrant
		for module = 0, 7 do begin
	
			;; Start position of this 2x1 module in the DAQ stack
			mstart = qstart + module*(2*asic_ny)

			;; Location of corner of this 2x1 in the Cheetah array
			ax = quad*(2*asic_nx)
			ay = module*asic_ny
		
			;; Extract the two ASICs in this 2x1 from the Cheetah array
			asic1 = gain[ax:ax+asic_nx-1, ay:ay+asic_ny-1] 
			asic2 = gain[ax+asic_nx:ax+2*asic_nx-1, ay:ay+asic_ny-1] 
		
			;; Place these two ASICs into the DAQ stack
			gain2[0,mstart] = asic1
			gain2[0,mstart+asic_ny] = asic2 
		
		endfor
	endfor



	;;
	;; Save in Cheetah format
	;;
	print,'Saving gain map for Cheetah'
	outfile = dialog_pickfile(title='Gain map for Cheetah (.h5)', filter='*.h5')
	if outfile[0] ne '' then begin
		write_h5,outfile, gainmap
	endif
	
	

	;;
	;; Save in Sebastien format
	;;
	print,'Saving gain map for CSPAD detector'
	outfile = dialog_pickfile(title='Gain map to send to CSPAD (.txt)', filter='*.txt')
	if outfile[0] ne '' then begin
		sg = size(gain2,/dim)
		openw, lun, outfile, /get

		for i=0L, sg[1]-1 do begin
			line = gain2[*,i]
			str = strjoin(strcompress(string(line),/remove_all),' ')
			writeu, lun, str
			printf, lun, ''
		endfor
	
		close, lun
		free_lun, lun
	endif



end