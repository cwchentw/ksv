#ifndef KSV_H
#define KSV_H

#ifdef __cplusplus
    #include <cstdio>
#else
    #include <stdio.h>
#endif

#if _MSC_VER
    #include <windows.h>
#else
    #include "boolean.h"
#endif

typedef unsigned char KSV_STATUS;

#define  KSV_SUCCESS        0
#define  KSV_FAILURE        1
#define  KSV_NO_MEMORY      2
#define  KSV_ERROR_LEXING   3
#define  KSV_ERROR_PARSING  4
#define  KSV_INVALID_FILE   5

typedef struct ksv_t ksv_t;

#ifdef __cplusplus
extern "C" {
#endif

ksv_t * ksv_new_default(void);
ksv_t * ksv_new(char *delimeter, char *end_of_line, char quote);
void ksv_delete(void *self);
KSV_STATUS ksv_load_stream_with_header_strictly(ksv_t *self, FILE *stream);
size_t ksv_row(ksv_t *self);
size_t ksv_col(ksv_t *self);
void ksv_restart(ksv_t *self);
char * ksv_next_header(ksv_t *self);
BOOL ksv_next_column(ksv_t *self);
char * ksv_next_data_by_column(ksv_t *self);
BOOL ksv_next_row(ksv_t *self);
char * ksv_next_data_by_row(ksv_t *self);

#ifdef __cplusplus
}
#endif

#endif  /* KSV_H */
