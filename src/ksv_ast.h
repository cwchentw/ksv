#ifndef KSV_AST_H
#define KSV_AST_H

#include "ksv.h"
#include "ksv_token.h"

typedef struct ksv_ast_t ksv_ast_t;

typedef unsigned char KSV_AST_TYPE;
#define  KSV_AST_FIELD      0
#define  KSV_AST_DELIMITER  1
#define  KSV_AST_EOL        2

ksv_ast_t * ksv_ast_new(KSV_AST_TYPE type);
void ksv_ast_delete(void *self);
KSV_STATUS ksv_ast_add_token(ksv_ast_t *self, ksv_token_t *token);
KSV_AST_TYPE ksv_ast_type(ksv_ast_t *self);
char * ksv_ast_string(ksv_ast_t *self);

#endif  /* KSV_AST_H */
