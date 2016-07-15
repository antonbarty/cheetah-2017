;;
;;	Automatically set up a new experiment in the specified directory, then return
;;
pro crawler_autosetup, dir

		;; Move to target directory, remembering current directory
		cd, dir, current=starting_dir
		
		
		;; Deduce experiment number, etc using de-referenced paths
		;;	This assumes the file path follows the pattern:   /reg/d/psdm/cxi/cxij4915/scratch/...
		print,'Deducing experiment and instrument from file paths...'
		spawn, 'pwd', path
		ss = strsplit(path, '/', /extract)
		
		if(ss[0] eq 'reg') then begin
			;; We are at SLAC
			ss[1] = 'd'
			ss[2] = 'psdm'
		endif
		instr = ss[3]
		expt = ss[4]
		xtcdir = '/' + strjoin(ss[0:4],'/') + '/xtc'
		userdir = '/' + strjoin(ss,'/') + '/'
		
		
		print,'    Full path: ', path
		print,'    Instrument: ', instr
		print,'    Experiment: ', expt
		print,'    XTC directory: ', xtcdir
		print,'    User directory: ', userdir
				
		
		
		
		;; User feedback
		desc = [ 	'1, base, , column, frame', $
					'0, label, Parameters', $
					'0, label, Instrument: '+instr+', left', $
					'0, label, Experiment: '+expt+', left', $
					'0, label, XTC directory: '+xtcdir+', left', $
					'2, label, Processing directory: '+userdir+', left', $
					'1, base, , column, frame', $
					'0, label, Options, left', $
					'2, button, Fix group permissions|Place configuration in /res, set_value=1, tag=option', $
					'1, base,, row', $
					'0, button, OK, Quit, Tag=OK', $
					'2, button, Cancel, Quit' $
		]		

		a = cw_form(desc, /column, title='Auto-detected parameters')

		;; Only do this if OK is pressed (!!)
		if a.OK ne 1 then begin		
			exit
		endif 

		
		
		;; Unpack template
		print,'>---------------------<'
		print,'Extracting template...'
		cmd = 'tar -xf /reg/g/cfel/cheetah/template.tar'
		print, cmd
		spawn, cmd
		
		;; Automatically fix permissions!
		if a.option[0] eq 1 then begin
			print,'>---------------------<'
			print,'Fixing permissions...'
			cmd = 'chgrp -R ' + expt + ' cheetah/'
			print, cmd
			spawn, cmd

			cmd = 'chmod -R  g+w cheetah/'
			print, cmd
			spawn, cmd
		endif
				
		;; Place configuration into /res
		if a.option[1] eq 1 then begin
			print,'>---------------------<'
			print,'Placing configuration into /res...'
			cmd = '/reg/g/cfel/cheetah/cheetah-stable/bin/make-labrynth'
			spawn, cmd
		endif
		
		;;
		;; Modify gui/crawler.config
		;;
		print,'>---------------------<'
		file = 'cheetah/gui/crawler.config'
		print,'Modifying ', file

		xtcsed = '\/' + strjoin(ss[0:4],'\/') + '\/xtc\/'
		cmd = "sed -i -r 's/(xtcdir=).*/\1"+xtcsed+"/'" + ' ' + file
		;print, cmd
		spawn, cmd

		;h5sed = '\/' + strjoin(ss,'\/') + '\/cheetah\/hdf5\/'
		;cmd = "sed -i -r 's/(hdf5dir=).*/\1"+h5sed+"/'" + ' ' + file
		;print, cmd
		;spawn, cmd
		
		print,'>-------------------------<'
		spawn, 'cat '+file
		print,'>-------------------------<'
		


		;;
		;; Modify process/process
		;;
		file = 'cheetah/process/process'
		print,'Modifying ', file

		expstr = '"' + expt + '"'
		cmd = "sed -i -r 's/(expt=).*/\1"+expstr+"/'" + ' ' + file
		;print, cmd
		spawn, cmd

		xtcsed = '"\/' + strjoin(ss[0:4],'\/') + '\/xtc\/"'
		cmd = "sed -i -r 's/(XTCDIR=).*/\1"+xtcsed+"/'" + ' ' + file
		;print, cmd
		spawn, cmd

		h5sed = '"\/' + strjoin(ss,'\/') + '\/cheetah\/hdf5\/"'
		cmd = "sed -i -r 's/(H5DIR=).*/\1"+h5sed+"/'" + ' ' + file
		;print, cmd
		spawn, cmd

		configsed = '"\/' + strjoin(ss,'\/') + '\/cheetah\/process\/"'
		cmd = "sed -i -r 's/(CONFIGDIR=).*/\1"+configsed+"/'" + ' ' + file
		;print, cmd
		spawn, cmd


		print,'>-------------------------<'
		spawn, 'cat '+file
		print,'>-------------------------<'
		
		
		;; Return to the main directory (circular, but it's what is expected from the calling code)
		cd, starting_dir

end

