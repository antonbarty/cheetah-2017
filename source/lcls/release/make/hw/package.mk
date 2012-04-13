# Package level makefile
# ----------------------
Makefile:;

# Symbols
# -------
SHELL := /bin/sh
RM    := rm -f
MV    := mv -f

pwd := $(shell pwd)
cwd := $(call reverse,$(subst /, ,$(pwd)))
pkg_name := $(word 1,$(cwd))
prj_name := $(word 2,$(cwd))

# Procedures
# ----------
pkgdir := $(RELEASE_DIR)/build/$(prj_name)/$(pkg_name)/$(tgt_arch)
blddir := $(pkgdir)/xil
xstdir := $(pkgdir)/xst
syndir := $(pkgdir)/syn
simdir := $(pkgdir)/sim
dirs   := $(pkgdir) $(blddir) $(xstdir) $(syndir) $(simdir)

ifneq ($(bcores),)
sdpaths := $(addprefix -sd , $(bcores))
endif

ifneq ($(guidefile),)
   GUIDEFLAGS := -gf $(guidefile)
endif

getstms = $(strip $(basename $(notdir $(1))))
edfs = $(addsuffix .edf, $(addprefix $(1), $(call getstms,$(2))))
ngcs = $(addsuffix .ngc, $(addprefix $(1), $(call getstms,$(2))))
opts = $(addsuffix .opt, $(addprefix $(1), $(call getstms,$(2))))
prjs = $(addsuffix .prj, $(addprefix $(1), $(call getstms,$(2))))
done = $(addsuffix .done,$(addprefix $(1), $(call getstms,$(2))))
getsrcfiles = $(shell srcs=`cut -f 3 -d ' ' $(1)`; echo $$srcs;)

define wrapper_template
  stem_$(1) := $$(call getstms,$(1))
  srcfiles_$$(stem_$(1)) := $$(addprefix $$(pwd)/,$$(call getsrcfiles,$(1)))
  useiobuf_$$(stem_$(1)) := NO
  src_$$(stem_$(1)) := $(1)
  lso_$$(stem_$(1)) := $$(wildcard $$(patsubst %.src,%.lso,$(1)))

$$(xstdir)/$$(stem_$(1)).ngc: $$(xstdir)/$$(stem_$(1)).opt $$(xstdir)/$$(stem_$(1)).prj $$(srcfiles_$$(stem_$(1)))

$$(xstdir)/$$(stem_$(1)).prj: $$(src_$$(stem_$(1))) $$(lso_$$(stem_$(1)))
$$(xstdir)/$$(stem_$(1)).opt: $$(xstoptions)
endef


define synplify_template
  stem_$(1) := $$(call getstms,$(1))
  srcfiles_$$(stem_$(1)) := $$(addprefix $$(pwd)/,$$(call getsrcfiles,$(1)))
  disiobuf_$$(stem_$(1)) := 1
  src_$$(stem_$(1)) := $(1)

$$(syndir)/$$(stem_$(1)).edf: $$(syndir)/$$(stem_$(1)).prj $$(sdc_$$(stem_$(1))) $$(srcfiles_$$(stem_$(1)))

$$(syndir)/$$(stem_$(1)).prj: $$(synoptions) $$(src_$$(stem_$(1)))
endef

define design_template
  ace_$(1)  := $$(pkgdir)/$(1)_ace.ace
  bit_$(1)  := $$(pkgdir)/$(1).bit
  twr_$(1)  := $$(pkgdir)/$(1).twr
  ncd_$(1)  := $$(pkgdir)/$(1).ncd
  map_$(1)  := $$(pkgdir)/$(1)_map.ncd
  ngd_$(1)  := $$(pkgdir)/$(1).ngd
  ngcs_$(1) := $$(call ngcs,$$(xstdir)/,$$(wrappers_$(1)) $$(topwrapp_$(1)))
  opts_$(1) := $$(call opts,$$(xstdir)/,$$(wrappers_$(1)) $$(topwrapp_$(1)))
  prjs_$(1) := $$(call prjs,$$(xstdir)/,$$(wrappers_$(1)) $$(topwrapp_$(1)))
  edfs_$(1) := $$(call edfs,$$(syndir)/,$$(synplifs_$(1)) $$(topsynpl_$(1)))
  tcls_$(1) := $$(call prjs,$$(syndir)/,$$(synplifs_$(1)) $$(topsynpl_$(1)))
  acefiles  += $$(ace_$(1))
  bitfiles  += $$(bit_$(1))
  twrfiles  += $$(twr_$(1))
  ncdfiles  += $$(ncd_$(1))
  mapfiles  += $$(map_$(1))
  ngdfiles  += $$(ngd_$(1))
  edffiles  += $$(edfs_$(1)) $$(ngcs_$(1))
  prjfiles  += $$(tcls_$(1)) $$(prjs_$(1)) $$(opts_$(1))
  ucf_$(1)  := $$(pwd)/$$(ucf_$(1))
