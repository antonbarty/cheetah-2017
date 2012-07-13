; Autorun.pro
; AB, 2012
;
; Automatically starts processing of CXI data runs once the XTC files have completed copying
;
; Looks for the last XTC file saved for this experiment
; Looks for the last processed data run in HDF5 directory
; If there are newer XTC files than the last processed data run:
; 	- Check whether the XTC files are still copying (xtc.inprogress)
;	- Checks whether job has already been submitted to the batch queue
;	- If data is ready to process, submit job to the LCLS batch queue system
;
; Calls script: scratch/cheetah/crystfinder-q 
; This called the script: scratch/cheetah/crystfinder 
; Uses autorun.ini as the cheetah .ini file
; Defaults to the version of cheetah in ~barty/c/cheetah/cheetah
; Changing any of these scripts will change how the autoprocessing works
;
; Could be implemented in sh, or perl, or python, or....

pro autorun

	
	h5dir = '/reg/d/psdm/cxi/cxi54312/scratch/hdf5/'
	xtcdir = '/reg/d/psdm/cxi/cxi54312/xtc/'
	command = '/reg/d/psdm/cxi/cxi54312/scratch/autorun/crystfinder-q'

	while 1 do begin
	
		;; XTCs available in directory
		xtc = file_search(xtcdir+'*.xtc')
		lastxtc = file_basename(xtc[n_elements(xtc)-1])
		lastxtcrun = strmid(lastxtc,6,4)
		

		;; Last directory processed 
		f = file_search(h5dir+'r*autorun')
		ff = file_basename(f[n_elements(f)-1])
		lastrun = strmid(ff,1,4)
		
		print,'Last run processed is: r', lastrun
		print, 'Last XTC run available: ', lastxtc

		
		if fix(lastrun) ge fix(lastxtcrun) then begin
			print, 'No new runs to process: (last XTC file is ', lastxtc, ')'
			wait, 30
			continue
		endif
		
				
		for run = lastrun+1, lastxtcrun do begin

			;; Generate name for the next run
			nextrun = run
			nextrunstr = string(nextrun)
			if nextrun lt 10 then nextrunstr = '000'+nextrunstr else $
			if nextrun lt 100 then nextrunstr = '00'+nextrunstr else $
			if nextrun lt 1000 then nextrunstr = '0'+nextrunstr
			nextrunstr = 'r'+nextrunstr
			nextrunstr = strcompress(nextrunstr, /remove_all)
		

			;; If no XTCs for this run, continue
			fx = file_search(xtcdir+'*'+nextrunstr+'*')
			if fx[0] eq '' then begin
				print,'No XTC files for run: ', nextrunstr
				wait, 10
				continue
			endif
		

			;; Are there XTC files still in progress copying?
			fp = file_search(xtcdir+'*'+nextrunstr+'*inprogress*')
			
			if fp[0] ne '' then begin
				print,'XTC copying still in progress for run: ', nextrunstr
				wait, 10
				continue
			endif


			;; Is there a job already in the queue?
			cmnd = 'bjobs -q psfehq -u all | grep autorun | grep ' + nextrunstr + ' | wc -l' 
			spawn, cmnd, result
			if result[0] ne '0' then begin
				print,'Run already submitted to batch queue: ', nextrunstr
				wait, 10
				continue
			endif
	

			;; OK - XTC files for next run are there, and not in progress copying
			cmnd = command + ' autorun.ini ' + nextrunstr
			
			print,'Spawning processing of run: ', nextrunstr
			print,'Command: ', cmnd
			spawn, cmnd


			wait, 10

		endfor
		
		
		
		wait, 30
	
	endwhile


end