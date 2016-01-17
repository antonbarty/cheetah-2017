;
; crawler_autorun
; Anton Barty, 2013
;
; Monitors a given set of HDF5 directories and updates a text file accordingly 
; Could be implemented in sh, or perl, or python
;

pro crawler_autorun_event, ev
  	WIDGET_CONTROL, ev.top, GET_UVALUE=pState
  	sState = *pState

	;;
	;; Main event processing case statement
	;;
	case ev.id of 
		sState.mbrun : begin
			widget_control, sState.mbrun, sensitive=0

			;; Check if lock file present
			if file_test('crawler.lock') then begin
				print,'Crawler already running somewhere:
				print,'Lock file "gui/crawler.lock" detected (delete this file if an error)'
				widget_control, sState.text, set_value='Lock file gui/crawler.lock detected'
				wait, 1
				widget_control, sState.text, set_value='Waiting for next scan event'
				widget_control, sState.mbrun,  timer=60
				widget_control, sState.mbrun, sensitive=1
				return
			endif

			spawn, 'touch crawler.lock'

			widget_control, sState.text, set_value='XTC files'
			crawler_xtc, sState.xtcdir
			;widget_control, sState.text, set_value='Datasets'
			;crawler_dataset, sState.datasetdb
			widget_control, sState.text, set_value='Scanning HDF5 directories'
			crawler_hdf5, sState.hdf5dir, sState.hdf5filter
			widget_control, sState.text, set_value='Scanning indexing results'
			crawler_crystfel, '../indexing'
			widget_control, sState.text, set_value='Merging'
			crawler_merge

			;;spawn,'cat crawler.txt'
			widget_control, sState.text, set_value='Waiting for next scan event'
			widget_control, sState.mbrun,  timer=60
			widget_control, sState.mbrun, sensitive=1
			file_delete, 'crawler.lock'
		end
		
		else : begin 
			help, ev
		end
			
		
	endcase

end


pro crawler_autorun, xtcdir=xtcdir, hdf5dir=hdf5dir, hdf5filter=hdf5filter, datasetdb=datasetdb

	if NOT KEYWORD_SET(xtcdir) then $
		xtcdir = '/reg/d/psdm/cxi/cxi69113/xtc/'
	if NOT KEYWORD_SET(hdf5dir) then $
		hdf5dir = '/reg/d/psdm/cxi/cxi69113/scratch/hdf5'
	if NOT KEYWORD_SET(hdf5filter) then $
		hdf5filter = 'r*'
	if NOT KEYWORD_SET(datasetdb) then $
		datasetdb = 'datasets.txt'


	;;
	;;	Check whether directories exist (and have the right permissions)
	;;
	t = file_test(xtcdir)
	if t ne 1 then begin
		str = string('XTC directory does not exist , or you do not have permission to access it. Please check your crawler.config file and restart. (', xtcdir, ')')
		r = dialog_message(str,/info)
		return
	endif

	t = file_test(hdf5dir)
	if t ne 1 then begin
		str = string('HDF5 directory does not exist. Please check your crawler.config file and restart (', hdf5dir, ')')
		r = dialog_message(str,/info)
		return
	endif

	t = file_test('crawler.txt', /write)
	if t ne 1 then begin
		str = string('crawler.txt file is not writeable.  Crawler will not work.  Please check and restart.')
		r = dialog_message(str,/info)
		return
	endif




	;;
	;;	Configure the GUI
	;;
	base = widget_base(/row, group=group, /tlb_size_events)	;mbar=bar, 
	widget_control, base, /managed

	mbrun = widget_button(base, value='Refresh')

	text = widget_label(base, xsize=200, value='Started')


	widget_control, base, base_set_title='Cheetah crawler'
	widget_control, base, /realize


	;;
	;;	Info structure
	;;
	sState = { $
			base : base, $
			mbrun : mbrun, $
			text : text,  $

			xtcdir : xtcdir, $
			hdf5dir : hdf5dir, $
			hdf5filter : hdf5filter, $
			datasetdb : datasetdb $
	}				
	pstate = ptr_new(sState)
	WIDGET_CONTROL, base, SET_UVALUE=pState

   	XMANAGER, 'Test', base, event='crawler_autorun_event', /NO_BLOCK
	widget_control, (*pState).mbrun,  timer=1


end
	
