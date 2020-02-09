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

INCLUDE_DIR=include
DIST_DIR=dist

ifeq ($(detected_OS),Windows)
	RM=del /q /f
endif

CC=
ifeq (,$(CC))
ifeq ($(detected_OS),Windows)
	CC=cl.exe
else
ifeq ($(detected_OS),Darwin)
	CC=clang
else
	CC=gcc
endif  # Darwin
endif  # Windows
endif  # CC

ifeq ($(detected_OS),Windows)
	TARGET=ksv.exe
else
	TARGET=ksv
endif

CL := cl icl

ifneq (,$(findstring $(CC),$(CL)))
	LIB_STATIC=ksv.lib
	LIB_DYNAMIC=ksv.dll
	LIB_STATIC_CPP=ksvpp.lib
	LIB_DYNAMIC_CPP=ksvpp.dll
	CFLAGS=/W4 /sdl /DWIN32 /D_WINDOWS /D_CRT_SECURE_NO_WARNINGS /GS
	CXXFLAGS=$(CFLAGS)
else
	LIB_STATIC=libksv.a
	LIB_STATIC_CPP=libksvpp.a
ifeq ($(detected_OS),Darwin)
	LIB_DYNAMIC=libksv.dylib
	LIB_DYNAMIC_CPP=libksvpp.dylib
else
ifeq ($(detected_OS),Windows)
	LIB_DYNAMIC=libksv.dll
	LIB_DYNAMIC_CPP=libksvpp.dll
else
	LIB_DYNAMIC=libksv.so
	LIB_DYNAMIC_CPP=libksvpp.so
endif
endif
	CFLAGS=-Wall -Wextra -std=c89 -fvisibility=hidden
	CXXFLAGS=-Wall -Wextra -std=c++98 -fvisibility=hidden
endif

DYNAMIC := dynamic dynamic-cpp

ifneq (,$(findstring $(MAKECMDGOALS),$(DYNAMIC)))
ifneq (,$(findstring $(CC),$(CL)))
	CFLAGS+=/DKSV_EXPORT_SYMBOLS
	CXXFLAGS+=/DKSV_EXPORT_SYMBOLS
endif  # CL
endif  # dynamic

ifneq (,$(DEBUG))
ifneq (,$(findstring $(CC),$(CL)))
	CFLAGS+=/D DEBUG
	CXXFLAGS+=/D DEBUG
else
	CFLAGS+=-DDEBUG
	CXXFLAGS+=-DDEBUG
endif
endif

ifneq (,$(findstring $(CC),$(CL)))
	CFLAGS+=/I ..$(SEP)$(INCLUDE_DIR)
	CXXFLAGS+=/I ..$(SEP)$(INCLUDE_DIR)
else
	CFLAGS+=-I ..$(SEP)$(INCLUDE_DIR)
	CXXFLAGS+=-I ..$(SEP)$(INCLUDE_DIR)
endif

ifneq (,$(findstring $(CC),$(CL)))
	OBJS=cstring.obj ksv_token.obj ksv_lexer.obj \
		ksv_ast.obj ksv_parser.obj ksv.obj
	OBJS_CPP=$(OBJS) ksvpp.obj
	EXEC_OBJS=ksv_help.obj ksv_argument.obj arrayd.obj ksv_cli.obj
else
	OBJS=cstring.o ksv_token.o ksv_lexer.o \
		ksv_ast.o ksv_parser.o ksv.o
	OBJS_CPP=$(OBJS) ksvpp.o
	EXEC_OBJS=ksv_help.o ksv_argument.o arrayd.o ksv_cli.o
endif

DYNAMIC := dynamic dynamic-cpp


.PHONY: all dynamic dynamic-cpp static static-cpp exec clean_obj clean

all: dynamic

dynamic: $(LIB_DYNAMIC)

$(LIB_DYNAMIC): $(OBJS)
ifneq (,$(findstring $(CC),$(CL)))
	link /DLL /OUT:..$(SEP)$(DIST_DIR)$(SEP)$(LIB_DYNAMIC) $(OBJS)
else
	$(CC) -shared -o ..$(SEP)$(DIST_DIR)$(SEP)$(LIB_DYNAMIC) $(OBJS)
