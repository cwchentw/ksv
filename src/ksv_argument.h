#ifndef KSV_ARGUMENT_H
#define KSV_ARGUMENT_H

#include "ksv.h"
#include "ksv_command.h"

typedef struct ksv_argument_t ksv_argument_t;

KSV_PRIVATE ksv_argument_t * ksv_argument_parse(int argc, char *argv[]);
KSV_PRIVATE void ksv_argument_delete(void *self);
KSV_PRIVATE KSV_COMMAND_TYPE ksv_argument_command(ksv_argument_t *self);
KSV_PRIVATE KSV_COMMAND_TYPE ksv_argument_subcommand(ksv_argument_t *self);
KSV_PRIVATE char * ksv_argument_path(ksv_argument_t *self);

#endif  /* KSV_ARGUMENT_H */
