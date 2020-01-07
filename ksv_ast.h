#ifndef KSV_AST_H
#define KSV_AST_H

typedef struct ksv_ast_t ksv_ast_t;

typedef unsigned char KSV_AST_TYPE;
#define  KSV_AST_FIELD      0
#define  KSV_AST_DELIMITER  1
#define  KSV_AST_EOL        2

ksv_ast_t * ksv_ast_new(KSV_AST_TYPE type);
void ksv_ast_delete(void *self);

#endif  /* KSV_AST_H */