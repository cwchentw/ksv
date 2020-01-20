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
	CFLAGS=/W4 /sdl /D_CRT_SECURE_NO_WARNINGS
else
	LIB_STATIC=libksv.a
ifeq ($(detected_OS),Darwin)
	LIB_DYNAMIC=libksv.dylib
else
	LIB_DYNAMIC=libksv.so
endif
	CFLAGS=-Wall -Wextra -std=c89 -fvisibility=hidden
endif

ifneq (,$(DEBUG))
ifneq (,$(findstring $(CC),$(CL)))
	CFLAGS+=/D DEBUG
else
	CFLAGS+=-DDEBUG
endif
endif

ifneq (,$(findstring $(CC),$(CL)))
	CFLAGS+=/I ..$(SEP)$(INCLUDE_DIR)
else
	CFLAGS+=-I ..$(SEP)$(INCLUDE_DIR)
endif

ifneq (,$(findstring $(CC),$(CL)))
	OBJS=cstring.obj ksv_token.obj ksv_lexer.obj \
		ksv_ast.obj ksv_parser.obj ksv.obj
	EXEC_OBJS=ksv_help.obj ksv_argument.obj ksv_cli.obj
else
	OBJS=cstring.o ksv_token.o ksv_lexer.o \
		ksv_ast.o ksv_parser.o ksv.o
	EXEC_OBJS=ksv_help.o ksv_argument.o ksv_cli.o
endif


.PHONY: all dynamic static exec clean_obj clean

all:
	$(MAKE) dynamic
	$(MAKE) clean_obj
	$(MAKE) static
	$(MAKE) exec

exec: $(TARGET)

$(TARGET): $(LIB_STATIC) $(EXEC_OBJS)
ifneq (,$(findstring $(CC),$(CL)))
	$(CC) /Fe:..$(SEP)$(DIST_DIR)$(SEP)$(TARGET) $(EXEC_OBJS) \
		..$(SEP)$(DIST_DIR)$(SEP)$(LIB_STATIC) $(CFLAGS)
else
	$(CC) -o ..$(SEP)$(DIST_DIR)$(SEP)$(TARGET) $(EXEC_OBJS) \
		..$(SEP)$(DIST_DIR)$(SEP)$(LIB_STATIC) $(CFLAGS)
endif

dynamic: $(LIB_DYNAMIC)

$(LIB_DYNAMIC): $(OBJS)
ifneq (,$(findstring $(CC),$(CL)))
	link /DLL /OUT:..$(SEP)$(DIST_DIR)$(SEP)$(LIB_DYNAMIC) $(OBJS)
else
	$(CC) -shared -o ..$(SEP)$(DIST_DIR)$(SEP)$(LIB_DYNAMIC) $(OBJS)
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

%.obj: %.c
	$(CC) /c $< $(CFLAGS)

%.o: %.c
ifeq (dynamic,$(MAKECMDGOALS))
	$(CC) -fPIC -c $< $(CFLAGS)
else
	$(CC) -c $< $(CFLAGS)
endif

clean_obj:
	$(RM) $(OBJS)

clean:
	$(RM) $(OBJS) $(EXEC_OBJS) \
	..$(SEP)$(DIST_DIR)$(SEP)$(LIB_DYNAMIC) \
	..$(SEP)$(DIST_DIR)$(SEP)$(LIB_STATIC) \
	..$(SEP)$(DIST_DIR)$(SEP)$(TARGET)
