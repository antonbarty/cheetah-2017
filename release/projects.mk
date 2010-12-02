# List of projects (low level first)
ifneq ($(findstring ppc-rtems-rce,$(tgt_arch)),)
projects := rtems \
            pdsdata \
            rce \
            rceusr \
            rceapp
#            rcehw

rtems_use := /afs/slac.stanford.edu/g/npa/package/rtems/4.9.2
#rtems_use := /reg/g/pcds/package/rtems/4.9.2
#rtems_use := ~/rtems/4.9.2

pdsdata_use := release
rce_use    := release
rceusr_use := release
rceapp_use := release
rcehw_use  := release
endif

ifneq ($(strip $(findstring i386-linux,$(tgt_arch)) \
               $(findstring x86_64-linux,$(tgt_arch))),)
projects := pdsdata \
      acqiris \
      evgr \
      leutron \
      qt \
      qwt \
      epics \
      offlinedb \
      pvcam \
      pds \
      pdsapp \
      ami


epics_use   := /reg/g/pcds/package/external/epicsca-pcds-R1.0-r410
acqiris_use := /reg/g/pcds/package/external/acqiris_3.3a
evgr_use := /reg/g/pcds/package/external/evgr_V00-00-02
leutron_use := /reg/g/pcds/package/external/leutron_V00-00-00
qt_use := /reg/g/pcds/package/external/qt-4.3.4
qwt_use := /reg/g/pcds/package/external/qwt-5.1.1-wfopt
offlinedb_use := /reg/g/pcds/package/external/offlinedb-1.1.0
pvcam_use := /reg/g/pcds/package/external/pvcam2.7.1.7

pds_use := release
pdsdata_use := release
pdsapp_use := release
ami_use := release

endif
