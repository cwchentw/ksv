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
    size_t capacity;
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
    csv->capacity = 16;

    csv->header = NULL;
    csv->rows = (char **) malloc(csv->capacity * sizeof(char *));
    if (!(csv->rows)) {
        PUTERR("Failed to allocate rows for csv object");
        PUTERR("Check available system memory");
        ksv_delete(csv);
        return NULL;
    }

    {
        size_t i;
        for (i = 0; i < csv->capacity; i++)
            csv->rows[i] = NULL;
    }

    csv->delimeter = delimeter;
    csv->end_of_line = end_of_line;
    csv->quote = quote;

    return csv;
}

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
                /* Parse csv header. */
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

        free(header);
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
