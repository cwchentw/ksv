#ifndef KSV_PARSER_H
#define KSV_PARSER_H

#include "ksv.h"
#include "ksv_ast.h"
#include "ksv_lexer.h"

typedef struct ksv_parser_t ksv_parser_t;

ksv_parser_t * ksv_parser_new(void);
void ksv_parser_delete(void *self);
KSV_STATUS ksv_parser_parse(ksv_parser_t *self, ksv_lexer_t *lexer);
ksv_ast_t * ksv_parser_next(ksv_parser_t *self);

#endif  /* KSV_PARSER_H */
