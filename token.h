#ifndef CSV_TOKEN_H
#define CSV_TOKEN_H

typedef struct csv_token_t csv_token_t;

typedef unsigned char CSV_TOKEN_TYPE;

#define  CSV_TOKEN_STRING       0
#define  CSV_TOKEN_DELIMETER    1
#define  CSV_TOKEN_END_OF_LINE  2
#define  CSV_TOKEN_QUOTE        3

csv_token_t * csv_token_new(CSV_TOKEN_TYPE type, char *string);
void csv_token_delete(void *self);

#endif  /* CSV_TOKEN_H */