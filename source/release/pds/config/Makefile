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
include $(RELEASE_DIR)/make/sw/python.mk
endif
