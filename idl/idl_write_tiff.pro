;;	PRO write_tiff
;;	(c) A.Barty and J.Walford, 1998
;;
;;	Writes greyscale TIFF images to file, as well as colour TIFFs.
;;	Colour TIFFs can either be palette mapped (ie: using the current WAVE
;;	colour map) or RGB (ie: copy the current CT into an RGB image).
;;	Palette TIFFs are smaller, but RGB images are more general.
;;
;;	Images are inverted by default, so that they print the same way they
;;	appear on screen: WAVE's (0,0) pixel is in the bottom left, but TIFF
;;	pixel (0,0) is in the top left.
;;
;; 	Images can be written in reverse video, which is useful for images copied
;;	from the Z-buffer which sould otherwise have a black background.
;;	I'm not sure that compression does much unless you have a binary image
;;	such as a graph copied from the zbuffer.
;;

PRO idl_write_tiff, filename, img, flip=flip, rv=rv, compress=compress, $
				rgb=rgb, h=h, quiet=quiet

	ON_ERROR,1

	if KEYWORD_SET(h) then begin
		print,'Usage:  iidl_write_tiff, filename, data, [options]'
		print,'IDL version.'
		print,'Options:'
		print,'        flip   Reflect image about vertical'
		print,'        rv     Optput in reverse video'
		print,'        rgb    Write as RGB TIFF'
		print,'               (default is current IDL colour palette)'
		return
	endif

	temp = float(img)
	if KEYWORD_SET(rv) then $
		temp = -temp;
	if NOT KEYWORD_SET(flip) then $
		temp=reverse(temp,2)

	temp = bytscl(temp)

	if KEYWORD_SET(rgb) then begin
		;print,'Copying current RGB colour palette and writing to RGB TIFF file'
		tvlct,r,g,b,/get
		cts=size(r)
		ct = bytarr(3,cts(1))
		ct(0,*) = r
		ct(1,*) = g
		ct(2,*) = b
		ct = congrid(ct,3,256,/inter)
		s=size(temp)
		outarr=bytarr(3,s(1),s(2))
		outarr(0,*,*) = ct(0,temp(*,*))
		outarr(1,*,*) = ct(1,temp(*,*))
		outarr(2,*,*) = ct(2,temp(*,*))
		write_tiff,filename,outarr
	endif $
	else begin
		;print,'Copying current colour palette...'
		tvlct,r,g,b,/get
		cts=size(r)
		ct = bytarr(3,cts(1))
		ct(0,*) = r
		ct(1,*) = g
		ct(2,*) = b
		ct = congrid(ct,3,256,/inter)
		write_tiff,filename,temp,red=r,green=g,blue=b
	endelse
end

