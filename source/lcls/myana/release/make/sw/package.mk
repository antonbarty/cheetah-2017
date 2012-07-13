# Package level makefile
# ----------------------
Makefile:;

# Symbols
# -------
SHELL := /bin/bash
RM    := rm -f
MV    := mv -f
empty :=
space := $(empty) $(empty)

cwd := $(call reverse,$(subst /, ,$(shell pwd)))
pkg_name := $(word 1,$(cwd))
prj_name := $(word 2,$(cwd))

# Defines which directories are being created by this makefile
libdir  := $(RELEASE_DIR)/build/$(prj_name)/lib/$(tgt_arch)
bindir  := $(RELEASE_DIR)/build/$(prj_name)/bin/$(tgt_arch)
objdir  := $(RELEASE_DIR)/build/$(prj_name)/obj/$(tgt_arch)/$(pkg_name)
depdir  := $(RELEASE_DIR)/build/$(prj_name)/dep/$(tgt_arch)/$(pkg_name)
prod_dirs := $(strip $(bindir) $(libdir))
temp_dirs  = $(strip $(sort $(foreach o,$(depends) $(objects),$(dir $(o)))))


# Procedures
# ----------

# Define some procedures and create (different!) rules for libraries
# and targets. Note that 'eval' needs gmake >= 3.80.
libraries :=
targets   :=
objects   := 
depends   :=
getobjects = $(strip \
	$(patsubst %.cc,$(1)/%.o,$(filter %.cc,$(2))) \
	$(patsubst %.cpp,$(1)/%.o,$(filter %.cpp,$(2))) \
	$(patsubst %.c,$(1)/%.o, $(filter %.c,$(2))) \
	$(patsubst %.s,$(1)/%.o, $(filter %.s,$(2))))
getprj = $(word 1,$(subst /, ,$(1)))
getlib = $(word 2,$(subst /, ,$(1)))
getproject = $(RELEASE_DIR)/build/$(1)/lib/$(tgt_arch)
getlibrary = $(call getproject,$(call getprj,$(1)))/lib$(call getlib,$(1)).$(LIBEXTNS)
getlibraries = $(foreach prjlib,$(1),$(call getlibrary,$(prjlib)))
getprojects  = $(foreach prjlib,$(1),$(call getprj,$(prjlib)))
getlinkdirs  = $(addprefix -L, $(sort $(foreach prj,$(call getprojects,$(1)),$(call getproject,$(prj)))))
getlinksdir  = $(addprefix -L, $(sort $(dir $(1))))
getlinklibs  = $(addprefix -l,$(foreach prjlib,$(1),$(call getlib,$(prjlib))))
getlinkslib  = $(addprefix -l,$(notdir $(1)))
getrpath  = $$ORIGIN/../../../$(1)/lib/$(tgt_arch)
getrpaths = $(subst $(space),:,$(strip $(foreach prj,$(call getprojects,$(1)),$(call getrpath,$(prj)))))


define object_template
  incdirs_$(1) := $$(addprefix -I$(RELEASE_DIR)/,$(2))
  incdirs_$(1) += -I$(RELEASE_DIR)
  incdirs_$(1) += $$(addprefix -I$(RELEASE_DIR)/build/,$(2))
  incdirs_$(1) += -I$(RELEASE_DIR)/build
  incdirs_$(1) += $$(addprefix -I,$(3))
endef

define library_template
  library_$(1) := $$(libdir)/lib$(1).$(LIBEXTNS)
  libobjs_$(1) := $$(call getobjects,$$(objdir),$$(libsrcs_$(1)))
  libraries    += $$(library_$(1))
  objects      += $$(libobjs_$(1))
  depends      += $$(libobjs_$(1):$$(objdir)/%.o=$$(depdir)/%.d)
  libraries_$(1) := $$(call getlibraries,$$(liblibs_$(1)))
  linkdirs_$(1)  := $$(call getlinkdirs,$$(liblibs_$(1)))
  linkdirs_$(1)  += $$(call getlinksdir,$$(libslib_$(1)))
