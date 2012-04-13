# Call sip interpreter for python wrappers
$(objdir)/%_sip_wrap.o: $(objdir)/%_sip_wrap.cpp
	@echo "[CX] Compiling $<"
	$(quiet)$(CXX) $(incdirs_$*_sip_wrap.cpp) $(DEFINES) $(CPPFLAGS) $(CFLAGS) -c $< -o $@


$(objdir)/%_sip_wrap.cpp: %_sip_wrap.sip
	@echo "[WG] Python sip $<"
	$(quiet)sip $(SIPFLAGS) -e -j 1 -c $(objdir)/ $<
	cp $(objdir)/sip$*part0.cpp .
	$(quiet)mv $(objdir)/sip$*part0.cpp $@

# $(incdirs_$*_sip_wrap.cpp)


# Call swig interpreter for python wrappers
$(objdir)/%_swig_wrap.o: $(objdir)/%_swig_wrap.c
	@echo "[CC] Compiling $<"
	$(quiet)$(CC) $(incdirs_$*_swig_wrap.c) $(DEFINES) $(CFLAGS) -c $< -o $@

$(objdir)/%_swig_wrap.c: %_swig_wrap.i
	@echo "[WG] Python swig $<"
	$(quiet)swig $(incdirs_$*_swig_wrap.c) -Wall -python -o $@ $<

SWGDEPSED = sed '\''s!$(notdir $*)\_swig_wrap_wrap.c!$(objdir)/$*\_swig_wrap.c $@!g'\'' 
SWGDEP = swig $(DEPFLAGS) $(incdirs_$*_swig_wrap.c)

$(depdir)/%_swig_wrap.d: %_swig_wrap.i
	@echo "[DI] Dependencies for $<" 
	$(quiet)$(SHELL) -ec '$(SWGDEP) $< | $(SWGDEPSED) > $@'

# Include the dependency files.  If one of the .d files in depends
# does not exist, then make invokes one of the rules [Dn] above to
# rebuild the missing .d file.  This can be short-circuited by
# defining the symbol 'no_depends'.

ifneq ($(depends),)
ifeq  ($(no_depends),)
-include $(depends)
endif
endif
