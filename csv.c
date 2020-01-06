#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "csv.h"
#include "lexer.h"
#include "print.h"

struct csv_t {
    size_t row;
    size_t col;
    size_t capacity;
    char **header;
    char ***rows;
    char *delimeter;
    char *end_of_line;
    char quote;
};

csv_t * csv_new_default(void)
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

    return csv_new(",", END_OF_LINE, '"');
}

csv_t * csv_new(char *delimeter, char *end_of_line, char quote)
{
    assert(0 != strcmp("", delimeter));
    assert(0 != strcmp("", end_of_line));
    assert(!quote);

    csv_t *csv = (csv_t *) malloc(sizeof(csv_t));
    if (!csv) {
        PUTERR("Failed to allocate csv object");
        PUTERR("Check available system memory");
        return csv;
    }

    csv->row = 0;
    csv->col = 0;
    csv->capacity = 16;

    csv->header = NULL;
    csv->rows = (char ***) malloc(csv->capacity * sizeof(char **));
    if (!(csv->rows)) {
        PUTERR("Failed to allocate rows for csv object");
        PUTERR("Check available system memory");
        csv_delete(csv);
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

BOOL csv_load_stream_with_header_strictly(csv_t *self, FILE *stream)
{
    assert(self);

    char *line = NULL;
    csv_lexer_t *lexer = NULL;

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
            lexer = csv_lexer_new(
                self->delimeter,
                self->end_of_line,
                self->quote
            );
            if (!lexer)
                goto ERROR_CSV;
            
            if (!csv_lexer_lex(lexer, line))
                goto ERROR_CSV;
    
            if (!visited) {
                /* Parse csv header. */
            }
            else {
                /* Parse csv row. */
            }
            
            csv_lexer_delete(lexer);
            lexer = NULL;
        }
    }

    return TRUE;

ERROR_CSV:
    if (lexer)
        csv_lexer_delete(lexer);

    if (line)
        free(line);

    return FALSE;
}

void csv_delete(void *self)
{
    assert(self);

    size_t row = ((csv_t *) self)->row;
    size_t col = ((csv_t *) self)->col;

    char **header = ((csv_t *) self)->header;
    if (header) {
        size_t i;
        for (i = 0; i < col; i++) {
            if (header[i])
                free((void *) header[i]);
        }
    }

    char ***rows = ((csv_t *) self)->rows;
    if (rows) {
        size_t i;
        for (i = 0; i < row; i++) {
            if (rows[i]) {
                size_t j;
                for (j = 0; j < col; j++) {
                    if (rows[i][j])
                        free((void *) rows[i][j]);
                }
                free((void *) rows[i]);
            }
        }
    }

    free(self);
}
