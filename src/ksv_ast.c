#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "cstring.h"
#include "ksv.h"
#include "ksv_ast.h"
#include "ksv_token.h"
#include "print.h"

typedef struct ksv_ast_field_t ksv_ast_field_t;
typedef struct ksv_ast_delimiter_t ksv_ast_delimiter_t;
typedef struct ksv_ast_eol_t ksv_ast_eol_t;

struct ksv_ast_t {
    KSV_AST_TYPE type;
    union {
        ksv_ast_field_t *field;
        ksv_ast_delimiter_t *delimiter;
        ksv_ast_eol_t *eol;
    } ast;
};

static BOOL is_valid_ast_type(KSV_AST_TYPE type);
static ksv_ast_field_t * ksv_ast_field_new(void);
static ksv_ast_delimiter_t * ksv_ast_delimiter_new(void);
static ksv_ast_eol_t * ksv_ast_eol_new(void);

ksv_ast_t * ksv_ast_new(KSV_AST_TYPE type)
{
    assert(is_valid_ast_type(type));

    ksv_ast_t *ast = \
        (ksv_ast_t *) malloc(sizeof(ksv_ast_t));
    if (!ast) {
        PUTERR("Failed to allocate memory for ksv ast");
        PUTERR("Check available system memory");
        return ast;
    }

    ast->type = type;

    switch (ast->type) {
    case KSV_AST_FIELD:
        ast->ast.field = ksv_ast_field_new();
        if (!(ast->ast.field))
            goto ERROR_AST;
        break;
    case KSV_AST_DELIMITER:
        ast->ast.delimiter = ksv_ast_delimiter_new();
        if (!(ast->ast.delimiter))
            goto ERROR_AST;
        break;
    case KSV_AST_EOL:
        ast->ast.eol = ksv_ast_eol_new();
        if (!(ast->ast.eol))
            goto ERROR_AST;
        break;
    }

    return ast;

ERROR_AST:
    ksv_ast_delete(ast);

    return NULL;
}

static BOOL is_valid_ast_type(KSV_AST_TYPE type)
{
    return KSV_AST_FIELD == type
        || KSV_AST_DELIMITER == type
        || KSV_AST_EOL == type;
}

struct ksv_ast_field_t {
    size_t size;
    size_t capacity;
    ksv_token_t **tokens;
};

static ksv_ast_field_t * ksv_ast_field_new(void)
{
    ksv_ast_field_t *ast = \
        (ksv_ast_field_t *) malloc(sizeof(ksv_ast_field_t));
    if (!ast) {
        PUTERR("Failed to allocate memory for ksv field ast");
        PUTERR("Check available system memory");
        return ast;
    }

    ast->size = 0;
    ast->capacity = 4;  /* Sensible initial capacity. */

    ast->tokens = \
        (ksv_token_t **) malloc(ast->capacity * sizeof(ksv_token_t *));
    if (!(ast->tokens)) {
        PUTERR("Failed to allocate memory for the tokens of ksv field ast");
        PUTERR("Check available system memory");
        free(ast);
        return NULL;
    }

    {
        size_t i;
        for (i = 0; i < ast->capacity; i++)
            ast->tokens[i] = NULL;
    }

    return ast;
}

struct ksv_ast_delimiter_t {
    size_t size;
    ksv_token_t *token;
};

static ksv_ast_delimiter_t * ksv_ast_delimiter_new(void)
{
    ksv_ast_delimiter_t *ast = \
        (ksv_ast_delimiter_t *) malloc(sizeof(ksv_ast_delimiter_t));
    if (!ast) {
        PUTERR("Failed to allocate memory for ksv delimiter ast");
        PUTERR("Check available system memory");
        return ast;
    }

    ast->size = 0;
    ast->token = NULL;

    return ast;
}

struct ksv_ast_eol_t {
    size_t size;
    ksv_token_t *token;
};

static ksv_ast_eol_t * ksv_ast_eol_new(void)
{
    ksv_ast_eol_t *ast = (ksv_ast_eol_t *) malloc(sizeof(ksv_ast_eol_t));
    if (!ast) {
        PUTERR("Failed to allocate memory for ksv eol ast");
        PUTERR("Check available system memory");
        return ast;
    }

    ast->size = 0;
    ast->token = NULL;

    return ast;
}

static void ksv_ast_field_delete(void *self);
static void ksv_ast_delimiter_delete(void *self);
static void ksv_ast_eol_delete(void *self);

void ksv_ast_delete(void *self)
{
    assert(self);

    KSV_AST_TYPE type = ((ksv_ast_t *) self)->type;

    switch (type) {
    case KSV_AST_FIELD:
        ksv_ast_field_delete(
            (void *) ((ksv_ast_t *) self)->ast.field);
        break;
    case KSV_AST_DELIMITER:
        ksv_ast_delimiter_delete(
            (void *) ((ksv_ast_t *) self)->ast.delimiter);
        break;
    case KSV_AST_EOL:
        ksv_ast_eol_delete(
            (void *) ((ksv_ast_t *) self)->ast.eol);
        break;
    }

    free(self);
}

