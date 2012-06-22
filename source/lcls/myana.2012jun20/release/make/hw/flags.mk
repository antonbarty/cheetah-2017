# Architecture flags
# ------------------
arch_tgts := xc4vfx12 xc4vfx20 xc4vfx60
arch_opts :=

define arch_opt_template
arch_tgts += $$(addsuffix -$(1),$$(arch_tgts))
endef
$(foreach opt,$(arch_opts),$(eval $(call arch_opt_template,$(opt))))

ifneq ($(findstring xc4vfx12,$(tgt_arch)),)
PLATFORM := xc4vfx12-ff668-10
endif

ifneq ($(findstring xc4vfx20,$(tgt_arch)),)
PLATFORM := xc4vfx20-ff672-10
endif

ifneq ($(findstring xc4vfx60,$(tgt_arch)),)
PLATFORM := xc4vfx60-ff1152-11
endif

BITGENOPTS := -g DebugBitstream:No -g Binary:no -g CRC:Enable \
-g ConfigRate:26 -g CclkPin:PullUp -g M0Pin:PullUp -g M1Pin:PullUp \
-g M2Pin:PullUp -g ProgPin:PullUp -g DonePin:PullUp -g InitPin:Pullup \
-g CsPin:Pullup -g DinPin:Pullup -g BusyPin:Pullup -g RdWrPin:Pullup \
-g TckPin:PullUp -g TdiPin:PullUp -g TdoPin:PullUp -g TmsPin:PullUp \
-g UnusedPin:PullDown -g UserID:0xDEADBEEF -g DCIUpdateMode:AsRequired \
-g StartUpClk:CClk -g DONE_cycle:4 -g GTS_cycle:5 -g GWE_cycle:6 \
-g LCK_cycle:NoWait -g Match_cycle:Auto -g Security:None -g Persist:No \
-g DonePipe:No -g DriveDone:No -g Encrypt:No

TIME := /usr/bin/time -f "(time %E: usr %U sys %S CPU %P i/o %I/%O)"
ACEGEN := $(TIME) xmd -tcl genace.tcl -jprog
BITGEN := $(TIME) bitgen -intstyle silent -d -w $(BITGENOPTS)
TWRGEN := $(TIME) trce -intstyle silent -v 50 -l 500 -u 100 
PARGEN := $(TIME) par -intstyle silent -w -ol high -xe n -t 2
MAPGEN := $(TIME) map -intstyle silent -cm speed -ol high -p $(PLATFORM) -pr b -k 4 -c 100
NGDGEN := $(TIME) ngdbuild -intstyle silent -dd ngo -p $(PLATFORM)
XSTGEN := $(TIME) xst -intstyle silent
OPTGEN := $(RELEASE_DIR)/make/hw/optgen.py
PRJGEN := $(RELEASE_DIR)/make/hw/prjgen.py
TCLGEN := $(RELEASE_DIR)/make/hw/tclgen.py
SYNPLF := $(TIME) synplify_pro -batch
