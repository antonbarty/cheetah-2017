# Prepare to run package level makefile
# -------------------------------------
Makefile:;

# This makefile is conditionally included at the start of each package
# specific Makefile. Its purpose is to correctly call itself after
# creating the appropriate directories, set tgt_arch and the target
# specific flags.


# Symbols
# -------
# To be sure there is one, and only one, `-f Makefile' option
MAKE := $(filter-out -f Makefile, $(MAKE)) -f Makefile

archs := $(arch_tgts)
archs.% := $(addsuffix .%,$(arch_tgts))

# Rule specific flags
clean-flags     := no_depends=y
cleanall-flags  := no_depends=y
userclean-flags := no_depends=y
print-flags     := no_depends=y


# Rules
# -----
.PHONY: $(archs) $(archs.%)

define arch_template
$(1):
	$$(quiet)$$(MAKE) PREMAKE_DONE=y tgt_arch=$(1) dir no_depends=y
	$$(quiet)$$(MAKE) PREMAKE_DONE=y tgt_arch=$(1) all

$(1).%:
	$$(quiet)$$(MAKE) PREMAKE_DONE=y tgt_arch=$(1) dir no_depends=y
	$$(quiet)$$(MAKE) PREMAKE_DONE=y tgt_arch=$(1) $$* $$($$*-flags)

endef

$(foreach arc,$(archs),$(eval $(call arch_template,$(arc))))
