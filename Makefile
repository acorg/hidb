# -*- Makefile -*-
# Eugene Skepner 2017
# ----------------------------------------------------------------------

MAKEFLAGS = -w

# ----------------------------------------------------------------------

TARGETS = \
	$(HIDB_LIB) \
	$(HIDB_PY_LIB) \
	$(DIST)/hidb-find-name

HIDB_SOURCES = hidb.cc hidb-export.cc hidb-import.cc variant-id.cc vaccines.cc
HIDB_PY_SOURCES = py.cc $(HIDB_SOURCES)
HIDB_FIND_NAME_SOURCES = hidb-find-name.cc

HIDB_LIB_MAJOR = 1
HIDB_LIB_MINOR = 0
HIDB_LIB_NAME = libhidb
HIDB_LIB = $(DIST)/$(call shared_lib_name,$(HIDB_LIB_NAME),$(HIDB_LIB_MAJOR),$(HIDB_LIB_MINOR))

HIDB_PY_LIB_MAJOR = 1
HIDB_PY_LIB_MINOR = 0
HIDB_PY_LIB_NAME = hidb_backend
HIDB_PY_LIB = $(DIST)/$(HIDB_PY_LIB_NAME)$(PYTHON_MODULE_SUFFIX)

# ----------------------------------------------------------------------

include $(ACMACSD_ROOT)/share/makefiles/Makefile.g++
include $(ACMACSD_ROOT)/share/makefiles/Makefile.python
include $(ACMACSD_ROOT)/share/makefiles/Makefile.dist-build.vars

CXXFLAGS = -MMD -g $(OPTIMIZATION) $(PROFILE) -fPIC -std=$(STD) $(WARNINGS) -Icc -I$(AD_INCLUDE) $(PKG_INCLUDES)
LDFLAGS = $(OPTIMIZATION) $(PROFILE)
LDLIBS = \
	$(AD_LIB)/$(call shared_lib_name,libacmacsbase,1,0) \
	$(AD_LIB)/$(call shared_lib_name,liblocationdb,1,0) \
	$(AD_LIB)/$(call shared_lib_name,libacmacschart,1,0) \
	$(shell pkg-config --libs liblzma) $(CXX_LIB)

PKG_INCLUDES = $(shell pkg-config --cflags liblzma) $(PYTHON_INCLUDES)

# ----------------------------------------------------------------------

all: check-acmacsd-root $(TARGETS)

install: check-acmacsd-root install-headers $(TARGETS)
	$(call install_lib,$(HIDB_LIB))
	$(call install_py_lib,$(HIDB_PY_LIB))
	ln -sf $(abspath py)/* $(AD_PY)
	ln -sf $(abspath bin)/hidb-* $(AD_BIN)
	ln -sf $(abspath dist)/hidb-* $(AD_BIN)
	@#-$(abspath bin)/hidb-get-from-albertine

test: install
	test/test

# ----------------------------------------------------------------------

-include $(BUILD)/*.d
include $(ACMACSD_ROOT)/share/makefiles/Makefile.dist-build.rules
include $(ACMACSD_ROOT)/share/makefiles/Makefile.rtags

# ----------------------------------------------------------------------

$(HIDB_LIB): $(patsubst %.cc,$(BUILD)/%.o,$(HIDB_SOURCES)) | $(DIST)
	@printf "%-16s %s\n" "SHARED" $@
	@$(call make_shared,$(HIDB_LIB_NAME),$(HIDB_LIB_MAJOR),$(HIDB_LIB_MINOR)) $(LDFLAGS) -o $@ $^ $(LDLIBS)

$(HIDB_PY_LIB): $(patsubst %.cc,$(BUILD)/%.o,$(HIDB_PY_SOURCES)) | $(DIST)
	@printf "%-16s %s\n" "SHARED" $@
	@$(call make_shared,$(HIDB_PY_LIB_NAME),$(HIDB_PY_LIB_MAJOR),$(HIDB_PY_LIB_MINOR)) $(LDFLAGS) -o $@ $^ $(LDLIBS) $(PYTHON_LDLIBS)

$(DIST)/%: $(BUILD)/%.o | $(HIDB_LIB)
	@printf "%-16s %s\n" "LINK" $@
	@$(CXX) $(LDFLAGS) -o $@ $^ $(HIDB_LIB) $(LDLIBS)

# ======================================================================
### Local Variables:
### eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
### End:
