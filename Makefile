ifeq ($(OS),Windows_NT)
    detected_OS := Windows
else
    detected_OS := $(shell sh -c 'uname -s 2>/dev/null || echo not')
endif

# Set default C compiler.
# Clean implict CC variable.
CC=

ifndef CC
	ifeq ($(detected_OS),Windows)
		CC=cl
	else ifeq ($(detected_OS),Darwin)
		CC=clang
	else
		CC=gcc
	endif
endif  # CC

export CC

ifeq ($(detected_OS),Windows)
	SEP=\\
else
	SEP=/
endif

SRC_DIR=src
EXAMPLE_DIR=example

prefix=
ifndef prefix
	prefix=/usr/local
endif

exec_prefix=$(prefix)
bindir=$(exec_prefix)/bin
includedir=$(prefix)/include
libdir=$(exec_prefix)/lib


.PHONY: all example example-cpp dynamic dynamic-cpp static static-cpp \
	exec install install-cpp uninstall clean-obj clean

all: dynamic

example:
	$(MAKE) -C .$(SEP)$(SRC_DIR) dynamic
	$(MAKE) -C .$(SEP)$(EXAMPLE_DIR)

example-cpp:
	$(MAKE) -C .$(SEP)$(SRC_DIR) dynamic-cpp
	$(MAKE) -C .$(SEP)$(EXAMPLE_DIR) example-cpp

dynamic:
	$(MAKE) -C .$(SEP)$(SRC_DIR) dynamic

dynamic-cpp:
	$(MAKE) -C .$(SEP)$(SRC_DIR) dynamic-cpp

static:
	$(MAKE) -C .$(SEP)$(SRC_DIR) static

static-cpp:
	$(MAKE) -C .$(SEP)$(SRC_DIR) static-cpp

exec:
	$(MAKE) -C .$(SEP)$(SRC_DIR) exec

install:
ifeq ($(detected_OS),Windows)
	echo "Not supported"
else
	mkdir -p $(bindir)
	mkdir -p $(includedir)
	mkdir -p $(libdir)
	install -C include/ksv.h $(includedir)
	if [ -e dist/ksv ]; then install -C dist/ksv $(bindir); fi
	if [ -e dist/libksv.a ]; then install -C dist/libksv.a $(libdir); fi
	if [ -e dist/libksv.so ]; then install -C dist/libksv.so $(libdir); fi
	if [ -e dist/libksv.dylib ]; then install -C dist/libksv.dylib $(libdir); fi
endif

install-cpp:
ifeq ($(detected_OS),Windows)
	echo "Not supported"
else
	mkdir -p $(includedir)
	mkdir -p $(libdir)
	install -C include/ksv.hpp $(includedir)
	if [ -e dist/libksvpp.a ]; then install -C dist/libksvpp.a $(libdir); fi
	if [ -e dist/libksvpp.so ]; then install -C dist/libksvpp.so $(libdir); fi
	if [ -e dist/libksvpp.dylib ]; then install -C dist/libksvpp.dylib $(libdir); fi
endif

uninstall:
ifeq ($(detected_OS),Windows)
	echo "Not supported"
else
	$(RM) $(bindir)/ksv
	$(RM) $(includedir)/ksv.h
	$(RM) $(libdir)/libksv.a
	$(RM) $(libdir)/libksv.so
	$(RM) $(libdir)/libksv.dylib
endif

uninstall-cpp:
ifeq ($(detected_OS),Windows)
	echo "Not supported"
else
	$(RM) $(includedir)/ksv.hpp
	$(RM) $(libdir)/libksvpp.a
	$(RM) $(libdir)/libksvpp.so
	$(RM) $(libdir)/libksvpp.dylib
endif

clean-obj:
	$(MAKE) -C .$(SEP)$(SRC_DIR) clean-obj

clean:
	$(MAKE) -C .$(SEP)$(SRC_DIR) clean
	$(MAKE) -C .$(SEP)$(EXAMPLE_DIR) clean
