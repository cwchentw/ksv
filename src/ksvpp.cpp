#include <cstdlib>
#include <iostream>
#include <stdexcept>
extern "C" {
#include "ksv.h"
}
#include "ksv.hpp"

struct _ksv_obj_t {
    ksv_t *ksv;
};

KSV::KSV(std::string delimiter,
        std::string end_of_line,
        std::string quote)
{
    char *_delimiter = (char *) delimiter.c_str();
    char *_eol = (char *) end_of_line.c_str();
    char *_quote = (char *) quote.c_str();

    this->obj = (_ksv_obj_t *) malloc(sizeof(_ksv_obj_t));
    if (!(this->obj)) {
        throw std::bad_alloc();
    }

    this->obj->ksv = ksv_new(_delimiter, _eol, _quote);
    if (!(this->obj->ksv)) {
        throw std::bad_alloc();
    }
}

KSV::~KSV()
{
    ksv_delete((void *) this->obj->ksv);
    free((void *) this->obj);
}

std::string KSV::version()
{
    return KSV_VERSION;
}

bool KSV::load_table_with_header_strictly(FILE *stream)
{
    return KSV_SUCCESS == \
        ksv_load_table_with_header_strictly(this->obj->ksv, stream)
        ? true : false;
}

bool KSV::load_header(FILE *stream)
{
    return KSV_SUCCESS == ksv_load_header(this->obj->ksv, stream) ? true : false;
}

bool KSV::load_record(FILE *stream)
{
    return KSV_SUCCESS == ksv_load_record(this->obj->ksv, stream) ? true : false;
}

bool KSV::has_header()
{
    return TRUE == ksv_has_header(this->obj->ksv) ? true : false;
}

size_t KSV::row()
{
    return ksv_row(this->obj->ksv);
}

size_t KSV::col()
{
    return ksv_col(this->obj->ksv);
}

void KSV::restart()
{
    ksv_restart(this->obj->ksv);
}

std::string KSV::next_header()
{
    char *header = ksv_next_header(this->obj->ksv);

    return !header ? "" : header;
}

bool KSV::next_column()
{
    return KSV_SUCCESS == ksv_next_column(this->obj->ksv) ? true : false;
}

std::string KSV::next_data_by_column()
{
    char *field = ksv_next_data_by_column(this->obj->ksv);

    return !field ? "" : field;
}

bool KSV::next_row()
{
    return KSV_SUCCESS == ksv_next_row(this->obj->ksv) ? true : false;
}

std::string KSV::next_data_by_row()
{
    char *field = ksv_next_data_by_row(this->obj->ksv);

    return !field ? "" : field;
}
