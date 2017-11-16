# -*- Makefile -*-
# Eugene Skepner 2017
# ----------------------------------------------------------------------

MAKEFLAGS = -w

# ----------------------------------------------------------------------

HIDB_SOURCES = hidb.cc hidb-export.cc hidb-import.cc variant-id.cc vaccines.cc
HIDB_PY_SOURCES = py.cc $(HIDB_SOURCES)
HIDB_FIND_NAME_SOURCES = hidb-find-name.cc

# ----------------------------------------------------------------------

include $(ACMACSD_ROOT)/share/makefiles/Makefile.g++
include $(ACMACSD_ROOT)/share/makefiles/Makefile.dist-build.vars

PYTHON_VERSION = $(shell python3 -c 'import sys; print("{0.major}.{0.minor}".format(sys.version_info))')
PYTHON_CONFIG = python$(PYTHON_VERSION)-config
PYTHON_MODULE_SUFFIX = $(shell $(PYTHON_CONFIG) --extension-suffix)

HIDB_LIB = $(DIST)/libhidb.so

CXXFLAGS = -MMD -g $(OPTIMIZATION) $(PROFILE) -fPIC -std=$(STD) $(WARNINGS) -Icc -I$(AD_INCLUDE) $(PKG_INCLUDES)
LDFLAGS = $(OPTIMIZATION) $(PROFILE)
HIDB_LDLIBS = \
	$(AD_LIB)/$(call shared_lib_name,libacmacsbase,1,0) \
	$(AD_LIB)/$(call shared_lib_name,liblocationdb,1,0) \
	$(AD_LIB)/$(call shared_lib_name,libacmacschart,1,0) \
	$(shell pkg-config --libs liblzma) $(shell $(PYTHON_CONFIG) --ldflags | sed -E 's/-Wl,-stack_size,[0-9]+//') $(CXX_LIB)

PKG_INCLUDES = $(shell pkg-config --cflags liblzma) $(shell $(PYTHON_CONFIG) --includes)

# ----------------------------------------------------------------------

BINS_TO_MAKE = $(DIST)/hidb_backend$(PYTHON_MODULE_SUFFIX) \
	       $(HIDB_LIB) \
	       $(DIST)/hidb-find-name

all: check-acmacsd-root $(BINS_TO_MAKE)

install: check-acmacsd-root install-headers $(AD_LIB)/libhidb.so $(BINS_TO_MAKE)
	ln -sf $(DIST)/hidb_backend$(PYTHON_MODULE_SUFFIX) $(AD_PY)
	ln -sf $(abspath py)/* $(AD_PY)
	ln -sf $(abspath bin)/hidb-* $(AD_BIN)
	ln -sf $(abspath dist)/hidb-* $(AD_BIN)
	@#-$(abspath bin)/hidb-get-from-albertine

$(AD_LIB)/libhidb.so: $(HIDB_LIB)
	$(call install_lib,$(HIDB_LIB))

test: install
	test/test

# ----------------------------------------------------------------------

-include $(BUILD)/*.d
include $(ACMACSD_ROOT)/share/makefiles/Makefile.dist-build.rules
include $(ACMACSD_ROOT)/share/makefiles/Makefile.rtags

# ----------------------------------------------------------------------

$(DIST)/hidb_backend$(PYTHON_MODULE_SUFFIX): $(patsubst %.cc,$(BUILD)/%.o,$(HIDB_PY_SOURCES)) | $(DIST)
	@printf "%-16s %s\n" "SHARED" $@
	@$(CXX) -shared $(LDFLAGS) -o $@ $^ $(HIDB_LDLIBS)

$(HIDB_LIB): $(patsubst %.cc,$(BUILD)/%.o,$(HIDB_SOURCES)) | $(DIST)
	@printf "%-16s %s\n" "SHARED" $@
	@$(CXX) -shared $(LDFLAGS) -o $@ $^ $(HIDB_LDLIBS)

$(DIST)/hidb-find-name: $(patsubst %.cc,$(BUILD)/%.o,$(HIDB_FIND_NAME_SOURCES)) | $(DIST) $(AD_LIB)/libhidb.so
	@printf "%-16s %s\n" "LINK" $@
	@$(CXX) $(LDFLAGS) -o $@ $^ -L$(AD_LIB) -lhidb $(HIDB_LDLIBS)

# $(DIST)/test-rapidjson: $(BUILD)/test-rapidjson.o $(BUILD)/chart.o $(BUILD)/chart-rj.o $(BUILD)/ace.o $(BUILD)/read-file.o $(BUILD)/xz.o | $(DIST)
#	$(CXX) $(LDFLAGS) -o $@ $^ $(shell pkg-config --libs liblzma)

# ======================================================================
### Local Variables:
### eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
### End:
