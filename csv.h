#ifndef CSV_H
#define CSV_H

#ifdef __cplusplus
    #include <cstdio>
#else
    #include <stdio.h>
#endif

#if _MSC_VER
    #ifndef WIN32_LEAN_AND_MEAN
        #define WIN32_LEAN_AND_MEAN
    #endif
    #include <windows.h>
#else
    #include "boolean.h"
#endif

typedef struct csv_t csv_t;

#ifdef __cplusplus
extern "C" {
#endif

csv_t * csv_new_default(void);
csv_t * csv_new(char *delimeter, char *end_of_line, char quote);
BOOL csv_load_stream_with_header_strictly(csv_t *self, FILE *stream);
void csv_delete(void *self);

#ifdef __cplusplus
}
#endif

#endif  /* CSV_H */