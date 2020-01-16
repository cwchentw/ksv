#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "ksv_argument.h"
#include "ksv_command.h"
#include "print.h"

struct ksv_argument_t {
    KSV_COMMAND_TYPE command;
};

static ksv_argument_t * ksv_argument_new(void);

ksv_argument_t * ksv_argument_parse(int argc, char *argv[])
{
    ksv_argument_t *arg = ksv_argument_new();
    if (!arg)
        return arg;

    {
        int i;
        for (i = 1; i < argc; ++i) {
            if (0 == strcmp("-v", argv[i])
                || 0 == strcmp("--version", argv[i])) {
                arg->command = KSV_COMMAND_VERSION;
                break;
            }
            else if (0 == strcmp("--license", argv[i])) {
                arg->command = KSV_COMMAND_LICENSE;
                break;
            }
        }
    }

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

    arg->command = KSV_COMMAND_UNKNOWN;

    return arg;
}

void ksv_argument_delete(void *self)
{
    assert(self);

    free(self);
}

KSV_COMMAND_TYPE ksv_argument_command(ksv_argument_t *self)
{
    assert(self);

    return self->command;
}
