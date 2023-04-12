ifndef QCONFIG
QCONFIG=qconfig.mk
endif
include $(QCONFIG)

NAME=tinyxml2

DIST_BASE=$(PRODUCT_ROOT)

#$(INSTALL_ROOT_$(OS)) is pointing to $QNX_TARGET
#by default, unless it was manually re-routed to
#a staging area by setting both INSTALL_ROOT_nto
#and USE_INSTALL_ROOT
tinyxml2_INSTALL_ROOT ?= $(INSTALL_ROOT_$(OS))

tinyxml2_VERSION = 9.0.0

#choose Release or Debug
CMAKE_BUILD_TYPE ?= Release

#set the following to FALSE if generating .pinfo files is causing problems
GENERATE_PINFO_FILES ?= TRUE

#override 'all' target to bypass the default QNX build system
ALL_DEPENDENCIES = tinyxml2_all
.PHONY: tinyxml2_all install check clean

CFLAGS += $(FLAGS)
LDFLAGS += -Wl,--build-id=md5

define PINFO
endef
PINFO_STATE=Experimental
USEFILE=

include $(MKFILES_ROOT)/qtargets.mk

CMAKE_ARGS = -DCMAKE_TOOLCHAIN_FILE=$(PROJECT_ROOT)/qnx.nto.toolchain.cmake \
             -DCMAKE_INSTALL_PREFIX=$(tinyxml2_INSTALL_ROOT)/$(CPUVARDIR)/usr \
             -DCMAKE_BUILD_TYPE=$(CMAKE_BUILD_TYPE) \
             -DCMAKE_SYSTEM_PROCESSOR=$(CPUVARDIR) \
             -DCMAKE_INSTALL_LIBDIR=$(QNX_TARGET)/$(CPUVARDIR)/usr/lib \
             -DCMAKE_INSTALL_INCLUDEDIR=$(QNX_TARGET)/usr/include/tinyxml2 \
             -DEXTRA_CMAKE_C_FLAGS="$(CFLAGS)" \
             -DEXTRA_CMAKE_CXX_FLAGS="$(CFLAGS)" \
             -DEXTRA_CMAKE_ASM_FLAGS="$(FLAGS)" \
             -DEXTRA_CMAKE_LINKER_FLAGS="$(LDFLAGS)" \
             -DBUILD_SHARED_LIBS=1 \
             -DBUILD_TESTING=OFF

MAKE_ARGS ?= -j $(firstword $(JLEVEL) 1)

ifndef NO_TARGET_OVERRIDE
tinyxml2_all:
	@mkdir -p build
	@cd build && cmake $(CMAKE_ARGS) $(DIST_BASE)
	@cd build && make VERBOSE=1 all $(MAKE_ARGS)

install check: tinyxml2_all
	@echo Installing...
	@cd build && make VERBOSE=1 install $(MAKE_ARGS)
	@echo Done.

install_test: install
	@mkdir -p build_tests
	@cd build_tests && cmake $(CMAKE_ARGS) $(DIST_BASE)/test
	@cd build_tests && make VERBOSE=1 install $(MAKE_ARGS)

clean iclean spotless:
	rm -rf build
	rm -rf build_tests

uninstall:
endif

#everything down below deals with the generation of the PINFO
#information for shared objects that is used by the QNX build
#infrastructure to embed metadata in the .so files, for example
#data and time, version number, description, etc. Metadata can
#be retrieved on the target by typing 'use -i <path to openblas .so file>'.
#this is optional: setting GENERATE_PINFO_FILES to FALSE will disable
#the insertion of metadata in .so files.
ifeq ($(GENERATE_PINFO_FILES), TRUE)
#the following rules are called by the cmake generated makefiles,
#in order to generate the .pinfo files for the shared libraries
%.so$(tinyxml2_VERSION):
	$(ADD_PINFO)
	$(ADD_USAGE)

endif
