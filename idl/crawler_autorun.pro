;
; crawler_autorun
; Anton Barty, 2013
;
; Monitors a given set of HDF5 directories and updates a text file accordingly 
; Could be implemented in sh, or perl, or python
;


pro crawler_autorun, hdf5dir, pattern

	xtcdir = '/reg/d/psdm/cxi/cxi69113/xtc/'
	hdf5dir = '/reg/d/psdm/cxi/cxi69113/scratch/hdf5'
	hdf5filter = 'r*-ab'
	datasetdb = 'datasets.txt'

	while 1 do begin

		crawler_xtc, xtcdir
		crawler_dataset
		crawler_hdf5, hdf5dir, hdf5filter
		crawler_merge
		
		spawn,'cat crawler.txt'
		print,'Pausing...'
		wait, 60	

	endwhile

end
	
