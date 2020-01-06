ifeq ($(OS),Windows_NT)
    detected_OS := Windows
else
    detected_OS := $(shell sh -c 'uname -s 2>/dev/null || echo not')
endif

ifneq (,$(findstring $(CC),cl.exe))
	RM=del /q /f
endif

ifneq (,$(findstring $(CC),cl.exe))
	CFLAGS=/W4 /sdl
else
	CFLAGS=-Wall -Wextra -std=c89
endif

ifneq (,$(findstring $(CC),cl.exe))
	OBJS=cstring.obj token.obj lexer.obj csv.obj
else
	OBJS=cstring.o token.o lexer.o csv.o
endif


.PHONY: all clean

all: $(OBJS)

%.obj: %.c
	$(CC) /c $< $(CFLAGS)

%.o: %.c
	$(CC) -c $< $(CFLAGS)

clean:
	$(RM) $(OBJS)
