#include <assert.h>
#include <stdlib.h>
#include "print.h"
#include "ksv_token.h"

struct ksv_token_t {
    KSV_TOKEN_TYPE type;
    char *string;
};

ksv_token_t * ksv_token_new(KSV_TOKEN_TYPE type, char *string)
{
    ksv_token_t *token = \
        (ksv_token_t *) malloc(sizeof(ksv_token_t));
    if (!token) {
        PUTERR("Failed to allocate memory for csv token");
        PUTERR("Check available system memory");
        return token;
    }

    token->type = type;
    token->string = string;

    return token;
}

void ksv_token_delete(void *self)
{
    assert(self);

    char *string = ((ksv_token_t *) self)->string;
    if (string)
        free((void *) string);

    free(self);
}
