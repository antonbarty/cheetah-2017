# List of projects (low level first)

#
# RTEMS
#
ifneq ($(findstring ppc-rtems-rce,$(tgt_arch)),)
projects := rtems \
            pdsdata \
            rce \
            rceusr \
            rceapp
#            rcehw

#rtems_use := /afs/slac.stanford.edu/g/npa/package/rtems/4.9.2
#rtems_use := /reg/g/pcds/package/rtems/4.9.2
rtems_use := ~/rtems/4.9.2

pdsdata_use := release
rce_use    := release
rceusr_use := release
rceapp_use := release
rcehw_use  := release
endif

#
# 32-bit linux
#
ifneq ($(findstring i386-linux,$(tgt_arch)),)
projects := pdsdata \
      acqiris \
      evgr \
      leutron \
      edt \
      qt \
      qwt \
      epics \
      offlinedb \
      pvcam \
      relaxd \
      fli \
      ami 

epics_use   := /reg/g/pcds/package/external/epicsca-pcds-R1.0-r410
acqiris_use := /reg/g/pcds/package/external/acqiris_3.3a
evgr_use := /reg/g/pcds/package/external/evgr_V00-00-02
leutron_use := /reg/g/pcds/package/external/leutron_V00-00-00
edt_use := /reg/g/pcds/package/external/edt
relaxd_use := /reg/g/pcds/package/external/relaxd-1.8.0
qt_use := /reg/g/pcds/package/external/qt-4.3.4
qwt_use := /reg/g/pcds/package/external/qwt-5.1.1-wfopt
offlinedb_use := /reg/g/pcds/package/external/offlinedb-1.3.0
pvcam_use := /reg/g/pcds/package/external/pvcam2.7.1.7
fli_use   := /reg/g/pcds/package/external/fli-dist-1.71

#pgpcard_use := /reg/g/pcds/package/pgpcard

pds_use := release
pdsdata_use := release
pdsapp_use := release
ami_use := release
rce_use := release
rceusr_use := release
rceapp_use := release

endif

#
# 64-bit linux
#
ifneq ($(findstring x86_64-linux,$(tgt_arch)),)
projects := pdsdata \
      qt \
      qwt \
      edt \
      offlinedb \
      leutron \
      python \
      libraw1394 \
      libdc1394 \
      fli \
      ami
qt_use      := /reg/g/pcds/package/external/qt-4.3.4
qwt_use     := /reg/g/pcds/package/external/qwt-5.1.1-wfopt
python_use  := /reg/g/pcds/package/python-2.5.2
libraw1394_use := /reg/g/pcds/package/external/libdc1394
libdc1394_use := /reg/g/pcds/package/external/libdc1394
offlinedb_use := /reg/g/pcds/package/external/offlinedb-1.3.0
edt_use := /reg/g/pcds/package/external/edt
leutron_use := /reg/g/pcds/package/external/leutron_V00-00-00
fli_use   := /reg/g/pcds/package/external/fli-dist-1.71

pds_use := release
pdsdata_use := release
pds_use     := release
pdsapp_use  := release
ami_use     := release

endif
