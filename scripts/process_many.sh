#!/bin/bash
for (( i=1; i<=$#; i++ )); do
    eval RUN=\$$i
    echo "Initializing directory for $RUN"
    cd $H5DIR
    mkdir $RUN
    if [ -f cheetah_$RUN.ini ];
    then
	cp cheetah_$RUN.ini $RUN/cheetah.ini
    else
	cp cheetah.ini $RUN/
    fi
    if [ -f psana_$RUN.cfg ];
    then
	cp psana_$RUN.cfg $RUN/psana.cfg
    else
	cp psana.cfg $RUN/
    fi
    cd $RUN
    echo "#!/bin/bash" > process.sh
    echo "#SBATCH --job-name=Cth$RUN" >> process.sh
    echo "#SBATCH -N1" >> process.sh
    echo "#SBATCH -n20" >> process.sh
    echo "cd $H5DIR/$RUN" >> process.sh
    echo "psana -c psana.cfg  $XTCDIR/*$RUN*.xtc > cheetah_messages.txt" >> process.sh
    sbatch process.sh
    echo "Cheetah job submitted for $RUN"
    cd ..
done
