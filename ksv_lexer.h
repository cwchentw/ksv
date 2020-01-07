#ifndef KSV_LEXER_H
#define KSV_LEXER_H

#include "ksv.h"
#include "ksv_token.h"

typedef struct ksv_lexer_t ksv_lexer_t;

ksv_lexer_t * ksv_lexer_new(char *delimeter, char *end_of_line, char quote);
void ksv_lexer_delete(void *self);
BOOL ksv_lexer_lex(ksv_lexer_t *self, char *input);
void ksv_lexer_start(ksv_lexer_t *self);
ksv_token_t * ksv_lexer_next(ksv_lexer_t *self);
ksv_token_t * ksv_lexer_peek(ksv_lexer_t *self, size_t n);

#endif  /* KSV_LEXER_H */