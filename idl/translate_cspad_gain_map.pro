pro translate_cspad_gain_map, file, gainval=gainval, menu=menu, geometry=geometry


	;;
	;;	Default values
	;;
	if n_elements(file) eq 0 then begin
		print,'Please select a gain file in LCLS format'
		file = dialog_pickfile(title='Select a gain file', filter='*.txt')
		if file[0] eq '' then return
	endif
	
	if NOT KEYWORD_SET(gainval) then gainval= 6.87526

	
	
	;;
	;;	Dialog box
	;;

	if KEYWORD_SET(menu) then begin
		desc = [ 	'1, base, , column', $
					'0, text, '+string(file)+', label_left=CsPad gain file (.txt): , width=80, tag=file', $		
					'0, text, '+string(geometry)+', label_left=Geometry file (.h5 pixelmap): , width=80, tag=geometry', $		
					'2, float, '+string(gainval)+', label_left=Gain factor:, width=20, tag=gainval', $
					'1, base,, row', $
					'0, button, OK, Quit, Tag=OK', $
					'2, button, Cancel, Quit' $
		]		

		a = cw_form(desc, /column, title='Parameters for gain map')

		;; Only do this if OK is pressed (!!)
		if a.OK eq 1 then begin		
			file = a.file
			geometry = a.geometry
			gainval = a.gainval
		endif $
		else begin
			return
		endelse
	endif


	;; 
	;;	Read in LCLS gain file
	;; 	DAQ layout is one column of all ASICs stacked on top of each other 
	;; 	Call this array the "DAQ stack"
	;;
	print,'Reading LCLS gain map'
	lcls_gain = read_ascii(file, delimiter=' ')
	lcls_gain = lcls_gain.(0)
	s_in = size(lcls_gain, /dim)
	
	;;display, lcls_gain


	
	
	;;
	;;	Size of Cheetah gain file
	;;
	s_out = s_in
	s_out[0] *= 8
	s_out[1] /= 8
	
	asic_nx = s_out[0]/8
	asic_ny = s_out[1]/8
	
	cheetah_gain = fltarr(s_out)
	


	;; Loop over quadrants and  translate this into the Cheetah 8x8 array	
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
		
			;; Extract the two ASICs in this 2x1 from the DAQ array
			asic1 = lcls_gain[*,mstart:mstart+asic_ny-1]
			asic2 = lcls_gain[*,mstart+asic_ny:mstart+2*asic_ny-1]


			;; Place these two ASICs into the Cheetah array
		 	cheetah_gain[ax, ay]  = asic1
		 	cheetah_gain[ax+asic_nx, ay]  = asic2
		
		endfor
	endfor

	
	
	;;	
	;; Loop over quadrants and populate Cheetah-style array
	;;
	;for quad =0, 3 do begin
	;
	;	;; start position of this quadrant in the DAQ stack
	;	qstart = quad*8*asic_ny
	;
	;	;; Location of corner of this 2x1 in the Cheetah array
	;	ax = quad*(2*asic_nx)
	;	ay = 0
	;
	;	q = lcls_gain[*, qstart:qstart+8*asic_ny-1]
	;	
	;	gain[ax,ay] = q
	;
	;endfor


	;;display, cheetah_gain


	;;
	;; Turn 0 and 1  into actual gain numbers
	;;
	gainmap = fltarr(s_out[0],s_out[1])
	gainmap(where(cheetah_gain eq 0)) = gainval
	gainmap(where(cheetah_gain eq 1)) = 1


	;;
	;; Display the gain map (high value = high gain)
	;;	
	img = 1/gainmap
	if keyword_set(geometry) then begin
		geom = read_geometry(geometry)
		img = pixelremap(img, geom.x, geom.y, 110e-6)
	endif 
	scrolldisplay, img
	



	;;
	;; Save in Cheetah format
	;;
	print,'Saving gain map for Cheetah'
	outfile = dialog_pickfile(title='Gain map for Cheetah (.h5)', path=file_dirname(file), filter='*.h5')
	if outfile[0] ne '' then begin
		write_h5,outfile, gainmap
	endif
	
	


end