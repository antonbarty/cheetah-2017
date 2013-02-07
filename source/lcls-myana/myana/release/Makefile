# Top level makefile
# ------------------
%.mk:;

# Checks
# ------
# Check release location variables
ifeq ($(RELEASE_DIR),)
export RELEASE_DIR := $(PWD)
endif

# Includes
# --------
include make/share/setup.mk
include projects.mk
include make/share/release.mk