ifneq ($$(bmm_$(1)),)
  ubmm_$(1) := $$(blddir)/$(1).bmm
  obmm_$(1) := -bm $$(notdir $$(bmm_$(1)))
$$(ubmm_$(1)): $$(bmm_$(1))
endif
ifneq ($$(elf_$(1)),)
  oelf_$(1) := -bd $$(elf_$(1))
endif
ifneq ($$(topsynpl_$(1)),)
  edn_$(1)  := $$(word $$(words $$(edfs_$(1))),$$(edfs_$(1)))
else
  edn_$(1)  := $$(word $$(words $$(ngcs_$(1))),$$(ngcs_$(1)))
endif
  useiobuf_$(1) := YES
  disiobuf_$(1) := 0

$$(ace_$(1)): $$(bit_$(1))

$$(bit_$(1)): $$(ncd_$(1)) $$(elf_$(1))

$$(twr_$(1)): $$(ncd_$(1))

$$(ncd_$(1)): $$(map_$(1))

$$(map_$(1)): $$(ngd_$(1))

$$(ngd_$(1)): $$(edfs_$(1)) $$(ngcs_$(1)) $$(ubmm_$(1)) $$(ucf_$(1))
endef

$(foreach dsg,$(designs),$(foreach wrapper,$(wrappers_$(dsg)) $(topwrapp_$(dsg)),$(eval $(call wrapper_template,$(wrapper)))))
$(foreach dsg,$(designs),$(foreach syn,$(synplifs_$(dsg)) $(topsynpl_$(dsg)),$(eval $(call synplify_template,$(syn)))))
$(foreach dsg,$(designs),$(eval $(call design_template,$(dsg))))


define simwrapper_template
  stem_$(1) := $$(call getstms,$(1))
  srcfiles_$$(stem_$(1)) := $$(addprefix $$(pwd)/,$$(call getsrcfiles,$(1)))
  src_$$(stem_$(1)) := $(1)
  lso_$$(stem_$(1)) := $$(wildcard $$(patsubst %.src,%.lso,$(1)))

$$(simdir)/$$(stem_$(1)).done: $$(simdir)/$$(stem_$(1)).prj $$(srcfiles_$$(stem_$(1)))

$$(simdir)/$$(stem_$(1)).prj: $$(src_$$(stem_$(1))) $$(lso_$$(stem_$(1)))
endef

define simtoplevel_template
  stem_$(1) := $$(call getstms,$(1))
  srcfiles_$$(stem_$(1)) := $$(addprefix $$(pwd)/,$$(call getsrcfiles,$(1)))
  src_$$(stem_$(1)) := $(1)
  lso_$$(stem_$(1)) := $$(wildcard $$(patsubst %.src,%.lso,$(1)))
  done_$$(stem_$(1)) := $$(call done,$$(simdir)/,$$(wrappers_$$(stem_$(1))))

$$(simdir)/$$(stem_$(1)).done: $$(simdir)/$$(stem_$(1)).prj $$(srcfiles_$$(stem_$(1))) $$(done_$$(stem_$(1)))

$$(simdir)/$$(stem_$(1)).prj: $$(src_$$(stem_$(1))) $$(lso_$$(stem_$(1)))
endef

define simulation_template
  sim_$(1)  := $$(pkgdir)/$(1).sim
  done_$(1) := $$(call done,$$(simdir)/,$$(wrappers_$(1)) $$(topwrapp_$(1)))
  simfiles  += $$(sim_$(1))

$$(sim_$(1)): $$(simdir)/.synopsys_vss.setup $$(done_$(1))
endef

$(foreach sim,$(simulations),$(foreach wrapper,$(wrappers_$(sim)),$(eval $(call simwrapper_template,$(wrapper)))))
$(foreach sim,$(simulations),$(eval $(call simtoplevel_template,$(topwrapp_$(sim)))))
$(foreach sim,$(simulations),$(eval $(call simulation_template,$(sim))))

# Rules
# -----
.SUFFIXES:  # Kills all implicit rules

# Kill rules to remake source files, there really isn't anyway to do
# this and this just adds time to make's execution and verbage to
# debug output.
%.vhd	:;
%.v	:;
%.src	:;
%.lso	:;
%.ucf	:;

rules := all ace bit twr ncd map ngd edf prj sim dir clean cleansim print

.PHONY: $(rules)

all: ace sim;
ace: $(acefiles);
bit: $(bitfiles);
twr: $(twrfiles);
ncd: $(ncdfiles);
map: $(mapfiles);
ngd: $(ngdfiles);
edf: $(edffiles);
prj: $(prjfiles);
sim: $(simfiles);
dir: $(dirs);

