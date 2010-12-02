# Top level makefile
# ------------------
Makefile:;

# Symbols
# -------
SHELL := /bin/bash

# Minimal directory tree
tree_dirs = build

# Rules
# -----
.PHONY: dir cleanall $(projects) $(projects.%)

dir: $(tree_dirs);
$(tree_dirs):
	@echo "[DR] Target <dir> at top level";
	$(quiet)mkdir -p $@

cleanall:
	@echo "[CL] Target <cleanall> at top level";
	$(quiet)$(RM) -r build

define soft-link
  if [ ! -e $(1) ]; then \
    if [ -e $(2) ]; then \
      echo '[SL] Make soft link $(1)'; \
      ln -s $(2) $(1);     \
    else \
      echo '[SL] *** project $(2) not found'; \
    fi \
  fi
endef

define delete-soft-links
  for prj in $(projects); do \
    if [ -h build/$$prj ]; then    \
       $(RM) build/$$prj;          \
    fi;                      \
  done
endef

define project_template
ifeq ($$(strip $$($(1)_use)),release)
$(1).%: dir
	$(quiet)echo "[PR] Target <$$*> project <$(1)>"
	$$(MAKE) -C $(1) $$*
else
ifneq ($$(findstring /,$$($(1)_use),),)
$(1).%: dir
	@$$(call soft-link,build/$(1),$$($(1)_use))
else
$$(error 'Project $(1) lacks a use statement')
endif
endif
endef

# revisit release_base
# $(1).%: dir
# 	@$$(call soft-link,build/$(1),$$(release_base)/$(1))

$(foreach prj,$(projects),$(eval $(call project_template,$(prj))))

define all-projects
  for prj in $(projects); do \
    $(MAKE) $$prj.$*;        \
  done
endef

%:; @$(all-projects)
