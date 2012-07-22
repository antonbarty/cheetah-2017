;;
;;	idl_write_png
;;
;;	Simplified interface to the write_png procedure
;;
;;	There are two approaches:
;;		True colour image (3-bit per pixel)
;;		img=tvrd(true=1) 	produces [3,nx,ny] true-colour 
;;		write_png, filename, img
;;	OR
;;		256-level indexed colour image
;;		write_png, filename, data(byte), r,g,b
;;



PRO idl_write_png, filename, img, flip=flip, rv=rv, uint16=uint16

	ON_ERROR,1

	temp = float(img)
	if KEYWORD_SET(rv) then $
		temp = -temp
	if KEYWORD_SET(flip) then $
		temp=reverse(temp,2)

	;; 16-bit greyscale
	if keyword_set(uint16) then begin
		temp -= min(temp)
		temp *= 65535./max(temp)
		temp = uint(temp)
		write_png,filename,temp
	endif $
	
	;;	Colour table PNG
	else begin
		tvlct,r,g,b,/get
		cts=size(r,/dim)
		temp = bytscl(temp, top=cts[0]-1)	
		write_png,filename,temp,r,g,b	
	endelse
end

