#ifndef KSV_PARSER_H
#define KSV_PARSER_H

#include "ksv.h"
#include "ksv_ast.h"
#include "ksv_lexer.h"

typedef struct ksv_parser_t ksv_parser_t;

KSV_PRIVATE ksv_parser_t * ksv_parser_new(void);
KSV_PRIVATE void ksv_parser_delete(void *self);
KSV_PRIVATE KSV_STATUS ksv_parser_parse(ksv_parser_t *self, ksv_lexer_t *lexer);
KSV_PRIVATE ksv_ast_t * ksv_parser_next(ksv_parser_t *self);

#endif  /* KSV_PARSER_H */
