# Package level makefile
# ----------------------
%.mk:;

# Checks
# ------
# Check release location variables
ifeq ($(RELEASE_DIR),)
export RELEASE_DIR := $(PWD)/../..
endif

include $(RELEASE_DIR)/make/share/setup.mk
include ../flags.mk

ifndef PREMAKE_DONE
include $(RELEASE_DIR)/make/share/premake.mk
else
include constituents.mk
include $(RELEASE_DIR)/make/sw/package.mk

#
#  Special rules for creating python C++ extension libraries
#
userall: $(libdir)/pyami.so

userclean:
	$(quiet)$(RM) $(libdir)/pyami.so

# Libraries
$(libdir)/py%.$(LIBEXTNS):
	@echo "[LD] Build python library $*"
	$(quiet)ln -s libpy$*.$(LIBEXTNS) $@

endif
