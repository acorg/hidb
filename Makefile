# -*- Makefile -*-
# Eugene Skepner 2017
# ----------------------------------------------------------------------

MAKEFLAGS = -w

# ----------------------------------------------------------------------

HIDB_SOURCES = hidb.cc hidb-export.cc hidb-import.cc variant-id.cc vaccines.cc
HIDB_PY_SOURCES = py.cc $(HIDB_SOURCES)

# ----------------------------------------------------------------------

include $(ACMACSD_ROOT)/share/makefiles/Makefile.g++
include $(ACMACSD_ROOT)/share/makefiles/Makefile.dist-build.vars

PYTHON_VERSION = $(shell python3 -c 'import sys; print("{0.major}.{0.minor}".format(sys.version_info))')
PYTHON_CONFIG = python$(PYTHON_VERSION)-config
PYTHON_MODULE_SUFFIX = $(shell $(PYTHON_CONFIG) --extension-suffix)

LOCATION_DB_LIB = $(AD_LIB)/liblocationdb.so
HIDB_LIB = $(DIST)/libhidb.so

# -fvisibility=hidden and -flto make resulting lib smaller (pybind11) but linking is much slower
OPTIMIZATION = -O3 #-fvisibility=hidden -flto
PROFILE = # -pg
CXXFLAGS = -MMD -g $(OPTIMIZATION) $(PROFILE) -fPIC -std=$(STD) $(WEVERYTHING) $(WARNINGS) -Icc -I$(AD_INCLUDE) $(PKG_INCLUDES)
LDFLAGS = $(OPTIMIZATION) $(PROFILE)
HIDB_LDLIBS = -L$(AD_LIB) -llocationdb -lacmacsbase -lacmacschart -lboost_filesystem -lboost_system $(shell pkg-config --libs liblzma) $(shell $(PYTHON_CONFIG) --ldflags | sed -E 's/-Wl,-stack_size,[0-9]+//')

PKG_INCLUDES = $(shell pkg-config --cflags liblzma) $(shell $(PYTHON_CONFIG) --includes)

# ----------------------------------------------------------------------

all: check-acmacsd-root $(DIST)/hidb_backend$(PYTHON_MODULE_SUFFIX) $(HIDB_LIB)

install: check-acmacsd-root install-headers $(DIST)/hidb_backend$(PYTHON_MODULE_SUFFIX) $(HIDB_LIB)
	ln -sf $(HIDB_LIB) $(AD_LIB)
	if [ $$(uname) = "Darwin" ]; then /usr/bin/install_name_tool -id $(AD_LIB)/$(notdir $(HIDB_LIB)) $(AD_LIB)/$(notdir $(HIDB_LIB)); fi
	ln -sf $(DIST)/hidb_backend$(PYTHON_MODULE_SUFFIX) $(AD_PY)
	ln -sf $(abspath py)/* $(AD_PY)
	ln -sf $(abspath bin)/hidb-* $(AD_BIN)
	-$(abspath bin)/hidb-get-from-albertine

install-headers:
	if [ ! -d $(AD_INCLUDE)/hidb ]; then mkdir $(AD_INCLUDE)/hidb; fi
	ln -sf $(abspath cc)/*.hh $(AD_INCLUDE)/hidb

test: install
	test/test

# ----------------------------------------------------------------------

-include $(BUILD)/*.d
include $(ACMACSD_ROOT)/share/makefiles/Makefile.dist-build.rules
include $(ACMACSD_ROOT)/share/makefiles/Makefile.rtags

# ----------------------------------------------------------------------

$(DIST)/hidb_backend$(PYTHON_MODULE_SUFFIX): $(patsubst %.cc,$(BUILD)/%.o,$(HIDB_PY_SOURCES)) | $(DIST) $(LOCATION_DB_LIB)
	@echo "SHARED     " $@ # '<--' $^
	@$(CXX) -shared $(LDFLAGS) -o $@ $^ $(HIDB_LDLIBS)
	@#strip $@

$(HIDB_LIB): $(patsubst %.cc,$(BUILD)/%.o,$(HIDB_SOURCES)) | $(DIST) $(LOCATION_DB_LIB)
	@echo "SHARED     " $@ # '<--' $^
	@$(CXX) -shared $(LDFLAGS) -o $@ $^ $(HIDB_LDLIBS)

# $(DIST)/test-rapidjson: $(BUILD)/test-rapidjson.o $(BUILD)/chart.o $(BUILD)/chart-rj.o $(BUILD)/ace.o $(BUILD)/read-file.o $(BUILD)/xz.o | $(DIST)
#	$(CXX) $(LDFLAGS) -o $@ $^ $(shell pkg-config --libs liblzma)

# ======================================================================
### Local Variables:
### eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
### End:
