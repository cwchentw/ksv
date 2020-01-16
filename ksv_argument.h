#ifndef KSV_ARGUMENT_H
#define KSV_ARGUMENT_H

#include "ksv_command.h"

typedef struct ksv_argument_t ksv_argument_t;

ksv_argument_t * ksv_argument_parse(int argc, char *argv[]);
void ksv_argument_delete(void *self);
KSV_COMMAND_TYPE ksv_argument_command(ksv_argument_t *self);

#endif  /* KSV_ARGUMENT_H */
