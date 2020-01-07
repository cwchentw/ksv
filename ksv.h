#ifndef KSV_H
#define KSV_H

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

typedef struct ksv_t ksv_t;

#ifdef __cplusplus
extern "C" {
#endif

ksv_t * ksv_new_default(void);
ksv_t * ksv_new(char *delimeter, char *end_of_line, char quote);
BOOL ksv_load_stream_with_header_strictly(ksv_t *self, FILE *stream);
void ksv_delete(void *self);

#ifdef __cplusplus
}
#endif

#endif  /* KSV_H */
