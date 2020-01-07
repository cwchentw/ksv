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

    free(self);
}

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
                    ksv_token_t *token_next = ksv_lexer_peek(lexer, 1);

                    if (token_next && KSV_TOKEN_QUOTE == ksv_token_type(token_next)) {
                        /* Consume escaped quote. */
                        if (!ksv_ast_add_token(ast, token))
                            goto ERROR_KSV_PARSER;

                        token = ksv_lexer_next(lexer);

                        /* Consume escaped quote. */
                        if (token) {
                            if (!ksv_ast_add_token(ast, token))
                                goto ERROR_KSV_PARSER;
                        }
                        
                        /* Keep consuming quoted string. */
                        token = ksv_lexer_next(lexer);
                    }
                    else {
                        /* Consume ending quote. */
                        if (token) {
                            if (!ksv_ast_add_token(ast, token))
                                goto ERROR_KSV_PARSER;
                        }

                        token = ksv_lexer_next(lexer);  /* Token for next round. */
                        break;
                    }
                }
                else {
                    token = ksv_lexer_next(lexer);

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
            
            token = ksv_lexer_next(lexer);  /* Token for next round. */
        }
        else if (token && KSV_TOKEN_END_OF_LINE == ksv_token_type(token)) {
            ksv_ast_t *ast = ksv_ast_new(KSV_AST_EOL);
            if (!ast)
                goto ERROR_KSV_PARSER;

            if (!ksv_ast_add_token(ast, token))
                goto ERROR_KSV_PARSER;

            token = ksv_lexer_next(lexer);  /* Token for next round. */
        }
        else if (token && KSV_TOKEN_STRING == ksv_token_type(token)) {
            ksv_ast_t *ast = ksv_ast_new(KSV_AST_FIELD);
            if (!ast)
                goto ERROR_KSV_PARSER;

            if (!ksv_ast_add_token(ast, token))
                goto ERROR_KSV_PARSER;

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
