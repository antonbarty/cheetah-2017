PRO display,a,quiet=quiet,wid=wid,title=title

    ;; You are free to use and modify this program provided the
    ;; following message is displayed when the code is run
	;if NOT KEYWORD_SET(quiet) then $
    ;	print,'display: (c) Anton Barty, 1999'

	oldwin = !d.window
	s=size(a)

	if KEYWORD_SET(wid) then $
		w_id = wid $
	else $
		w_id = 1

	if s(0) ne 2 then begin
		print,'Not a 2D array'
		return
	endif

	if KEYWORD_SET(title) then $
		win_title = title+' ['+strtrim(string(w_id),2)+']' $
	else $
		win_title = 'Image ['+strtrim(string(w_id),2)+']'

	;; If there are no windows available create one 
	if !d.window eq -1 then $	
		window,w_id ,title=win_title,xsize=s(1),ysize=s(2),retain=3 $
	else begin
		;; Does window w_id  exist?
		device,window_state=wins
		if wins(w_id) eq 1 then $
			wset,w_id $
		else $
			window,w_id,title=win_title,xsize=s(1),ysize=s(2),retain=3  

		;; No Window1 after wset --> create it
		if !d.window ne w_id then $	
			window,w_id,title=win_title,xsize=s(1),ysize=s(2),retain=3 
	endelse

	;; Window wrong size --> resize it
	if (!d.X_size ne s(1)) OR (!d.Y_size ne s(2)) then $
		window,w_id,title=win_title,xsize=s(1),ysize=s(2),retain=3

	;;; window,/free,title='Image',xsize=s(1),ysize=s(2),retain=3

	tvscl,a

	;; Reset output to old window
	if oldwin ne -1 then $
		wset,oldwin
end

