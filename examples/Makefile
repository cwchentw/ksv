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
	EXEC_CSV_TO_TSV_CPP=csv_to_tsv_cpp.exe
else
	EXEC_CSV_TO_TSV=csv_to_tsv
	EXEC_CSV_TO_TSV_CPP=csv_to_tsv_cpp
endif

CL := cl icl

ifneq (,$(findstring $(CC),$(CL)))
	OBJS_CSV_TO_TSV=csv_to_tsv.obj
	OBJS_CSV_TO_TSV_CPP=csv_to_tsv_cpp.obj
else
	OBJS_CSV_TO_TSV=csv_to_tsv.o
	OBJS_CSV_TO_TSV_CPP=csv_to_tsv_cpp.o
endif

ifneq (,$(findstring $(CC),$(CL)))
	LIB_EXPORT=ksv.exp
	LIB_STATIC=ksv.lib
	LIB_DYNAMIC=ksv.dll
	LIB_EXPORT_CPP=ksvpp.exp
	LIB_STATIC_CPP=ksvpp.lib
	LIB_DYNAMIC_CPP=ksvpp.dll
	CFLAGS=/W4 /sdl /DWIN32 /D_WINDOWS /D_CRT_SECURE_NO_WARNINGS /DKSV_IMPORT_SYMBOLS
	CXXFLAGS=$(CFLAGS)
else
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
	CFLAGS=-Wall -Wextra -std=c89
	CXXFLAGS=-Wall -Wextra -std=c++98
endif

.PHONY: example clean

example: $(EXEC_CSV_TO_TSV)
ifeq ($(detected_OS),Windows)
	.$(SEP)$(EXEC_CSV_TO_TSV) ..$(SEP)$(DATA_DIR)$(SEP)iris_partial.csv
else
ifeq ($(detected_OS),Darwin)
	.$(SEP)$(EXEC_CSV_TO_TSV) ..$(SEP)$(DATA_DIR)$(SEP)iris_partial.csv
else
	LD_LIBRARY_PATH=. .$(SEP)$(EXEC_CSV_TO_TSV) ..$(SEP)$(DATA_DIR)$(SEP)iris_partial.csv
ifeq ($(detected_OS),Linux)
	LD_LIBRARY_PATH=. valgrind --leak-check=full .$(SEP)$(EXEC_CSV_TO_TSV) ..$(SEP)$(DATA_DIR)$(SEP)iris_partial.csv
endif  # Linux
endif  # Darwin
endif  # Windows

$(EXEC_CSV_TO_TSV): $(OBJS_CSV_TO_TSV)
ifneq (,$(findstring $(CC),$(CL)))
	$(CP) ..$(SEP)$(DIST_DIR)$(SEP)$(LIB_EXPORT) .
	$(CP) ..$(SEP)$(DIST_DIR)$(SEP)$(LIB_STATIC) .
	$(CP) ..$(SEP)$(DIST_DIR)$(SEP)$(LIB_DYNAMIC) .
	$(CC) /Fe:$(EXEC_CSV_TO_TSV) $(OBJS_CSV_TO_TSV) $(LIB_STATIC) $(LIB_EXPORT) $(CFLAGS)
else
	$(CP) ..$(SEP)$(DIST_DIR)$(SEP)$(LIB_DYNAMIC) .
	$(CC) -o $(EXEC_CSV_TO_TSV) $(OBJS_CSV_TO_TSV) $(LIB_DYNAMIC) $(CFLAGS)
endif

example-cpp: $(EXEC_CSV_TO_TSV_CPP)
ifeq ($(detected_OS),Windows)
	.$(SEP)$(EXEC_CSV_TO_TSV_CPP) ..$(SEP)$(DATA_DIR)$(SEP)iris_partial.csv
else
ifeq ($(detected_OS),Darwin)
	.$(SEP)$(EXEC_CSV_TO_TSV_CPP) ..$(SEP)$(DATA_DIR)$(SEP)iris_partial.csv
else
	LD_LIBRARY_PATH=. .$(SEP)$(EXEC_CSV_TO_TSV_CPP) ..$(SEP)$(DATA_DIR)$(SEP)iris_partial.csv
ifeq ($(detected_OS),Linux)
	LD_LIBRARY_PATH=. valgrind --leak-check=full .$(SEP)$(EXEC_CSV_TO_TSV_CPP) ..$(SEP)$(DATA_DIR)$(SEP)iris_partial.csv
endif  # Linux
endif  # Darwin
endif  # Windows

$(EXEC_CSV_TO_TSV_CPP): $(OBJS_CSV_TO_TSV_CPP)
ifneq (,$(findstring $(CC),$(CL)))
	$(CP) ..$(SEP)$(DIST_DIR)$(SEP)$(LIB_EXPORT_CPP) .
	$(CP) ..$(SEP)$(DIST_DIR)$(SEP)$(LIB_STATIC_CPP) .
	$(CP) ..$(SEP)$(DIST_DIR)$(SEP)$(LIB_DYNAMIC_CPP) .
	$(CC) /Fe:$(EXEC_CSV_TO_TSV_CPP) $(OBJS_CSV_TO_TSV_CPP) $(LIB_STATIC_CPP) $(LIB_EXPORT_CPP) $(CXXFLAGS) /EHsc
else
	$(CP) ..$(SEP)$(DIST_DIR)$(SEP)$(LIB_DYNAMIC_CPP) .
	$(CXX) -o $(EXEC_CSV_TO_TSV_CPP) $(OBJS_CSV_TO_TSV_CPP) $(LIB_DYNAMIC_CPP) $(CXXFLAGS)
endif

%.obj: %.c
	$(CC) /c $< /I..$(SEP)$(INCLUDE_DIR) $(CFLAGS) /MD

%.obj: %.cpp
	$(CC) /c $< /I..$(SEP)$(INCLUDE_DIR) $(CXXFLAGS) /MD /EHsc

%.o: %.c
	$(CC) -c $< -I ..$(SEP)$(INCLUDE_DIR) $(CFLAGS)

%.o: %.cpp
	$(CXX) -c $< -I ..$(SEP)$(INCLUDE_DIR) $(CXXFLAGS)

clean:
	$(RM) $(EXEC_CSV_TO_TSV) $(EXEC_CSV_TO_TSV_CPP) \
	$(OBJS_CSV_TO_TSV) $(OBJS_CSV_TO_TSV_CPP) \
		$(LIB_DYNAMIC) $(LIB_STATIC) $(LIB_EXPORT) \
		$(LIB_DYNAMIC_CPP) $(LIB_STATIC_CPP) $(LIB_EXPORT_CPP)
