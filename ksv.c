#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "ksv.h"
#include "ksv_lexer.h"
#include "ksv_parser.h"
#include "print.h"

struct ksv_t {
    size_t row;
    size_t col;
    size_t size_header;
    size_t capacity_header;
    size_t size_rows;
    size_t capacity_rows;
    char **header;
    char **rows;
    char *delimeter;
    char *end_of_line;
    char quote;
};

ksv_t * ksv_new_default(void)
{
#ifndef END_OF_LINE
    #ifdef _WIN32
        #define END_OF_LINE "\r\n"
    #elif __unix__ || __APPLE__
        #define END_OF_LINE "\n"
    #else
        #error "Unsupported platform"
    #endif
#endif  /* END_OF_LINE */

    return ksv_new(",", END_OF_LINE, '"');
}

ksv_t * ksv_new(char *delimeter, char *end_of_line, char quote)
{
    assert(0 != strcmp("", delimeter));
    assert(0 != strcmp("", end_of_line));
    assert(quote);

    ksv_t *csv = (ksv_t *) malloc(sizeof(ksv_t));
    if (!csv) {
        PUTERR("Failed to allocate csv object");
        PUTERR("Check available system memory");
        return csv;
    }

    csv->row = 0;
    csv->col = 0;
    csv->size_header = 0;
    csv->capacity_header = 16;
    csv->size_rows = 0;
    csv->capacity_rows = 64;

    csv->header = NULL;
    csv->rows = (char **) malloc(csv->capacity_rows * sizeof(char *));
    if (!(csv->rows)) {
        PUTERR("Failed to allocate rows for csv object");
        PUTERR("Check available system memory");
        ksv_delete(csv);
        return NULL;
    }

    {
        size_t i;
        for (i = 0; i < csv->capacity_rows; i++)
            csv->rows[i] = NULL;
    }

    csv->delimeter = delimeter;
    csv->end_of_line = end_of_line;
    csv->quote = quote;

    return csv;
}

static BOOL ksv_header_push(ksv_t *self, char *field);

BOOL ksv_load_stream_with_header_strictly(ksv_t *self, FILE *stream)
{
    assert(self);

    char *line = NULL;
    ksv_lexer_t *lexer = NULL;
    ksv_parser_t *parser = NULL;

    size_t line_width = 150;  /* Sensible initial line width. */
    line = (char *) malloc(line_width * sizeof(char));
    if (!line) {
        PUTERR("Failed to allocate memory for C string");
        PUTERR("Check available system memory");
        return FALSE;
    }

    BOOL visited = FALSE;
    while (fgets(line, line_width, stream)) {
        if (line_width == strlen(line)) {
            if ('\n' != line[line_width-1]) {
                line_width <<= 1;
                if (!realloc(line, line_width)) {
                    PUTERR("Failed to realloc memory for C string");
                    PUTERR("Check available system memory");
                    return FALSE;
                }
            }
            else {
                goto LOAD_LINE;
            }
        }
        else {
    LOAD_LINE:
            lexer = ksv_lexer_new(
                self->delimeter,
                self->end_of_line,
                self->quote
            );
            if (!lexer)
                goto ERROR_CSV;

            parser = ksv_parser_new();
            if (!parser)
                goto ERROR_CSV;

            if (!ksv_lexer_lex(lexer, line))
                goto ERROR_CSV;

            if (!ksv_parser_parse(parser, lexer))
                goto ERROR_CSV;

            if (!visited) {
                self->header = \
                    (char **) malloc(self->capacity_header * sizeof(char *));
                if (!(self->header)) {
                    PUTERR("Failed to allocate header for csv object");
                    PUTERR("Check available system memory");
                    goto ERROR_CSV;
                }

                {
                    size_t i;
                    for (i = 0; i < self->capacity_header; ++i)
                        self->header[i] = NULL;
                }

                ksv_ast_t *ast = ksv_parser_next(parser);
                char *field = NULL;
                while (ast) {
                    switch (ksv_ast_type(ast)) {
                    case KSV_AST_FIELD:
                        field = ksv_ast_string(ast);
                        if (!field)
                            goto ERROR_CSV;

                        if (!ksv_header_push(self, field))
                            goto ERROR_CSV;

                        self->col += 1;
                        break;
                    case KSV_AST_DELIMITER:
                        break;
                    case KSV_AST_EOL:
                        goto END_CSV_HEADER;
                    default:
                        assert(0 && "Unknown ksv ast");
                    }

                    ast = ksv_parser_next(parser);
                }
            END_CSV_HEADER:
                visited = TRUE;
            }
            else {
                /* Parse csv row. */
            }

            ksv_lexer_delete(lexer);
            lexer = NULL;

            ksv_parser_delete(parser);
            parser = NULL;
        }
    }

    free(line);

    return TRUE;

ERROR_CSV:
    if (parser)
        ksv_parser_delete(parser);

    if (lexer)
        ksv_lexer_delete(lexer);

    if (line)
        free(line);

    return FALSE;
}

static BOOL ksv_header_expand(ksv_t *self);

static BOOL ksv_header_push(ksv_t *self, char *field)
{
    assert(self);

    if (!ksv_header_expand(self))
        return FALSE;

    self->header[self->size_header] = field;
    self->size_header += 1;

    return TRUE;
}

static BOOL ksv_header_expand(ksv_t *self)
{
    assert(self);

    if (self->size_header < self->capacity_header)
        return TRUE;

    self->capacity_header <<= 1;
    char **old_header = self->header;
    char **new_header = \
        (char **) malloc(self->capacity_header * sizeof(char *));
    if (!new_header) {
        PUTERR("Failed to reallocate memory for header of csv object");
        PUTERR("Check available system memory");
        return FALSE;
    }

    {
        size_t i;
        for (i = 0; i < self->size_header; i++)
            new_header[i] = old_header[i];
    }

    {
        size_t i;
        for (i = self->size_header; i < self->capacity_header; i++)
            new_header[i] = NULL;
    }

    self->header = new_header;
    free(old_header);

    return TRUE;
}

void ksv_delete(void *self)
{
    assert(self);

    size_t row = ((ksv_t *) self)->row;
    size_t col = ((ksv_t *) self)->col;

    char **header = ((ksv_t *) self)->header;
    if (header) {
        size_t i;
        for (i = 0; i < col; i++) {
            if (header[i])
                free((void *) header[i]);
        }

        free((void *) header);
    }

    char **rows = ((ksv_t *) self)->rows;
    if (rows) {
        size_t i;
        for (i = 0; i < row; i++) {
            if (rows[i]) {
                free((void *) rows[i]);
            }
        }

        free((void *) rows);
    }

    free(self);
}
