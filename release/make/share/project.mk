# Project level makefile
# ----------------------
Makefile:;

# Symbols
# -------
cwd := $(subst /, ,$(shell pwd))
prj_name := $(word $(words $(cwd)),$(cwd))

archs := $(arch_tgts)
archs.% := $(addsuffix .%,$(arch_tgts))
packages.% := $(addsuffix .%,$(packages))

# Rules
# -----
.PHONY: $(archs) $(archs.%) $(packages) $(packages.%)

define package_template
$(1).%:
	$(quiet)echo "[PK] Target <$$*> package <$(1)>"
	$(quiet)$$(MAKE) -C $(1) $$*
endef

$(foreach pkg,$(packages),$(eval $(call package_template,$(pkg))))

define arch_template
packages_$(1) := $$(foreach pkg,$$(packages),$$(pkg).$(1))
$(1): $$(packages_$(1));
$(1).%: $$(addsuffix .%,$$(packages_$(1)));
endef

$(foreach arc,$(archs),$(eval $(call arch_template,$(arc))))

cleanall:
	@echo "[RO] Removing project $(prj_name) build directory"
	$(quiet)$(RM) -r $(RELEASE_DIR)/build/$(prj_name)

%:;
	@echo "---- No target <$*> for project <$(prj_name)>"
