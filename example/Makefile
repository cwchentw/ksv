ifeq ($(OS),Windows_NT)
    detected_OS := Windows
else
    detected_OS := $(shell sh -c 'uname -s 2>/dev/null || echo not')
endif

ifeq ($(detected_OS),Windows)
	RM=del /q /f
	CP=copy
else
	CP=cp -f
endif

ifeq ($(detected_OS),Windows)
	SEP=\\
else
	SEP=/
endif

INCLUDE_DIR=include
DIST_DIR=dist
DATA_DIR=data

ifeq ($(detected_OS),Windows)
	EXEC_CSV_TO_TSV=csv_to_tsv.exe
else
	EXEC_CSV_TO_TSV=csv_to_tsv
endif

CL := cl icl

ifneq (,$(findstring $(CC),$(CL)))
	OBJS_CSV_TO_TSV=csv_to_tsv.obj
else
	OBJS_CSV_TO_TSV=csv_to_tsv.o
endif

ifneq (,$(findstring $(CC),$(CL)))
	LIB_STATIC=ksv.lib
	LIB_DYNAMIC=ksv.dll
	CFLAGS=/W4 /sdl /D_CRT_SECURE_NO_WARNINGS
else
	LIB_STATIC=libksv.a
ifeq ($(detected_OS),Darwin)
	LIB_DYNAMIC=libksv.dylib
else
ifeq ($(detected_OS),Windows)
	LIB_DYNAMIC=libksv.dll
else
	LIB_DYNAMIC=libksv.so
endif
endif
	CFLAGS=-Wall -Wextra -std=c89
endif


.PHONY: example clean

example: $(EXEC_CSV_TO_TSV)
ifeq ($(detected_OS),Linux)
	LD_LIBRARY_PATH=. .$(SEP)$(EXEC_CSV_TO_TSV) ..$(SEP)$(DATA_DIR)$(SEP)iris.csv
else
	.$(SEP)$(EXEC_CSV_TO_TSV) ..$(SEP)$(DATA_DIR)$(SEP)iris.csv
endif

$(EXEC_CSV_TO_TSV): $(OBJS_CSV_TO_TSV)
ifneq (,$(findstring $(CC),$(CL)))
	$(CP) ..$(SEP)$(DIST_DIR)$(SEP)$(LIB_STATIC) .
	$(CP) ..$(SEP)$(DIST_DIR)$(SEP)$(LIB_DYNAMIC) .
	$(CC) /Fe:$(EXEC_CSV_TO_TSV) $(OBJS_CSV_TO_TSV) $(LIB_STATIC)
else
	$(CP) ..$(SEP)$(DIST_DIR)$(SEP)$(LIB_DYNAMIC) .
	$(CC) -o $(EXEC_CSV_TO_TSV) $(OBJS_CSV_TO_TSV) $(LIB_DYNAMIC)
endif

%.obj: %.c
	$(CC) /c $< $(CFLAGS) /I..$(SEP)$(INCLUDE_DIR)

%.o: %.c
	$(CC) -c $< $(CFLAGS) -I ..$(SEP)$(INCLUDE_DIR)

clean:
	$(RM) $(EXEC_CSV_TO_TSV) $(OBJS_CSV_TO_TSV) \
		$(LIB_DYNAMIC) $(LIB_STATIC)