ifneq ($$(liblibs_$(1)),)
  linklibs_$(1)  := $$(call reverse,$$(call getlinklibs,$$(liblibs_$(1))))
endif
ifneq ($$(libslib_$(1)),)
  linklibs_$(1)  += $$(call reverse,$$(call getlinkslib,$$(libslib_$(1))))
endif
ifeq ($$(LIBEXTNS),so)
ifneq ($$(ifversn_$(1)),)
  ifversnflags_$(1) := -Wl,--version-script=$$(ifversn_$(1))
endif
endif
  linkflags_$(1) := $$(linkdirs_$(1)) $$(linklibs_$(1))
$$(library_$(1)): $$(libobjs_$(1))
endef

$(foreach lib,$(libnames),$(eval $(call library_template,$(lib))))
$(foreach lib,$(libnames),$(foreach obj,$(libsrcs_$(lib)),$(eval $(call object_template,$(obj),$(libincs_$(lib)),$(libsinc_$(lib))))))

define target_template
  target_$(1)  := $$(bindir)/$(1)
  tgtobjs_$(1) := $$(call getobjects,$$(objdir),$$(tgtsrcs_$(1)))
  targets      += $$(target_$(1))
  objects      += $$(tgtobjs_$(1))
  depends      += $$(tgtobjs_$(1):$$(objdir)/%.o=$$(depdir)/%.d)
  libraries_$(1) := $$(call getlibraries,$$(tgtlibs_$(1)))
  linkdirs_$(1)  := $$(call getlinkdirs,$$(tgtlibs_$(1)))
  linkdirs_$(1)  += $$(call getlinksdir,$$(tgtslib_$(1)))
ifneq ($$(tgtlibs_$(1)),)
  linklibs_$(1)  := $$(call reverse,$$(call getlinklibs,$$(tgtlibs_$(1))))
endif
ifneq ($$(tgtslib_$(1)),)
  linklibs_$(1)  += $$(call reverse,$$(call getlinkslib,$$(tgtslib_$(1))))
endif
ifeq ($$(LIBEXTNS),so)
  rpaths_$(1)    := -Wl,-rpath='$$(call getrpaths,$$(tgtlibs_$(1)))'
endif
  linkflags_$(1) := $$(linkdirs_$(1)) $$(linklibs_$(1)) $$(rpaths_$(1))
ifneq ($$(MANAGERS),)
  nomanagrs_$(1) := $$(filter-out $$(managrs_$(1)),$$(MANAGERS))
  nomanagrs_$(1) := $$(nomanagrs_$(1):%=$$(RTEMSDIR)/no-%.rel)
  tgtobjs_$(1)   += $$(nomanagrs_$(1))
endif
$$(target_$(1)): $$(tgtobjs_$(1)) $$(libraries_$(1))
endef

$(foreach tgt,$(tgtnames),$(eval $(call target_template,$(tgt))))
$(foreach tgt,$(tgtnames),$(foreach obj,$(tgtsrcs_$(tgt)),$(eval $(call object_template,$(obj),$(tgtincs_$(tgt)),$(tgtsinc_$(tgt))))))


# Rules
# -----
rules := all dir obj lib bin clean cleanall userall userclean print

.PHONY: $(rules) $(libnames) $(tgtnames)

.SUFFIXES:  # Kills all implicit rules

all: bin userall;

obj: $(objects);

lib: $(libraries);

bin: lib $(targets);

dir: $(prod_dirs) $(temp_dirs);

print:
	@echo	"bindir    = $(bindir)"
	@echo	"libdir    = $(libdir)"
	@echo	"objdir    = $(objdir)"
	@echo	"depdir    = $(depdir)"
	@echo   "targets   = $(targets)"
	@echo	"libraries = $(libraries)"
	@echo	"depends   = $(depends)"
	@echo	"objects   = $(objects)"
	@echo	"managers  = $(MANAGERS)"

