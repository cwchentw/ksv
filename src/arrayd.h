#ifndef ARRAYD_H
#define ARRAYD_H

#include <stddef.h>

#define  ARRAYD_VERSION  "0.1.0"

#define  ARRAYD_VERSION_MAJOR  0
#define  ARRAYD_VERSION_MINOR  1
#define  ARRAYD_VERSION_PATCH  0

#if _MSC_VER
    #if defined(ARRAYD_IMPORT_SYMBOLS)
        #define ARRAYD_PUBLIC __declspec(dllimport)
    #elif defined(ARRAYD_EXPORT_SYMBOLS)
        #define ARRAYD_PUBLIC __declspec(dllexport)
    #else
        #define ARRAYD_PUBLIC
    #endif
#elif __GNUC__ >= 4 || __clang__
    #define ARRAYD_PUBLIC __attribute__((__visibility__("default")))
#else
    #define ARRAYD_PUBLIC
#endif

#if __GNUC__ >= 4 || __clang__
    #define ARRAYD_PRIVATE __attribute__((__visibility__("hidden")))
#else
    #define ARRAYD_PRIVATE
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

typedef struct arrayd_t arrayd_t;

#ifdef __cplusplus
extern "C" {
#endif

ARRAYD_PUBLIC arrayd_t * arrayd_new(void);
ARRAYD_PUBLIC void arrayd_delete(void *self);
ARRAYD_PUBLIC double arrayd_at(arrayd_t *self, size_t index);
ARRAYD_PUBLIC BOOL arrayd_push(arrayd_t *self, double data);
ARRAYD_PUBLIC void arrayd_sort(arrayd_t *self);

#ifdef __cplusplus
}
#endif

#endif  /* ARRAYD_H */
