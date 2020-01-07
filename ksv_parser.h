#ifndef KSV_PARSER_H
#define KSV_PARSER_H

#include "ksv.h"
#include "ksv_lexer.h"

typedef struct ksv_parser_t ksv_parser_t;

ksv_parser_t * ksv_parser_new(void);
void ksv_parser_delete(void *self);
BOOL ksv_parser_parse(ksv_parser_t *self, ksv_lexer_t *lexer);

#endif  /* KSV_PARSER_H */
