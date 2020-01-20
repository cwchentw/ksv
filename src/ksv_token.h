#ifndef KSV_TOKEN_H
#define KSV_TOKEN_H

#include "ksv.h"

typedef struct ksv_token_t ksv_token_t;

typedef unsigned char KSV_TOKEN_TYPE;

#define  KSV_TOKEN_STRING       0
#define  KSV_TOKEN_DELIMETER    1
#define  KSV_TOKEN_END_OF_LINE  2
#define  KSV_TOKEN_QUOTE        3

KSV_PRIVATE ksv_token_t * ksv_token_new(KSV_TOKEN_TYPE type, char *string);
KSV_PRIVATE void ksv_token_delete(void *self);
KSV_PRIVATE KSV_TOKEN_TYPE ksv_token_type(ksv_token_t *self);
KSV_PRIVATE char * ksv_token_string(ksv_token_t *self);

#endif  /* KSV_TOKEN_H */
