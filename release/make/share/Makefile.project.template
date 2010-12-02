# Project level makefile
# ----------------------
%.mk:;

# Checks
# ------
# Check release location variables
ifeq ($(RELEASE_DIR),)
export RELEASE_DIR := $(PWD)/..
endif

# Includes
# --------
include $(RELEASE_DIR)/make/share/setup.mk
include flags.mk
include packages.mk
include $(RELEASE_DIR)/make/share/project.mk
