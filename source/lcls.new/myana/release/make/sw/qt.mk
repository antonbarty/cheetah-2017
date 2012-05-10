#
#  Qt meta object compilation is signaled by including <source>_moc.cc in the {lib,tgt}srcs directive
#

# Call Qt meta object compiler (moc)
MOC := $(RELEASE_DIR)/build/qt/bin/moc

$(objdir)/%_moc.o: $(objdir)/%_moc.cc
	@echo "[CX] Compiling $<"
	$(CXX) $(incdirs_$(<F)) $(DEFINES) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

$(objdir)/%_moc.cc: %.hh
	@echo "[MOC] Qt moc $<"
	$(MOC) $< -o $@
