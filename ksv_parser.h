#ifndef KSV_PARSER_H
#define KSV_PARSER_H

typedef struct ksv_parser_t ksv_parser_t;

ksv_parser_t * ksv_parser_new(void);
void ksv_parser_delete(void *self);

#endif  /* KSV_PARSER_H */