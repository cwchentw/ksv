#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "ksv_argument.h"
#include "ksv_command.h"
#include "print.h"

struct ksv_argument_t {
    char *path;
    KSV_COMMAND_TYPE command;
    KSV_COMMAND_TYPE subcommand;
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
            if (0 == strcmp("version", argv[i])) {
                arg->command = KSV_COMMAND_VERSION;
                break;
            }
            else if (0 == strcmp("license", argv[i])) {
                arg->command = KSV_COMMAND_LICENSE;
                break;
            }
            else if (0 == strcmp("help", argv[i])) {
                arg->command = KSV_COMMAND_HELP;
                break;
            }
            else if (0 == strcmp("width", argv[i])) {
                arg->command = KSV_COMMAND_WIDTH;
            }
            else if (0 == strcmp("height", argv[i])) {
                arg->command = KSV_COMMAND_HEIGHT;
            }
            else if (0 == strcmp("dimension", argv[i])) {
                arg->command = KSV_COMMAND_DIMENSION;
            }
            else if (0 == strcmp("header", argv[i])) {
                arg->command = KSV_COMMAND_HEADER;
            }
            else if (0 == strcmp("table", argv[i])) {
                arg->command = KSV_COMMAND_SHOW;
            }
            else if (0 == strcmp("stats", argv[i])) {
                if (i + 1 >= argc) {
                    break;
                }

                arg->command = KSV_COMMAND_STATS;

                if (0 == strcmp("quartiles", argv[i+1])) {
                    arg->subcommand = KSV_STATS_COMMAND_QUARTILES;
                    i += 1;
                }
                else if (0 == strcmp("quintiles", argv[i+1])) {
                    arg->subcommand = KSV_STATS_COMMAND_QUINTILES;
                    i += 1;
                }
            }
            else {
                arg->path = argv[i];
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

    arg->path = NULL;
    arg->command = KSV_COMMAND_UNKNOWN;
    arg->subcommand = KSV_COMMAND_UNKNOWN;

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

KSV_COMMAND_TYPE ksv_argument_subcommand(ksv_argument_t *self)
{
    assert(self);

    return self->subcommand;
}

char * ksv_argument_path(ksv_argument_t *self)
{
    assert(self);

    return self->path;
}
