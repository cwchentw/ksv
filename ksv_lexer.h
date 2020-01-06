#ifndef KSV_LEXER_H
#define KSV_LEXER_H

#include "ksv.h"

typedef struct ksv_lexer_t ksv_lexer_t;

ksv_lexer_t * ksv_lexer_new(char *delimeter, char *end_of_line, char quote);
BOOL ksv_lexer_lex(ksv_lexer_t *self, char *input);
void ksv_lexer_delete(void *self);

#endif  /* KSV_LEXER_H */