static void ksv_ast_field_delete(void *self)
{
    assert(self);

    size_t capacity = ((ksv_ast_field_t *) self)->capacity;
    ksv_token_t **tokens = ((ksv_ast_field_t *) self)->tokens;
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

static void ksv_ast_delimiter_delete(void *self)
{
    assert(self);

    ksv_token_t *token = ((ksv_ast_delimiter_t *) self)->token;
    if (token)
        ksv_token_delete(token);

    free(self);
}

static void ksv_ast_eol_delete(void *self)
{
    assert(self);

    ksv_token_t *token = ((ksv_ast_eol_t *) self)->token;
    if (token)
        ksv_token_delete(token);

    free(self);
}

static KSV_STATUS ksv_ast_field_add_token(ksv_ast_field_t *self, ksv_token_t *token);
static KSV_STATUS ksv_ast_delimiter_add_token(ksv_ast_delimiter_t *self, ksv_token_t *token);
static KSV_STATUS ksv_ast_eol_add_token(ksv_ast_eol_t *self, ksv_token_t *token);

KSV_STATUS ksv_ast_add_token(ksv_ast_t *self, ksv_token_t *token)
{
    assert(self);
    assert(token);

    KSV_STATUS status;
    switch (self->type) {
    case KSV_AST_FIELD:
        status = ksv_ast_field_add_token(self->ast.field, token);
        if (KSV_SUCCESS != status)
            return status;
        break;
    case KSV_AST_DELIMITER:
        status = ksv_ast_delimiter_add_token(self->ast.delimiter, token);
        if (KSV_SUCCESS != status)
            return status;
        break;
    case KSV_AST_EOL:
        status = ksv_ast_eol_add_token(self->ast.eol, token);
        if (KSV_SUCCESS != status)
            return status;
        break;
    default:
        assert(0 && "Unknown ksv ast");
    }

    return KSV_SUCCESS;
}

static KSV_STATUS ksv_ast_field_expand(ksv_ast_field_t *self);

static KSV_STATUS ksv_ast_field_add_token(ksv_ast_field_t *self, ksv_token_t *token)
{
    assert(self);
    assert(token);

    KSV_STATUS s = ksv_ast_field_expand(self);
    if (KSV_SUCCESS != s)
        return s;

    self->tokens[self->size] = token;
    self->size += 1;

    return KSV_SUCCESS;
}

static KSV_STATUS ksv_ast_field_expand(ksv_ast_field_t *self)
{
    assert(self);

    if (self->size + 1 <= self->capacity)
        return KSV_SUCCESS;

    self->capacity <<= 1;
    ksv_token_t **old_tokens = self->tokens;
    ksv_token_t **new_tokens = \
        (ksv_token_t **) malloc(self->capacity * sizeof(ksv_token_t *));
    if (!new_tokens) {
        PUTERR("Failed to allocate the tokens of ksv field ast");
        PUTERR("Check available system memory");
        return KSV_NO_MEMORY;
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

    return KSV_SUCCESS;
}

static KSV_STATUS ksv_ast_delimiter_add_token(ksv_ast_delimiter_t *self, ksv_token_t *token)
{
    assert(self);
    assert(token);

    if (self->size > 0)
        return KSV_ERROR_PARSING;

    self->token = token;
    self->size += 1;

    return KSV_SUCCESS;
}

static KSV_STATUS ksv_ast_eol_add_token(ksv_ast_eol_t *self, ksv_token_t *token)
{
    assert(self);
    assert(token);

    if (self->size > 0)
        return KSV_ERROR_PARSING;

    self->token = token;
    self->size += 1;

    return KSV_SUCCESS;
}

static char * ksv_ast_field_string(ksv_ast_field_t *self);
static char * ksv_ast_delimiter_string(ksv_ast_delimiter_t *self);
static char * ksv_ast_eol_string(ksv_ast_eol_t *self);

char * ksv_ast_string(ksv_ast_t *self)
{
    assert(self);

    char *out = NULL;

    switch(self->type) {
    case KSV_AST_FIELD:
        out = ksv_ast_field_string(self->ast.field);
        break;
    case KSV_AST_DELIMITER:
        out = ksv_ast_delimiter_string(self->ast.delimiter);
        break;
    case KSV_AST_EOL:
        out = ksv_ast_eol_string(self->ast.eol);
        break;
    }

    return out;
}

static char * ksv_ast_field_string(ksv_ast_field_t *self)
{
    assert(self);

    char *out = NULL;
    size_t size = 16;  /* Sensible initial size. */

    out = (char *) malloc(size * sizeof(char));
    if (!out) {
        PUTERR("Failed to allocate memory for C string");
        PUTERR("Check available system memory");
        return out;
    }

    out[0] = '\0';

    size_t total_size = 0;

    {
        size_t i;
        for (i = 0; i < self->size; i++) {
            total_size += strlen(ksv_token_string(self->tokens[i]));

            while (total_size > size) {
                size <<= 1;
                if (!realloc(out, size)) {
                    PUTERR("Failed to reallocate memory for C string");
                    PUTERR("Check available system memory");
                    free(out);
                    return NULL;
                }
            }

        #if _MSC_VER
            strcat_s(out, size, ksv_token_string(self->tokens[i]));
        #else
            strcat(out, ksv_token_string(self->tokens[i]));
        #endif

            out[total_size] = '\0';
        }
    }

    return out;
}

static char * ksv_ast_delimiter_string(ksv_ast_delimiter_t *self)
{
    assert(self);

    return string_allocate(ksv_token_string(self->token));
}

KSV_AST_TYPE ksv_ast_type(ksv_ast_t *self)
{
    assert(self);

    return self->type;
}

static char * ksv_ast_eol_string(ksv_ast_eol_t *self)
{
    assert(self);

    return string_allocate(ksv_token_string(self->token));
}