print:
	@echo	"pkgdir = $(pkgdir)"
	@echo	"ace    = $(acefiles)"
	@echo	"bit    = $(bitfiles)"
	@echo	"ncd    = $(ncdfiles)"
	@echo   "map    = $(mapfiles)"
	@echo	"ngd    = $(ngdfiles)"
	@echo	"edf    = $(edffiles)"
	@echo	"prj    = $(prjfiles)"
	@echo	"sim    = $(simfiles)"


clean:
	@echo "[CL] Remove $(pkgdir)"
	$(quiet)rm -rf $(pkgdir)

cleansim:
	@echo "[CL] Remove $(simdir)"
	$(quiet)rm -rf $(simdir)


# Directory structure
$(dirs):
	$(quiet)mkdir -p $@


# Xilinx
$(blddir)/%.bmm:
# Unfortunately the bmm file is needed under blddir by bitgen (through
# data2mem) since its location cannot be specified as bitgen argument
	@echo "[BM] Copy BMM file to build directory for design $*"
	$(quiet)cp $(bmm_$*) $(blddir)

$(pkgdir)/%.ngd:
	@echo "[GD] Generate NGD file for design $*"
	$(quiet)cd $(blddir) && \
	$(NGDGEN) $(sdpaths) $(obmm_$*) -uc $(ucf_$*) $(edn_$*) $*.ngd && \
	mv $*.ngd ../

$(pkgdir)/%_map.ncd:
	@echo "[MP] Generate MAP file for design $*"
	$(quiet)cd $(blddir) && \
	$(MAPGEN) $(GUIDEFLAGS) -o $*_map.ncd ../$*.ngd $*.pcf && mv $*_map.ncd ../

$(pkgdir)/%.ncd:
	@echo "[PR] Place and route design $*"
	$(quiet)cd $(blddir) \
	&& $(PARGEN) $(GUIDEFLAGS) ../$*_map.ncd $*.ncd $*.pcf && mv $*.ncd ../

$(pkgdir)/%.twr: 
	@echo "[BT] Generate timing report for design $*"
	$(quiet)cd $(blddir) && \
        $(TWRGEN) -xml $* ../$*.ncd -o $*.twr $*.pcf && mv $*.twr ../

$(pkgdir)/%.bit:
	@echo "[BT] Generate BIT file for design $*"
	$(quiet)cd $(blddir) && \
	$(BITGEN) $(oelf_$*) ../$*.ncd $*.bit && mv $*.bit ../

$(pkgdir)/%_ace.ace:
	@echo "[AC] Generate ACE file for design $*"
	$(quiet)cd $(blddir) && \
	$(ACEGEN) -hw ../$*.bit -ace $*_ace.ace && mv $*_ace.ace ../


# Xilinx XST
$(xstdir)/%.prj:
	@echo "[XP] Generate list of soure files for $*"
	$(quiet)$(PRJGEN) $(scores) $(pwd) $@ $(src_$*) $(lso_$*)

$(xstdir)/%.opt:
	@echo "[XO] Generate synthesis options file for $*"
	$(quiet)$(OPTGEN) $(xstoptions) $* $(useiobuf_$*) $(PLATFORM) $@

$(xstdir)/%.ngc:
	@echo "[XD] Compile NGC file for $*"
	$(quiet)cd $(xstdir) && rm -rf $* && $(XSTGEN) -ifn $*.opt


# Synplify
$(syndir)/%.prj:
	@echo "[SL] Generate list of soure files for $*"
	$(quiet)$(TCLGEN) $(synoptions) $* $(disiobuf_$*) $(pwd) $(sdc_$*) $(src_$*) $@

$(syndir)/%.edf:
	@echo "[SD] Compile EDF file for $*"
	$(quiet)cd $(syndir) && rm -rf $* && $(SYNPLF) $*.prj


# Synopsys
$(simdir)/%.prj:
	@echo "[SP] Generate list of simulation soure files for $*"
	$(quiet)$(PRJGEN) $(scores) $(pwd) $@ $(src_$*) $(lso_$*)

$(simdir)/%.done:
	@echo "[SC] Compile SIM file for $*"
	$(quiet)cd $(simdir) && \
	vhdlan -nc $(call getsrcfiles,$(simdir)/$*.prj) && touch $*.done

$(simdir)/.synopsys_vss.setup: $(simoptions)
	@echo "[ST] Copy simulation options file $<"
	$(quiet)cp $< $@

$(pkgdir)/%.sim:
	@echo "[SE] Generate simulation executable $*"
	$(quiet)cd $(simdir) && rm -rf $@.db.dir \
	&& scs -nc -debug -time "ps" -time_res "1ps" $* -exe $@
