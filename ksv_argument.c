#include <assert.h>
#include <stdlib.h>
#include "ksv_argument.h"
#include "print.h"

struct ksv_argument_t {
    /* Declare it later. */
};

static ksv_argument_t * ksv_argument_new(void);

ksv_argument_t * ksv_argument_parse(int argc, char *argv[])
{
    ksv_argument_t *arg = ksv_argument_new();
    if (!arg)
        return arg;

    return arg;
}

static ksv_argument_t * ksv_argument_new(void)
{
    ksv_argument_t *arg = \
        (ksv_argument_t *) malloc(sizeof(ksv_argument_t));
    if (!arg) {
    #if DEBUG
        PUTERR("Failed to allocate memory for ksv argument object");
        PUTERR("Check available system memory");
    #endif
        return arg;
    }

    return arg;
}

void ksv_argument_delete(void *self)
{
    assert(self);

    free(self);
}
