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
    #if DEBUG
        PUTERR("Failed to allocate memory for csv token");
        PUTERR("Check available system memory");
    #endif
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

KSV_TOKEN_TYPE ksv_token_type(ksv_token_t *self)
{
    assert(self);

    return self->type;
}

char * ksv_token_string(ksv_token_t *self)
{
    assert(self);

    return self->string;
}
