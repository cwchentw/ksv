#ifndef KSV_H
#define KSV_H

#ifdef __cplusplus
    #include <cstdio>
#else
    #include <stdio.h>
#endif

#if _MSC_VER
    #include <windows.h>
#else  /* !_MSC_VER */
#ifdef __cplusplus
    #ifndef _BOOL_IS_DEFINED
        typedef bool BOOL;
        #define FALSE  false
        #define TRUE   true
        #define _BOOL_IS_DEFINED
    #endif  /* BOOL */
#else
    #if __STDC_VERSION__ < 199901L
        #ifndef _BOOL_IS_DEFINED
            typedef char BOOL;
            #define FALSE  0
            #define TRUE   1
            #define _BOOL_IS_DEFINED
        #endif  /* BOOL */
    #else
        #ifndef _BOOL_IS_DEFINED
            #include <stdbool.h>
            typedef bool BOOL;
            #define FALSE  false
            #define TRUE   true
            #define _BOOL_IS_DEFINED
        #endif  /* BOOL */
    #endif  /* C89 */
#endif  /* __cplusplus */
#endif  /* _MSC_VER */

typedef unsigned char KSV_STATUS;

#define  KSV_SUCCESS        0
#define  KSV_FAILURE        1
#define  KSV_NO_MEMORY      2
#define  KSV_ERROR_LEXING   3
#define  KSV_ERROR_PARSING  4
#define  KSV_INVALID_FILE   5

typedef struct ksv_t ksv_t;

#if _MSC_VER
    #ifndef KSV_IMPORT_SYMBOLS
        #define KSV_EXPORT_SYMBOLS
    #endif
#endif

#if _MSC_VER
    #if KSV_IMPORT_SYMBOLS
        #define KSV_PUBLIC __declspec(dllimport)
    #elif KSV_EXPORT_SYMBOLS
        #define KSV_PUBLIC __declspec(dllexport)
    #endif
#elif __GNUC__ >= 4 || __clang__
    #define KSV_PUBLIC __attribute__((__visibility__("default")))
#else
    #define KSV_PUBLIC
#endif

#if __GNUC__ >= 4 || __clang__
    #define KSV_PRIVATE __attribute__((__visibility__("hidden")))
#else
    #define KSV_PRIVATE
#endif

#ifdef __cplusplus
extern "C" {
#endif

KSV_PUBLIC ksv_t * ksv_new_default(void);
KSV_PUBLIC ksv_t * ksv_new(char *delimeter, char *end_of_line, char *quote);
KSV_PUBLIC void ksv_delete(void *self);
KSV_PUBLIC KSV_STATUS ksv_load_table_with_header_strictly(ksv_t *self, FILE *stream);
KSV_PUBLIC KSV_STATUS ksv_load_header(ksv_t *self, FILE *stream);
KSV_PUBLIC KSV_STATUS ksv_load_record(ksv_t *self, FILE *stream);
KSV_PUBLIC BOOL ksv_has_header(ksv_t *self);
KSV_PUBLIC size_t ksv_row(ksv_t *self);
KSV_PUBLIC size_t ksv_col(ksv_t *self);
KSV_PUBLIC void ksv_restart(ksv_t *self);
KSV_PUBLIC char * ksv_next_header(ksv_t *self);
KSV_PUBLIC BOOL ksv_next_column(ksv_t *self);
KSV_PUBLIC char * ksv_next_data_by_column(ksv_t *self);
KSV_PUBLIC BOOL ksv_next_row(ksv_t *self);
KSV_PUBLIC char * ksv_next_data_by_row(ksv_t *self);

#ifdef __cplusplus
}
#endif

#endif  /* KSV_H */
