#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "cstring.h"
#include "ksv_lexer.h"
#include "ksv_token.h"
#include "print.h"

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
    #if DEBUG
        PUTERR("Failed to allocate memory for csv lexer");
        PUTERR("Check available system memory");
    #endif
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
    #if DEBUG
        PUTERR("Failed to allocate memory for tokens of csv lexer");
        PUTERR("Check available system memory");
    #endif
        free(lexer);
        return NULL;
    }

    {
        size_t i;
        for (i = 0; i < lexer->capacity; i++)
            lexer->tokens[i] = NULL;
    }

    return lexer;
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

    free(tokens);
    free(self);
}

static KSV_STATUS ksv_lexer_push(ksv_lexer_t *self, ksv_token_t *token);

KSV_STATUS ksv_lexer_lex(ksv_lexer_t *self, char *input)
{
    assert(self);

#if DEBUG
    PUTERR("Source to scan: -->%s<--", input);
#endif

    {
        size_t i;
        size_t j;
        for (i = 0; i < strlen(input); i++) {
            if ('\n' == input[i]) {
                if (0 == strcmp("\n", self->end_of_line)) {
                    char *eol = string_allocate("\n");
                    if (!eol)
                        return KSV_NO_MEMORY;

                #if DEBUG
                    PUTERR("EOL as token: -->%s<--", eol);
                #endif

                    ksv_token_t *token = \
                        ksv_token_new(KSV_TOKEN_END_OF_LINE, eol);
                    if (!token)
                        return KSV_NO_MEMORY;

                    KSV_STATUS s = ksv_lexer_push(self, token);
                    if (KSV_SUCCESS != s)
                        return s;
                }
                else {
                    goto LEX_STRING;
                }
            }
            else if ('\r' == input[i]) {
                if (0 == strcmp("\r\n", self->end_of_line)) {
                    if (i+1 < strlen(input) && '\n' == input[i+1]) {
                        char *eol = string_allocate("\r\n");
                        if (!eol)
                            return KSV_NO_MEMORY;

                    #if DEBUG
                        PUTERR("EOL as token: -->%s<--", eol);
                    #endif

                        ksv_token_t *token = \
                            ksv_token_new(KSV_TOKEN_END_OF_LINE, eol);
                        if (!token)
                            return KSV_NO_MEMORY;

                        KSV_STATUS s = ksv_lexer_push(self, token);
                        if (KSV_SUCCESS != s)
                            return s;

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
                    return KSV_NO_MEMORY;

            #if DEBUG
                PUTERR("Quote as token: -->%s<--", quote);
            #endif

                ksv_token_t *token = \
                    ksv_token_new(KSV_TOKEN_QUOTE, quote);
                if (!token)
                    return KSV_NO_MEMORY;

                KSV_STATUS s = ksv_lexer_push(self, token);
                if (KSV_SUCCESS != s)
                    return s;
            }
            else if (self->delimeter[0] == input[i]) {
                for (j = 0; j < strlen(self->delimeter) && i+j < strlen(input); j++) {
                    if (self->delimeter[j] != input[i+j])
                        goto LEX_STRING;
                }

                char *delim = string_allocate(self->delimeter);
                if (!delim)
                    return KSV_NO_MEMORY;

            #if DEBUG
                PUTERR("Delimeter as token: -->%s<--", delim);
            #endif

                ksv_token_t *token = \
                    ksv_token_new(KSV_TOKEN_DELIMETER, delim);
                if (!token)
                    return KSV_NO_MEMORY;

                KSV_STATUS s = ksv_lexer_push(self, token);
                if (KSV_SUCCESS != s)
                    return s;

                i += j - 1;
            }
            else {
            LEX_STRING:
                for (j = i; j < strlen(input); j++) {
                    if (self->quote == input[j])
                        break;

                    if (0 == strcmp("\n", self->end_of_line) && '\n' == input[j])
                        break;

                    if (0 == strcmp("\r\n", self->end_of_line)
                        && '\r' == input[j]
                        && j+1 < strlen(input)
                        && '\n' == input[j+1])
                        break;

                    size_t k;
                    BOOL is_delim = TRUE;
                    for (k = 0; k < strlen(self->delimeter) && j+k < strlen(input); k++) {
                        if (self->delimeter[k] != input[j+k]) {
                            is_delim = FALSE;
                            break;
                        }
                    }

                    if (is_delim) {
                        j += k - 1;
                        break;
                    }
                }

                char *s = string_allocate_substring(input, i, j - 1);
                if (!s)
                    return KSV_NO_MEMORY;

            #if DEBUG
                PUTERR("String as token: -->%s<--", s);
            #endif

                ksv_token_t *token = \
                    ksv_token_new(KSV_TOKEN_STRING, s);
                if (!token)
                    return KSV_NO_MEMORY;

                KSV_STATUS status = ksv_lexer_push(self, token);
                if (KSV_SUCCESS != status)
                    return status;

                i = j - 1;
            }
        }
    }

    return KSV_SUCCESS;
}

static KSV_STATUS ksv_lexer_expand(ksv_lexer_t *self);

static KSV_STATUS ksv_lexer_push(ksv_lexer_t *self, ksv_token_t *token)
{
    assert(self);
    assert(token);

    if (KSV_SUCCESS != ksv_lexer_expand(self))
        return KSV_NO_MEMORY;

    self->tokens[self->size] = token;
    self->size += 1;

    return KSV_SUCCESS;
}

static KSV_STATUS ksv_lexer_expand(ksv_lexer_t *self)
{
    assert(self);

    if (self->size + 1 <= self->capacity)
        return KSV_SUCCESS;

    self->capacity <<= 1;
    ksv_token_t **old_tokens = self->tokens;
    ksv_token_t **new_tokens = \
        (ksv_token_t **) malloc(self->capacity * sizeof(ksv_token_t *));
    if (!new_tokens) {
    #if DEBUG
        PUTERR("Failed to allocate tokens for csv lexer");
        PUTERR("Check available system memory");
    #endif
        return KSV_NO_MEMORY;
    }

    {
        size_t i;
        for (i = 0; i < self->size; i++)
            new_tokens[i] = old_tokens[i];
    }

    {
        size_t i;
        for (i = self->size; i < self->capacity; i++)
            new_tokens[i] = NULL;
    }

    self->tokens = new_tokens;
    free(old_tokens);

    return KSV_SUCCESS;
}

void ksv_lexer_start(ksv_lexer_t *self)
{
    assert(self);

    self->index = 0;
}

ksv_token_t * ksv_lexer_next(ksv_lexer_t *self)
{
    assert(self);

    if (self->index >= self->size)
        return NULL;

    ksv_token_t *token = self->tokens[self->index];
    self->index += 1;

    ksv_token_t *copied = ksv_token_new(
        ksv_token_type(token),
        string_allocate(ksv_token_string(token))
    );

    return copied;
}

ksv_token_t * ksv_lexer_peek(ksv_lexer_t *self, size_t n)
{
    assert(self);

    if (self->index + n >= self->size)
        return NULL;

    return self->tokens[self->index + n];
}
