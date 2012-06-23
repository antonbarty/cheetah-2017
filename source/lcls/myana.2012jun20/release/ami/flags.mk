include $(RELEASE_DIR)/make/sw/flags.mk

ifneq ($(findstring x86_64-linux,$(tgt_arch)),)
else
  DEFINES += -fopenmp
endif