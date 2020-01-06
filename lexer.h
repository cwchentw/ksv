#ifndef CSV_LEXER_H
#define CSV_LEXER_H

#include "csv.h"

typedef struct csv_lexer_t csv_lexer_t;

csv_lexer_t * csv_lexer_new(char *delimeter, char *end_of_line, char quote);
BOOL csv_lexer_lex(csv_lexer_t *self, char *input);
void csv_lexer_delete(void *self);

#endif  /* CSV_LEXER_H */