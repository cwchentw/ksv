#include <assert.h>
#include <stdlib.h>
#include "ksv_ast.h"
#include "ksv_parser.h"
#include "ksv_token.h"
#include "print.h"

struct ksv_parser_t {
    size_t size;
    size_t capacity;
    size_t index;
    ksv_ast_t **asts;
};

ksv_parser_t * ksv_parser_new(void)
{
    ksv_parser_t *parser = \
        (ksv_parser_t *) malloc(sizeof(ksv_parser_t));
    if (!parser) {
    #if DEBUG
        PUTERR("Failed to allocate memory for ksv parser");
        PUTERR("Check available system memory");
    #endif
        return parser;
    }

    parser->size = 0;
    parser->capacity = 16;
    parser->index = 0;

    parser->asts = \
        (ksv_ast_t **) malloc(parser->capacity * sizeof(ksv_ast_t *));
    if (!(parser->asts)) {
    #if DEBUG
        PUTERR("Failed to allocate memory for the asts of ksv parser");
        PUTERR("Check available system memory");
    #endif
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

static KSV_STATUS ksv_parser_push(ksv_parser_t *self, ksv_ast_t *ast);

KSV_STATUS ksv_parser_parse(ksv_parser_t *self, ksv_lexer_t *lexer)
{
    assert(self);
    assert(lexer);

    ksv_token_t *token = ksv_lexer_next(lexer);
    while (token) {
        if (token && KSV_TOKEN_QUOTE == ksv_token_type(token)) {
            ksv_ast_t *ast = ksv_ast_new(KSV_AST_FIELD);
            if (!ast)
                return KSV_NO_MEMORY;

            /* Drop the token. */
            ksv_token_delete(token);

            BOOL is_paired = FALSE;

            /* Consume next token. */
            token = ksv_lexer_next(lexer);
            while (token) {
                if (token && KSV_TOKEN_QUOTE == ksv_token_type(token)) {
                    /* Drop the token. */
                    ksv_token_delete(token);

                    /* Consume next token. */
                    token = ksv_lexer_next(lexer);

                    if (token && KSV_TOKEN_QUOTE == ksv_token_type(token)) {
                        /* Drop the token. */
                        ksv_token_delete(token);

                        /* Keep consuming quoted string. */
                        token = ksv_lexer_next(lexer);
                    }
                    else {
                        is_paired = TRUE;

                        KSV_STATUS s = ksv_parser_push(self, ast);
                        if (KSV_SUCCESS != s)
                            return s;

                    #if DEBUG
                        char *out = ksv_ast_string(ast);
                        if (!ast)
                            return KSV_NO_MEMORY;

                        PUTERR("Parse quoted field: -->%s<--", out);

                        free(out);
                    #endif

                        break;
                    }
                }
                else {
                    /* Consume a token greedily. */
                    if (token) {
                        KSV_STATUS s = ksv_ast_add_token(ast, token);
                        if (KSV_SUCCESS != s)
                            return s;
                    }

                    /* Keep consuming quoted string. */
                    token = ksv_lexer_next(lexer);
                }
            }

            if (!is_paired) {
            #if DEBUG
                PUTERR("Parse unpaired quote");
            #endif
                return KSV_UNPAIRED_QUOTE;
            }
        }
        else if (token && KSV_TOKEN_DELIMETER == ksv_token_type(token)) {
            ksv_ast_t *ast = ksv_ast_new(KSV_AST_DELIMITER);
            if (!ast)
                return KSV_NO_MEMORY;

            KSV_STATUS s = ksv_ast_add_token(ast, token);
            if (KSV_SUCCESS != s)
                return s;

            s = ksv_parser_push(self, ast);
            if (KSV_SUCCESS != s)
                return s;

        #if DEBUG
            char *out = ksv_ast_string(ast);
            if (!out)
                return KSV_NO_MEMORY;

            PUTERR("Parse delimiter: -->%s<--", out);

            free(out);
        #endif

            token = ksv_lexer_next(lexer);  /* Token for next round. */
        }
        else if (token && KSV_TOKEN_END_OF_LINE == ksv_token_type(token)) {
            ksv_ast_t *ast = ksv_ast_new(KSV_AST_EOL);
            if (!ast)
                return KSV_NO_MEMORY;

            KSV_STATUS s = ksv_ast_add_token(ast, token);
            if (KSV_SUCCESS != s)
                return s;

            s = ksv_parser_push(self, ast);
            if (KSV_SUCCESS != s)
                return s;

        #if DEBUG
            char *out = ksv_ast_string(ast);
            if (!out)
                return KSV_NO_MEMORY;

            PUTERR("Parse EOL: -->%s<--", out);

            free(out);
        #endif


            token = ksv_lexer_next(lexer);  /* Token for next round. */
        }
        else if (token && KSV_TOKEN_STRING == ksv_token_type(token)) {
            ksv_ast_t *ast = ksv_ast_new(KSV_AST_FIELD);
            if (!ast)
                return KSV_NO_MEMORY;

            KSV_STATUS s = ksv_ast_add_token(ast, token);
            if (KSV_SUCCESS != s)
                return s;

            s = ksv_parser_push(self, ast);
            if (KSV_SUCCESS != s)
                return s;

        #if DEBUG
            char *out = ksv_ast_string(ast);
            if (!out)
                return KSV_NO_MEMORY;

            PUTERR("Parse field: -->%s<--", out);

            free(out);
        #endif

            token = ksv_lexer_next(lexer);  /* Token for next round. */
        }
        else {
        #if DEBUG
            PUTERR("Unknown token");
        #endif

            return KSV_ERROR_PARSING;
        }
    }

    return KSV_SUCCESS;
}

static KSV_STATUS ksv_parser_expand(ksv_parser_t *self);

static KSV_STATUS ksv_parser_push(ksv_parser_t *self, ksv_ast_t *ast)
{
    assert(self);
    assert(ast);

    KSV_STATUS s = ksv_parser_expand(self);
    if (KSV_SUCCESS != s)
        return s;

    self->asts[self->size] = ast;
    self->size += 1;

    return KSV_SUCCESS;
}

static KSV_STATUS ksv_parser_expand(ksv_parser_t *self)
{
    assert(self);

    if (self->size + 1 <= self->capacity)
        return KSV_SUCCESS;

    self->capacity <<= 1;
    ksv_ast_t **old_asts = self->asts;
    ksv_ast_t **new_asts = \
        (ksv_ast_t **) malloc(self->capacity * sizeof(ksv_ast_t *));
    if (!(new_asts)) {
    #if DEBUG
        PUTERR("Failed to allocate memory for the asts of ksv parser");
        PUTERR("Check available system memory");
    #endif
        return KSV_NO_MEMORY;
    }

    {
        size_t i;
        for (i = 0; i < self->size; i++)
            new_asts[i] = old_asts[i];
    }

    {
        size_t i;
        for (i = self->size; i < self->capacity; i++)
            new_asts[i] = NULL;
    }

    self->asts = new_asts;
    free(old_asts);

    return KSV_SUCCESS;
}

ksv_ast_t * ksv_parser_next(ksv_parser_t *self)
{
    assert(self);

    if (self->index > self->size - 1)
        return NULL;

    ksv_ast_t *ast = self->asts[self->index];
    self->index += 1;

    return ast;
}
