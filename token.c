#include <assert.h>
#include <stdlib.h>
#include "print.h"
#include "token.h"

struct csv_token_t {
    CSV_TOKEN_TYPE type;
    char *string;
};

csv_token_t * csv_token_new(CSV_TOKEN_TYPE type, char *string)
{
    csv_token_t *token = \
        (csv_token_t *) malloc(sizeof(csv_token_t));
    if (!token) {
        PUTERR("Failed to allocate memory for csv token");
        PUTERR("Check available system memory");
        return token;
    }

    token->type = type;
    token->string = string;

    return token;
}

void csv_token_delete(void *self)
{
    assert(self);

    char *string = ((csv_token_t *) self)->string;
    if (string)
        free((void *) string);

    free(self);
}
