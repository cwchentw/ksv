#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "cstring.h"
#include "lexer.h"
#include "print.h"
#include "token.h"

struct ksv_lexer_t {
    char *delimeter;
    char *end_of_line;
    char quote;
    size_t size;
    size_t capacity;
    size_t index;
    ksv_token_t **tokens;
};

ksv_lexer_t * ksv_lexer_new(char *delimeter, char *end_of_line, char quote)
{
    ksv_lexer_t *lexer = (ksv_lexer_t *) malloc(sizeof(ksv_lexer_t));
    if (!lexer) {
        PUTERR("Failed to allocate memory for csv lexer");
        PUTERR("Check available system memory");
        return lexer;
    }

    lexer->delimeter = delimeter;
    lexer->end_of_line = end_of_line;
    lexer->quote = quote;

    lexer->size = 0;
    lexer->capacity = 16;
    lexer->index = 0;

    lexer->tokens = \
        (ksv_token_t **) malloc(lexer->capacity * sizeof(ksv_token_t *));
    if (!(lexer->tokens)) {
        PUTERR("Failed to allocate memory for tokens of csv lexer");
        PUTERR("Check available system memory");
        free(lexer);
        return NULL;
    }

    return lexer;
}

static BOOL ksv_lexer_push(ksv_lexer_t *self, ksv_token_t *token);

BOOL ksv_lexer_lex(ksv_lexer_t *self, char *input)
{
    assert(self);

    {
        size_t i;
        size_t j = 0;
        for (i = 0; i < strlen(input); i++) {
            BOOL is_string = FALSE;

            if ('\n' == input[i]) {
                if (0 == strcmp("\n", self->delimeter)) {
                    char *eol = string_allocate("\n");
                    if (!eol)
                        return FALSE;
                    
                    ksv_token_t *token = \
                        ksv_token_new(KSV_TOKEN_END_OF_LINE, eol);
                    if (!token)
                        return FALSE;

                    if (!ksv_lexer_push(self, token))
                        return FALSE;
                }
                else {
                    goto LEX_STRING;
                }
            }
            else if ('\r' == input[i]) {
                if (0 == strcmp("\r\n", self->delimeter)) {
                    if (i+1 < strlen(input) && '\n' == input[i+1]) {
                        char *eol = string_allocate("\r\n");
                        if (!eol)
                            return FALSE;

                        ksv_token_t *token = \
                            ksv_token_new(KSV_TOKEN_END_OF_LINE, eol);
                        if (!token)
                            return FALSE;

                        if (!ksv_lexer_push(self, token))
                            return FALSE;

                        i += 1;
                    }
                }
                else {
                    goto LEX_STRING;
                }
            }
            else if (self->quote == input[i]) {
                char *quote = string_allocate_char(self->quote);
                if (!quote)
                    return FALSE;

                ksv_token_t *token = \
                    ksv_token_new(KSV_TOKEN_QUOTE, quote);
                if (!token)
                    return FALSE;

                if (!ksv_lexer_push(self, token))
                    return FALSE;
            }
            else {
        LEX_STRING:
                j = i;
                
                is_string = TRUE;
            }

            if (!is_string && j > i) {
                char *s = string_allocate_substring(input, i, j);
                if (!s)
                    return FALSE;

                ksv_token_t *token = \
                    ksv_token_new(KSV_TOKEN_STRING, s);
                if (!token)
                    return FALSE;

                if (!ksv_lexer_push(self, token))
                    return FALSE;
            }
        }
    }

    return TRUE;
}

static BOOL ksv_lexer_expand(ksv_lexer_t *self);

static BOOL ksv_lexer_push(ksv_lexer_t *self, ksv_token_t *token)
{
    assert(self);

    if (!ksv_lexer_expand(self))
        return FALSE;

    self->tokens[self->size] = token;
    self->size += 1;

    return TRUE;
}

static BOOL ksv_lexer_expand(ksv_lexer_t *self)
{
    assert(self);

    if (self->size < self->capacity)
        return TRUE;

    self->capacity <<= 1;
    ksv_token_t **old_tokens = self->tokens;
    ksv_token_t **new_tokens = \
        (ksv_token_t **) malloc(self->capacity * sizeof(ksv_token_t *));
    if (!new_tokens) {
        PUTERR("Failed to allocate tokens for csv lexer");
        PUTERR("Check available system memory");
        return FALSE;
    }

    {
        size_t i;
        for (i = 0; i < self->size; i++)
            new_tokens[i] = old_tokens[i];
    }

    {
        size_t i;
        for (i = self->size; i <= self->capacity; i++)
            new_tokens[i] = NULL;
    }

    self->tokens = new_tokens;
    free(old_tokens);

    return TRUE;
}

void ksv_lexer_delete(void *self)
{
    assert(self);

    size_t capacity = ((ksv_lexer_t *) self)->capacity;
    ksv_token_t **tokens = ((ksv_lexer_t *) self)->tokens;
    
    {
        size_t i;
        for (i = 0; i < capacity; i++) {
            if (tokens[i])
                ksv_token_delete(tokens[i]);
        }
    }

    free(self);
}