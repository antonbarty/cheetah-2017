pro translate_cspad_gain_map, file, gainval, menu=menu


	;;
	;;	Default values
	;;
	if n_elements(file) eq 0 then begin
		print,'Please select a gain file in LCLS format'
		file = dialog_pickfile(title='Select a gain file', filter='*.txt')
		if file[0] eq '' then return
	endif
	
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
	;;	Read in LCLS gain file
	;;
	lcls_gain = read_ascii(file, delimiter=' ')
	lcls_gain = lcls_gain.(0)
	s_in = size(lcls_gain, /dim)
	
	display, lcls_gain
	
	
	
	;;
	;;	Size of Cheetah gain file
	;;
	s_out = s_in
	s_out[0] *= 4
	s_out[1] /= 4
	
	asic_nx = s_out[0]/8
	asic_ny = s_out[1]/8
	
	
	gain = fltarr(s_out)
	

	
	
	
	;;	
	;; Loop over quadrants and populate Cheetah-style array
	;;
	for quad =0, 3 do begin

		;; start position of this quadrant in the DAQ stack
		qstart = quad*8*asic_ny

		;; Location of corner of this 2x1 in the Cheetah array
		ax = quad*(2*asic_nx)
		ay = 0

		q = lcls_gain[*, qstart:qstart+8*asic_ny-1]
		
		gain[ax,ay] = q

	endfor

	display, gain



	;;
	;; Turn 0 and 1  into actual gain numbers
	;;
	gainmap = fltarr(s_out[0],s_out[1])
	gainmap(where(gain eq 0)) = gainval
	gainmap(where(gain eq 1)) = 1


	;;
	;; Display the gain map (high value = high gain)
	;;
	
	;img = 1/gainmap
	;img = pixelremap(img, geom.x, geom.y, 110e-6)
	;scrolldisplay, img

	


	;;
	;; Save in Cheetah format
	;;
	print,'Saving gain map for Cheetah'
	outfile = dialog_pickfile(title='Gain map for Cheetah (.h5)', filter='*.h5')
	if outfile[0] ne '' then begin
		write_h5,outfile, gainmap
	endif
	
	



end