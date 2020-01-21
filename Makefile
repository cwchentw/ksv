ifeq ($(OS),Windows_NT)
    detected_OS := Windows
else
    detected_OS := $(shell sh -c 'uname -s 2>/dev/null || echo not')
endif

ifeq ($(detected_OS),Windows)
	SEP=\\
else
	SEP=/
endif

SRC_DIR=src
EXAMPLE_DIR=example


.PHONY: all example dynamic dynamic-cpp static static-cpp exec clean

all: dynamic

example:
	$(MAKE) -C .$(SEP)$(SRC_DIR) dynamic
	$(MAKE) -C .$(SEP)$(SRC_DIR) dynamic-cpp
	$(MAKE) -C .$(SEP)$(EXAMPLE_DIR)

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

clean:
	$(MAKE) -C .$(SEP)$(SRC_DIR) clean
	$(MAKE) -C .$(SEP)$(EXAMPLE_DIR) clean