endif

dynamic-cpp: $(LIB_DYNAMIC_CPP)

$(LIB_DYNAMIC_CPP): $(OBJS_CPP)
ifneq (,$(findstring $(CC),$(CL)))
	link /DLL /OUT:..$(SEP)$(DIST_DIR)$(SEP)$(LIB_DYNAMIC_CPP) $(OBJS_CPP)
else
	$(CXX) -shared -o ..$(SEP)$(DIST_DIR)$(SEP)$(LIB_DYNAMIC_CPP) $(OBJS_CPP)
endif

static: $(LIB_STATIC)

$(LIB_STATIC): $(OBJS)
ifneq (,$(findstring $(CC),$(CL)))
	lib /out:..$(SEP)$(DIST_DIR)$(SEP)$(LIB_STATIC) $(OBJS)
else
ifeq ($(detected_OS),Darwin)
	libtool -static -o ..$(SEP)$(DIST_DIR)$(SEP)$(LIB_STATIC) $(OBJS)
else
	$(AR) rcs ..$(SEP)$(DIST_DIR)$(SEP)$(LIB_STATIC) $(OBJS)
endif
endif

static-cpp: $(LIB_STATIC_CPP)

$(LIB_STATIC_CPP): $(OBJS_CPP)
ifneq (,$(findstring $(CC),$(CL)))
	lib /out:..$(SEP)$(DIST_DIR)$(SEP)$(LIB_STATIC_CPP) $(OBJS_CPP)
else
ifeq ($(detected_OS),Darwin)
	libtool -static -o ..$(SEP)$(DIST_DIR)$(SEP)$(LIB_STATIC_CPP) $(OBJS_CPP)
else
	$(AR) rcs ..$(SEP)$(DIST_DIR)$(SEP)$(LIB_STATIC_CPP) $(OBJS_CPP)
endif
endif

exec: $(TARGET)

$(TARGET): $(LIB_STATIC) $(EXEC_OBJS)
ifneq (,$(findstring $(CC),$(CL)))
	$(CC) /Fe:..$(SEP)$(DIST_DIR)$(SEP)$(TARGET) $(EXEC_OBJS) \
		..$(SEP)$(DIST_DIR)$(SEP)$(LIB_STATIC) $(CFLAGS)
else
	$(CC) -o ..$(SEP)$(DIST_DIR)$(SEP)$(TARGET) $(EXEC_OBJS) \
		..$(SEP)$(DIST_DIR)$(SEP)$(LIB_STATIC) $(CFLAGS)
endif

%.obj: %.c
ifneq (,$(findstring $(MAKECMDGOALS),$(DYNAMIC)))
	$(CC) /c $< $(CFLAGS) /MD
else
	$(CC) /c $< $(CFLAGS) /MT
endif

%.obj: %.cpp
ifeq (dynamic-cpp,$(MAKECMDGOALS))
	$(CC) /c $< $(CXXFLAGS) /MD
else
	$(CC) /c $< $(CXXFLAGS) /MT
endif

%.o: %.c
ifneq (,$(findstring $(MAKECMDGOALS),$(DYNAMIC)))
	$(CC) -fPIC -c $< $(CFLAGS)
else
	$(CC) -c $< $(CFLAGS)
endif

%.o: %.cpp
ifeq (dynamic-cpp,$(MAKECMDGOALS))
	$(CXX) -fPIC -c $< $(CXXFLAGS)
else
	$(CXX) -c $< $(CXXFLAGS)
endif

clean_obj:
	$(RM) $(OBJS_CPP)

clean:
	$(RM) $(OBJS_CPP) $(EXEC_OBJS) \
	..$(SEP)$(DIST_DIR)$(SEP)$(LIB_DYNAMIC) \
	..$(SEP)$(DIST_DIR)$(SEP)$(LIB_DYNAMIC_CPP) \
	..$(SEP)$(DIST_DIR)$(SEP)$(LIB_STATIC) \
	..$(SEP)$(DIST_DIR)$(SEP)$(LIB_STATIC_CPP) \
	..$(SEP)$(DIST_DIR)$(SEP)$(TARGET) \
	..$(SEP)$(DIST_DIR)$(SEP)*.exp
