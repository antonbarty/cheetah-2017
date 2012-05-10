# Checks
# ------
SHELL := /bin/bash
REQUIRED_MAKE_VERSION := 3.80

required_make_major_version := $(word 1,$(subst ., ,$(REQUIRED_MAKE_VERSION)))
required_make_minor_version := $(word 2,$(subst ., ,$(REQUIRED_MAKE_VERSION)))
make_major_version := $(word 1,$(subst ., ,$(MAKE_VERSION)))
make_minor_version := $(word 2,$(subst ., ,$(MAKE_VERSION)))

test_make_version = $(shell \
  if [[ $(make_major_version) <  $(required_make_major_version) || \
        $(make_major_version) == $(required_make_major_version) && \
        $(make_minor_version) <  $(required_make_minor_version) ]] ; then \
     echo 'error'; \
  else \
     echo 'ok'; \
  fi)

ifneq ($(call test_make_version), ok)
$(error 'Makefile version is $(MAKE_VERSION) but needs to be $(REQUIRED_MAKE_VERSION) or higher')
endif


# Symbols
# -------
# To be sure there is one, and only one, `-f Makefile' option
MAKE := $(filter-out -f Makefile, $(MAKE)) -f Makefile

# Define reverse function
reverse = $(shell                       \
  set reversed="";                      \
  for entry in $(1); do                 \
    reversed="$$entry $$reversed";      \
  done;                                 \
  echo $$reversed)

# Set verbosity
ifeq ($(verbose),y)
  quiet :=
  MAKEFLAGS :=
else
  quiet := @
  MAKEFLAGS := -s
endif

# Target variable
# ---------------
tgt_arch := $(MAKECMDGOALS)
