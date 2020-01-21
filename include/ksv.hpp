#ifndef KSV_HPP
#define KSV_HPP

#if _MSC_VER
    #define KSV_STDCALL __stdcall
#else
    #define KSV_STDCALL
#endif

#if _MSC_VER
    #if defined(KSV_IMPORT_SYMBOLS)
        #define KSV_PUBLIC(type) __declspec(dllimport) type KSV_STDCALL
    #elif defined(KSV_EXPORT_SYMBOLS)
        #define KSV_PUBLIC(type) __declspec(dllexport) type KSV_STDCALL
    #else
        #define KSV_PUBLIC(type) type
    #endif
#elif __GNUC__ >= 4 || __clang__
    #define KSV_PUBLIC(type) __attribute__((__visibility__("default"))) type
#else
    #define KSV_PUBLIC(type) type
#endif

#if __GNUC__ >= 4 || __clang__
    #define KSV_PRIVATE __attribute__((__visibility__("hidden")))
#else
    #define KSV_PRIVATE
#endif

#ifdef __cplusplus
    #include <cstdio>
#else
    #include <stdio.h>
#endif

#include <cstdio>
#include <iostream>

using std::string;

typedef struct _ksv_obj_t _ksv_obj_t;

class KSV_PUBLIC(KSV)
{
public:
    KSV(std::string delimiter = ",",
        std::string end_of_line = "\n",
        std::string quote = "\"");
    ~KSV();
    std::string version();
    bool load_table_with_header_strictly(FILE *stream);
    bool load_header(FILE *stream);
    bool load_record(FILE *stream);
    bool has_header();
    size_t row();
    size_t col();
    void restart();
    std::string next_header();
    bool next_column();
    std::string next_data_by_column();
    bool next_row();
    std::string next_data_by_row();
private:
    _ksv_obj_t *obj;
};

#endif  /* KSV_HPP */
