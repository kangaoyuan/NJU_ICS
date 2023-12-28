HAS_NAVY = 1
RAMDISK_FILE = build/ramdisk.img

NAME = nanos-lite
SRCS = $(shell find -L ./src/ -name "*.c" -o -name "*.cpp" -o -name "*.S")
include $(AM_HOME)/Makefile

ifeq ($(ARCH),native)
ISA = am_native
endif

# nanos-lite ramdisk_file dependency to support fs 
./src/resources.S: $(RAMDISK_FILE)
	@touch $@

ifeq ($(HAS_NAVY),)
    files = $(RAMDISK_FILE) src/files.h src/syscall.h
    # create an empty file if it does not exist
    # $(foreach f, $(files), loop-body)
    # $(if cond, true-part, false-part)
    $(foreach f,$(files), $(if $(wildcard $f),, $(shell touch $f)))
else
    ifeq ($(wildcard $(NAVY_HOME)/libs/libos/src/syscall.h),)
        $(error $$NAVY_HOME must be a Navy-apps repo)
endif

# fs file recording table
update:
	$(MAKE) -s -C $(NAVY_HOME) ISA=$(ISA) ramdisk
	# ln -sf may overwrite an existing symbolic link pointing to a new different file
	@ln -sf $(NAVY_HOME)/build/ramdisk.img $(RAMDISK_FILE)
	@ln -sf $(NAVY_HOME)/build/ramdisk.h src/files.h
	@ln -sf $(NAVY_HOME)/libs/libos/src/syscall.h src/syscall.h

.PHONY: update
endif
