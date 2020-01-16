#ifndef KSV_ARGUMENT_H
#define KSV_ARGUMENT_H

typedef struct ksv_argument_t ksv_argument_t;

ksv_argument_t * ksv_argument_parse(int argc, char *argv[]);
void ksv_argument_delete(void *self);

#endif  /* KSV_ARGUMENT_H */
