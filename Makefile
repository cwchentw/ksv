ifeq ($(OS),Windows_NT)
    detected_OS := Windows
else
    detected_OS := $(shell sh -c 'uname -s 2>/dev/null || echo not')
endif

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

ifneq (,$(findstring $(CC),cl.exe))
	LIB_STATIC=ksv.lib
	LIB_DYNAMIC=ksv.dll
	CFLAGS=/W4 /sdl
else
	LIB_STATIC=libksv.a
ifeq ($(detected_OS),Darwin)
	LIB_DYNAMIC=libksv.dylib
else
	LIB_DYNAMIC=libksv.so
endif
	CFLAGS=-Wall -Wextra -std=c89
endif

ifneq (,$(DEBUG))
ifneq (,$(findstring $(CC),cl.exe))
	CFLAGS+=/D DEBUG
else
	CFLAGS+=-DDEBUG
endif
endif

ifneq (,$(findstring $(CC),cl.exe))
	OBJS=cstring.obj ksv_token.obj ksv_lexer.obj ksv.obj
	EXEC_OBJS=ksv_cli.obj
else
	OBJS=cstring.o ksv_token.o ksv_lexer.o ksv.o
	EXEC_OBJS=ksv_cli.o
endif


.PHONY: all dynamic static exec clean_obj clean

all:
	$(MAKE) dynamic
	$(MAKE) clean_obj
	$(MAKE) static
	$(MAKE) exec

exec: $(TARGET)

$(TARGET): $(LIB_STATIC) $(EXEC_OBJS)
ifneq (,$(findstring $(CC),cl.exe))
	$(CC) /Fe: $(TARGET) $(EXEC_OBJS) $(LIB_STATIC) $(CFLAGS)
else
	$(CC) -o $(TARGET) $(EXEC_OBJS) $(LIB_STATIC) $(CFLAGS)
endif

dynamic: $(LIB_DYNAMIC)

$(LIB_DYNAMIC): $(OBJS)
ifneq (,$(findstring $(CC),cl.exe))
	link /DLL /OUT:$(LIB_DYNAMIC) $(OBJS)
else
	$(CC) -shared -o $(LIB_DYNAMIC) $(OBJS)
endif

static: $(LIB_STATIC)

$(LIB_STATIC): $(OBJS)
ifneq (,$(findstring $(CC),cl.exe))
	lib /out:$(LIB_STATIC) $(OBJS)
else
ifeq ($(detected_OS),Darwin)
	libtool -static -o $(LIB_STATIC) $(OBJS)
else
	$(AR) rcs $(LIB_STATIC) $(OBJS)
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
	$(RM) $(OBJS) $(EXEC_OBJS) $(LIB_DYNAMIC) $(LIB_STATIC) $(TARGET)
