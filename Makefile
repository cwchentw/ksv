ifeq ($(OS),Windows_NT)
    detected_OS := Windows
else
    detected_OS := $(shell sh -c 'uname -s 2>/dev/null || echo not')
endif

ifneq (,$(findstring $(CC),cl.exe))
	RM=del /q /f
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

ifneq (,$(findstring $(CC),cl.exe))
	OBJS=cstring.obj ksv_token.obj ksv_lexer.obj ksv.obj
else
	OBJS=cstring.o ksv_token.o ksv_lexer.o ksv.o
endif


.PHONY: all dynamic static clean_obj clean

all:
	$(MAKE) dynamic
	$(MAKE) clean_obj
	$(MAKE) static

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
	$(RM) $(OBJS) $(LIB_DYNAMIC) $(LIB_STATIC)
