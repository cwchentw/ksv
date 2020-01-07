#include <assert.h>
#include <stdlib.h>
#include "ksv_ast.h"
#include "ksv_parser.h"
#include "ksv_token.h"
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

    free(asts);
    free(self);
}

static BOOL ksv_parser_push(ksv_parser_t *self, ksv_ast_t *ast);

BOOL ksv_parser_parse(ksv_parser_t *self, ksv_lexer_t *lexer)
{
    assert(self);
    assert(lexer);

    ksv_token_t *token = ksv_lexer_next(lexer);
    while (token) {
        if (token && KSV_TOKEN_QUOTE == ksv_token_type(token)) {
            ksv_ast_t *ast = ksv_ast_new(KSV_AST_FIELD);
            if (!ast)
                goto ERROR_KSV_PARSER;

            /* Consume starting quote. */
            if (!ksv_ast_add_token(ast, token))
                goto ERROR_KSV_PARSER;

            token = ksv_lexer_next(lexer);
            while (token) {
                if (token && KSV_TOKEN_QUOTE == ksv_token_type(token)) {
                    /* Consume the quote. */
                    if (token) {
                        if (!ksv_ast_add_token(ast, token))
                            goto ERROR_KSV_PARSER;
                    }

                    token = ksv_lexer_next(lexer);
                    if (token && KSV_TOKEN_QUOTE == ksv_token_type(token)) {
                        /* Consume escaped quote. */
                        if (token) {
                            if (!ksv_ast_add_token(ast, token))
                                goto ERROR_KSV_PARSER;
                        }

                        /* Keep consuming quoted string. */
                        token = ksv_lexer_next(lexer);
                    }
                    else {
                        if (!ksv_parser_push(self, ast))
                            goto ERROR_KSV_PARSER;

                    #if DEBUG
                        char *out = ksv_ast_string(ast);
                        if (!ast)
                            goto ERROR_KSV_PARSER;

                        PUTERR("Parse quoted field: -->%s<--", out);

                        free(out);
                    #endif

                        break;
                    }
                }
                else {
                    /* Consume a token greedily. */
                    if (token) {
                        if (!ksv_ast_add_token(ast, token))
                            goto ERROR_KSV_PARSER;
                    }

                    /* Keep consuming quoted string. */
                    token = ksv_lexer_next(lexer);
                }
            }
        }
        else if (token && KSV_TOKEN_DELIMETER == ksv_token_type(token)) {
            ksv_ast_t *ast = ksv_ast_new(KSV_AST_DELIMITER);
            if (!ast)
                goto ERROR_KSV_PARSER;

            if (!ksv_ast_add_token(ast, token))
                goto ERROR_KSV_PARSER;

            if (!ksv_parser_push(self, ast))
                goto ERROR_KSV_PARSER;

        #if DEBUG
            char *out = ksv_ast_string(ast);
            if (!out)
                goto ERROR_KSV_PARSER;

            PUTERR("Parse delimiter: -->%s<--", out);

            free(out);
        #endif

            token = ksv_lexer_next(lexer);  /* Token for next round. */
        }
        else if (token && KSV_TOKEN_END_OF_LINE == ksv_token_type(token)) {
            ksv_ast_t *ast = ksv_ast_new(KSV_AST_EOL);
            if (!ast)
                goto ERROR_KSV_PARSER;

            if (!ksv_ast_add_token(ast, token))
                goto ERROR_KSV_PARSER;

            if (!ksv_parser_push(self, ast))
                goto ERROR_KSV_PARSER;

        #if DEBUG
            char *out = ksv_ast_string(ast);
            if (!out)
                goto ERROR_KSV_PARSER;

            PUTERR("Parse EOL: -->%s<--", out);

            free(out);
        #endif


            token = ksv_lexer_next(lexer);  /* Token for next round. */
        }
        else if (token && KSV_TOKEN_STRING == ksv_token_type(token)) {
            ksv_ast_t *ast = ksv_ast_new(KSV_AST_FIELD);
            if (!ast)
                goto ERROR_KSV_PARSER;

            if (!ksv_ast_add_token(ast, token))
                goto ERROR_KSV_PARSER;

            if (!ksv_parser_push(self, ast))
                goto ERROR_KSV_PARSER;

        #if DEBUG
            char *out = ksv_ast_string(ast);
            if (!out)
                goto ERROR_KSV_PARSER;

            PUTERR("Parse field: -->%s<--", out);

            free(out);
        #endif

            token = ksv_lexer_next(lexer);  /* Token for next round. */
        }
        else {
            PUTERR("Unknown token");

            token = ksv_lexer_next(lexer);  /* Token for next round. */
        }
    }

    return TRUE;

ERROR_KSV_PARSER:
    return FALSE;
}

static BOOL ksv_parser_expand(ksv_parser_t *self);

static BOOL ksv_parser_push(ksv_parser_t *self, ksv_ast_t *ast)
{
    assert(self);
    assert(ast);

    if (!ksv_parser_expand(self))
        return FALSE;

    self->asts[self->size] = ast;
    self->size += 1;

    return TRUE;
}

static BOOL ksv_parser_expand(ksv_parser_t *self)
{
    assert(self);

    if (self->size + 1 <= self->capacity)
        return TRUE;

    self->capacity <<= 1;
    ksv_ast_t **old_asts = self->asts;
    ksv_ast_t **new_asts = \
        (ksv_ast_t **) malloc(self->capacity * sizeof(ksv_ast_t *));
    if (!(new_asts)) {
        PUTERR("Failed to allocate memory for the asts of ksv parser");
        PUTERR("Check available system memory");
        return FALSE;
    }

    {
        size_t i;
        for (i = 0; i < self->size; i++)
            new_asts[i] = old_asts[i];
    }

    {
        size_t i;
        for (i = self->size; i <= self->capacity; i++)
            new_asts[i] = NULL;
    }

    self->asts = new_asts;
    free(old_asts);

    return TRUE;
}