clean: userclean
ifneq ($(objects),)
	@echo "[RO] Removing object files"
	$(quiet)$(RM) $(objects)
endif
ifneq ($(depends),)
	@echo "[RD] Removing depend files"
	$(quiet)$(RM) $(depends)
endif
ifneq ($(libraries),)
	@echo "[RL] Removing libraries: $(notdir $(libraries))"
	$(quiet)$(RM) $(libraries)
endif
ifneq ($(targets),)
	@echo "[RT] Removing targets: $(notdir $(targets))"
	$(quiet)$(RM) $(targets)
endif

cleanall: userclean
	$(quiet)$(RM) -r $(temp_dirs)


# Directory structure
$(prod_dirs) $(temp_dirs):
	$(quiet)mkdir -p $@


# Libraries
$(libdir)/lib%.$(LIBEXTNS):
	@echo "[LD] Build library $*"
	$(quiet)$(LD) $(LDFLAGS) $(ifversnflags_$*) $(linkflags_$*) $^ -o $@


# Exceutables
$(bindir)/%:
	@echo "[LT] Linking target $*"
	$(quiet)$(LX) $(DEFINES) $(tgtobjs_$*) $(linkflags_$*) $(LXFLAGS) -o $@


# Objects for C, C++ and assembly files
$(objdir)/%.o: %.c
	@echo "[CC] Compiling $<"
	$(quiet)$(CC) $(incdirs_$<) $(CPPFLAGS) $(DEFINES) $(CFLAGS) -c $< -o $@

$(objdir)/%.o: %.cc
	@echo "[CX] Compiling $<"
	$(quiet)$(CXX) $(incdirs_$<) $(CPPFLAGS) $(DEFINES) $(CXXFLAGS) -c $< -o $@

$(objdir)/%.o: %.cpp
	@echo "[CX] Compiling $<"
	$(quiet)$(CXX) $(incdirs_$<) $(CPPFLAGS) $(DEFINES) $(CXXFLAGS) -c $< -o $@

$(objdir)/%.o: %.s
	@echo "[CS] Compiling $<"
	$(quiet)$(CXX) $(incdirs_$<) $(CPPFLAGS) $(DEFINES) $(CASFLAGS) -c $< -o $@


# Defines rules to (re)build dependency files
DEPSED = sed '\''s!$(notdir $*)\.o!$(objdir)/$*\.o $@!g'\'' 
CXXDEP = $(CXX) $(incdirs_$<) $(CPPFLAGS) $(DEPFLAGS)
CCDEP  = $(CC)  $(incdirs_$<) $(CPPFLAGS) $(DEPFLAGS)

$(depdir)/%.d: %.c
	@echo "[DC] Dependencies for $<"
	$(quiet)$(SHELL) -ec '$(CCDEP) $< | $(DEPSED)  > $@'

$(depdir)/%.d: %.cc
	@echo "[DX] Dependencies for $<"
	$(quiet)$(SHELL) -ec '$(CXXDEP) $< | $(DEPSED) > $@'

$(depdir)/%.d: %.cpp
	@echo "[DX] Dependencies for $<"
	$(quiet)$(SHELL) -ec '$(CXXDEP) $< | $(DEPSED) > $@'

$(depdir)/%.d: %.s
	@echo "[DS] Dependencies for $<" 
	$(quiet)$(SHELL) -ec '$(CCDEP) $< | $(DEPSED) > $@'

# Include the dependency files.  If one of the .d files in depends
# does not exist, then make invokes one of the rules [Dn] above to
# rebuild the missing .d file.  This can be short-circuited by
# defining the symbol 'no_depends'.

ifneq ($(depends),)
ifeq  ($(no_depends),)
-include $(depends)
endif
endif
