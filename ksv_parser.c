#include <assert.h>
#include <stdlib.h>
#include "ksv_ast.h"
#include "ksv_parser.h"
#include "print.h"

struct ksv_parser_t {
    size_t size;
    size_t capacity;
    ksv_ast_t **asts;
};

ksv_parser_t * ksv_parser_new(void)
{
    ksv_parser_t *parser = \
        (ksv_parser_t *) malloc(sizeof(ksv_parser_t));
    if (!parser) {
        PUTERR("Failed to allocate memory for ksv parser");
        PUTERR("Check available system memory");
        return parser;
    }

    parser->size = 0;
    parser->capacity = 16;

    parser->asts = \
        (ksv_ast_t **) malloc(parser->capacity * sizeof(ksv_ast_t *));
    if (!(parser->asts)) {
        PUTERR("Failed to allocate memory for the asts of ksv parser");
        PUTERR("Check available system memory");
        free(parser);
        return NULL;
    }

    {
        size_t i;
        for (i = 0; i < parser->capacity; i++)
            parser->asts[i] = NULL;
    }

    return parser;
}

void ksv_parser_delete(void *self)
{
    assert(self);

    size_t capacity = ((ksv_parser_t *) self)->capacity;
    ksv_ast_t **asts = ((ksv_parser_t *) self)->asts;

    {
        size_t i;
        for (i = 0; i < capacity; i++) {
            if (asts[i])
                ksv_ast_delete(asts[i]);
        }
    }

    free(self);
}
