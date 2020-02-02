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
    size_t index_header;
    size_t size_rows;
    size_t capacity_rows;
    size_t index_col;
    size_t index_row;
    size_t index;
    char **header;
    char **rows;
    char *delimeter;
    char *end_of_line;
    char *quote;
};

#if _MSC_VER
BOOL WINAPI DllMain (
    HINSTANCE const instance,  // handle to DLL module
    DWORD     const reason,    // reason for calling function
    LPVOID    const reserved)  // reserved
{
    // Perform actions based on the reason for calling.
    switch (reason) {
    case DLL_PROCESS_ATTACH:
        // Initialize once for each new process.
        // Return FALSE to fail DLL load.
        break;

    case DLL_THREAD_ATTACH:
        // Do thread-specific initialization.
        break;

    case DLL_THREAD_DETACH:
        // Do thread-specific cleanup.
        break;

    case DLL_PROCESS_DETACH:
        // Perform any necessary cleanup.
        break;
    }

    return TRUE;  // Successful DLL_PROCESS_ATTACH.
}
#endif

ksv_t * ksv_new_default(void)
{
    return ksv_new(",", "\n", "\"");
}

ksv_t * ksv_new(char *delimeter, char *end_of_line, char *quote)
{
    assert(delimeter && 0 != strcmp("", delimeter));
    assert(end_of_line && 0 != strcmp("", end_of_line));
    assert(quote && 0 != strcmp("", quote));

    ksv_t *csv = (ksv_t *) malloc(sizeof(ksv_t));
    if (!csv) {
        PUTERR("Failed to allocate csv object");
        PUTERR("Check available system memory");
        return csv;
    }

    csv->row = 0;
    csv->col = 0;

    csv->size_header = 0;
    csv->capacity_header = 8;  /* Arbitrary header width. */
    csv->index_header = 0;

    csv->size_rows = 0;
    csv->capacity_rows = 8;  /* Arbitrary content width. */
    csv->index_col = 0;
    csv->index_row = 0;

    csv->header = NULL;  /* CSV sheet may be header-less. */
    csv->rows = NULL;  /* Lazy row allocation. */

    csv->delimeter = delimeter;
    csv->end_of_line = end_of_line;
    csv->quote = quote;

    return csv;
}

void ksv_delete(void *self)
{
    assert(self);

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

    size_t capacity_rows = ((ksv_t *) self)->capacity_rows;
    char **rows = ((ksv_t *) self)->rows;
    if (rows) {
        size_t i;
        for (i = 0; i < capacity_rows; i++) {
            if (rows[i]) {
                free((void *) rows[i]);
            }
        }

        free((void *) rows);
    }

    free(self);
}

BOOL ksv_has_header(ksv_t *self)
{
    assert(self);

    return self->header ? TRUE : FALSE;
}

size_t ksv_row(ksv_t *self)
{
    assert(self);

    return self->row;
}

size_t ksv_col(ksv_t *self)
{
    assert(self);

    return self->col;
}

void ksv_restart(ksv_t *self)
{
    assert(self);

    self->index_header = 0;
    self->index_col = 0;
    self->index_row = 0;
}

char * ksv_next_header(ksv_t *self)
{
    assert(self);

    if (!(self->header))
        return NULL;

    if (self->index_header >= self->size_header)
        return NULL;

    char *header = self->header[self->index_header];
    self->index_header += 1;

    return header;
}

BOOL ksv_next_column(ksv_t *self)
{
    assert(self);

    if (self->index_col + 1 >= self->col)
        return FALSE;

    self->index_col += 1;
    self->index_row = 0;

    return TRUE;
}

char * ksv_next_data_by_column(ksv_t *self)
{
    assert(self);

    if (self->index_row + self->index_col >= self->size_rows)
        return NULL;

    size_t index = self->index_row + self->index_col;
    char *field =  self->rows[index];

    self->index_row += self->col;

    return field;
}

BOOL ksv_next_row(ksv_t *self)
{
    assert(self);

    if (self->index_row * self->col + self->index_col >= self->size_rows)
        return FALSE;

    self->index_col = 0;
    self->index_row += 1;

    return TRUE;
}

char * ksv_next_data_by_row(ksv_t *self)
{
    assert(self);

    if (self->index_col >= self->col)
        return NULL;

    size_t index = self->index_row * self->col + self->index_col;
    char *field = self->rows[index];

    self->index_col += 1;

    return field;
}

static KSV_STATUS ksv_header_push(ksv_t *self, char *field);
static KSV_STATUS ksv_rows_push(ksv_t *self, char *field);

KSV_STATUS ksv_load_table_with_header_strictly(ksv_t *self, FILE *stream)
{
    assert(self);

    char *line = NULL;
    ksv_lexer_t *lexer = NULL;
    ksv_parser_t *parser = NULL;

    KSV_STATUS ksv_status = ksv_load_header(self, stream);
    if (KSV_SUCCESS != ksv_status)
        goto ERROR_CSV;

    /* When loading a whole CSV sheet, it is very likely that the count of
       fields will exceed 64. */
    self->capacity_rows = 64;
    self->rows = (char **) malloc(self->capacity_rows * sizeof(char *));
    if (!(self->rows)) {
    #if DEBUG
        PUTERR("Failed to allocate rows for csv object");
        PUTERR("Check available system memory");
    #endif
        return KSV_NO_MEMORY;
    }

    {
        size_t i;
        for (i = 0; i < self->capacity_rows; i++)
            self->rows[i] = NULL;
    }

    size_t line_width = 150;  /* Sensible initial line width. */
    line = (char *) malloc(line_width * sizeof(char));
    if (!line) {
    #if DEBUG
        PUTERR("Failed to allocate memory for C string");
        PUTERR("Check available system memory");
    #endif
        return KSV_NO_MEMORY;
    }

    ksv_status = KSV_FAILURE;
    while (fgets(line, line_width, stream)) {
        if (line_width == strlen(line)) {
            if ('\n' != line[line_width-1]) {
                line_width <<= 1;
                if (!realloc(line, line_width)) {
                #if DEBUG
                    PUTERR("Failed to realloc memory for C string");
                    PUTERR("Check available system memory");
                #endif
                    ksv_status = KSV_NO_MEMORY;
                    goto ERROR_CSV;
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
            if (!lexer) {
                ksv_status = KSV_NO_MEMORY;
                goto ERROR_CSV;
            }

            parser = ksv_parser_new();
            if (!parser) {
                ksv_status = KSV_NO_MEMORY;
                goto ERROR_CSV;
            }

            ksv_status = ksv_lexer_lex(lexer, line);
            if (KSV_SUCCESS != ksv_status)
                goto ERROR_CSV;

            ksv_status = ksv_parser_parse(parser, lexer);
            if (KSV_SUCCESS != ksv_status)
                goto ERROR_CSV;

            ksv_ast_t *ast = ksv_parser_next(parser);
            char *field = NULL;
            while (ast) {
                switch (ksv_ast_type(ast)) {
                case KSV_AST_FIELD:
                    field = ksv_ast_string(ast);
                    if (!field) {
                        ksv_status = KSV_NO_MEMORY;
                        goto ERROR_CSV;
                    }

                    ksv_status = ksv_rows_push(self, field);
                    if (KSV_SUCCESS != ksv_status)
                        goto ERROR_CSV;

                    break;
                case KSV_AST_DELIMITER:
                    /* Pass. */
                    break;
                case KSV_AST_EOL:
                    /* Stop reading CSV row. */
                    goto END_CSV_ROW;
                default:
                    assert(0 && "Unknown ksv ast");
                }

                ast = ksv_parser_next(parser);
            }
        }

    END_CSV_ROW:
        if (0 != self->size_rows % self->col) {
            ksv_status = KSV_INVALID_FILE;
            goto ERROR_CSV;
        }

        self->row += 1;

        ksv_lexer_delete(lexer);
        lexer = NULL;

        ksv_parser_delete(parser);
        parser = NULL;
    }

    free(line);

    return KSV_SUCCESS;

ERROR_CSV:
    if (parser)
        ksv_parser_delete(parser);

    if (lexer)
        ksv_lexer_delete(lexer);

    if (line)
        free(line);

    return ksv_status;
}

KSV_STATUS ksv_load_header(ksv_t *self, FILE *stream)
{
    assert(self);

    self->header = \
        (char **) malloc(self->capacity_header * sizeof(char *));
    if (!(self->header))
        return KSV_NO_MEMORY;

    {
        size_t i;
        for (i = 0; i < self->capacity_header; ++i)
            self->header[i] = NULL;
    }

    char *line = NULL;
    char *more_line = NULL;
    char *buf = NULL;
    char *more_buf = NULL;
    ksv_lexer_t *lexer = NULL;
    ksv_parser_t *parser = NULL;

    size_t line_width = 150;  /* Sensible initial line width. */
    line = (char *) malloc(line_width * sizeof(char));
    if (!line) {
    #if DEBUG
        PUTERR("Failed to allocate memory for C string");
        PUTERR("Check available system memory");
    #endif
        return KSV_NO_MEMORY;
    }

    line[0] = '\0';

    size_t buffer_width = 150;  /* Sensible initial buffer width. */
    size_t buffer_offset = 0;
    buf = (char *) malloc(buffer_width * sizeof(char));
    if (!buf) {
    #if DEBUG
        PUTERR("Failed to allocate memory for string buffer");
        PUTERR("Check available system memory");
    #endif
        free(line);
        return KSV_NO_MEMORY;
    }

    buf[0] = '\0';

    KSV_STATUS ksv_status = KSV_FAILURE;
    while (fgets(line, line_width, stream)) {
        if (line_width == strlen(line)) {
            if ('\n' != line[line_width-1]) {
                line_width <<= 1;
                more_line = realloc(line, line_width);
                if (!more_line) {
                #if DEBUG
                    PUTERR("Failed to realloc memory for C string");
                    PUTERR("Check available system memory");
                #endif
                    ksv_status = KSV_NO_MEMORY;
                    goto ERROR_CSV;
                }
                else {
                    line = more_line;
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
            if (!lexer) {
                ksv_status = KSV_NO_MEMORY;
                goto ERROR_CSV;
            }

            parser = ksv_parser_new();
            if (!parser) {
                ksv_status = KSV_NO_MEMORY;
                goto ERROR_CSV;
            }

            if (KSV_UNPAIRED_QUOTE == ksv_status) {
            #if DEBUG
                PUTERR("Line to add to buffer: -->%s<--", line);
                PUTERR("Buffer before scan: -->%s<--", buf);
            #endif
                if (buffer_width - buffer_offset <= line_width) {
                #if DEBUG
                    PUTERR("Extend text buffer");
                #endif
                    while (buffer_width - buffer_offset <= line_width) {
                        buffer_width <<= 1;
                    }

                    more_buf = realloc(buf, buffer_width);

                    if (!more_buf) {
                    #if DEBUG
                        PUTERR("Failed to allocate memory for C string buffer");
                        PUTERR("Check available system memory");
                    #endif
                        ksv_status = KSV_NO_MEMORY;
                        goto ERROR_CSV;
                    }
                    else {
                        buf = more_buf;
                    }
                }

                strcat(buf, line);
                buffer_offset += line_width;
                buf[buffer_offset] = '\0';

            #if DEBUG
                PUTERR("Buffer to scan: -->%s<--", buf);
            #endif

                ksv_status = ksv_lexer_lex(lexer, buf);
            }
            else {
                ksv_status = ksv_lexer_lex(lexer, line);

                /* Clean text buffer. */
                buf[0] = '\0';
                buffer_offset = 0;
            }

            if (KSV_SUCCESS != ksv_status)
                goto ERROR_CSV;

            ksv_status = ksv_parser_parse(parser, lexer);
            if (KSV_UNPAIRED_QUOTE == ksv_status) {
                if (0 == buffer_offset) {
                    if (buffer_width - buffer_offset <= line_width + 1) {
                        while (buffer_width - buffer_offset <= line_width + 1) {
                            buffer_width <<= 1;
                        }

                        more_buf = realloc(buf, buffer_width);

                        if (!more_buf) {
                        #if DEBUG
                            PUTERR("Failed to allocate memory for C string buffer");
                            PUTERR("Check available system memory");
                        #endif
                            ksv_status = KSV_NO_MEMORY;
                            goto ERROR_CSV;
                        }
                        else {
                            buf = more_buf;
                        }

                        strcpy(buf, line);
                        buffer_offset += line_width;
                        buf[buffer_offset] = '\0';

                    #if DEBUG
                        PUTERR("Copy string to buffer: -->%s<--", buf);
                    #endif
                    }
                }

                goto END_CSV_RECORD;
            }
            else if (KSV_SUCCESS != ksv_status)
                goto ERROR_CSV;

            ksv_ast_t *ast = ksv_parser_next(parser);
            char *field = NULL;
            while (ast) {
                switch (ksv_ast_type(ast)) {
                case KSV_AST_FIELD:
                    field = ksv_ast_string(ast);
                    if (!field) {
                        ksv_status = KSV_NO_MEMORY;
                        goto ERROR_CSV;
                    }

                    ksv_status = ksv_header_push(self, field);
                    if (KSV_SUCCESS != ksv_status) {
                        goto ERROR_CSV;
                    }

                    self->col += 1;
                    break;
                case KSV_AST_DELIMITER:
                    /* Pass. */
                    break;
                case KSV_AST_EOL:
                    /* Stop reading CSV row. */
                    break;
                default:
                    assert(0 && "Unknown ksv ast");
                }

                ast = ksv_parser_next(parser);
            }

        END_CSV_RECORD:
            ksv_lexer_delete(lexer);
            lexer = NULL;

            ksv_parser_delete(parser);
            parser = NULL;
        }

        /* Only read first line. */
        if (KSV_UNPAIRED_QUOTE != ksv_status)
            break;
    }

    free(buf);
    free(line);

    return KSV_SUCCESS;

ERROR_CSV:
    if (parser)
        ksv_parser_delete(parser);

    if (lexer)
        ksv_lexer_delete(lexer);

    if (buf)
        free(buf);

    if (line)
        free(line);

    return ksv_status;
}

KSV_STATUS ksv_load_record(ksv_t *self, FILE *stream)
{
    assert(self);

    if (!(self->rows)) {
        /* Mallocate memory for a row in CSV sheet. */
        self->rows = \
            (char **) malloc(self->capacity_rows * sizeof(char *));
        if (!(self->rows)) {
        #if DEBUG
            PUTERR("Failed to allocate rows for csv object");
            PUTERR("Check available system memory");
        #endif
            return KSV_NO_MEMORY;
        }

        {
            size_t i;
            for (i = 0; i < self->capacity_rows; i++)
                self->rows[i] = NULL;
        }
    }
    else {
        /* Clean old fields. */
        size_t i;
        for (i = 0; i < self->capacity_rows; ++i) {
            if (self->rows[i])
                free((void *) self->rows[i]);
        }
    }

    /* Re-initialize the internal status of ksv object. */
    self->size_rows = 0;
    self->col = 0;
    self->row = 0;

    char *line = NULL;
    char *more_line = NULL;
    char *buf = NULL;
    char *more_buf = NULL;
    ksv_lexer_t *lexer = NULL;
    ksv_parser_t *parser = NULL;

    size_t line_width = 150;  /* Sensible initial line width. */
    line = (char *) malloc(line_width * sizeof(char));
    if (!line) {
    #if DEBUG
        PUTERR("Failed to allocate memory for C string");
        PUTERR("Check available system memory");
    #endif
        return KSV_NO_MEMORY;
    }

    line[0] = '\0';

    size_t buffer_width = 150;  /* Sensible initial buffer width. */
    size_t buffer_offset = 0;
    buf = (char *) malloc(buffer_width * sizeof(char));
    if (!buf) {
    #if DEBUG
        PUTERR("Failed to allocate memory for C string buffer");
        PUTERR("Check available system memory");
    #endif
        free(line);
        return KSV_NO_MEMORY;
    }

    buf[0] = '\0';

    KSV_STATUS ksv_status = KSV_FAILURE;
    while (fgets(line, line_width, stream)) {
        if (line_width == strlen(line)) {
            if ('\n' != line[line_width-1]) {
                line_width <<= 1;
                more_line = realloc(line, line_width);
                if (!more_line) {
                #if DEBUG
                    PUTERR("Failed to realloc memory for C string");
                    PUTERR("Check available system memory");
                #endif
                    ksv_status = KSV_NO_MEMORY;
                    goto ERROR_CSV;
                }
                else {
                    line = more_line;
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
            if (!lexer) {
                ksv_status = KSV_NO_MEMORY;
                goto ERROR_CSV;
            }

            parser = ksv_parser_new();
            if (!parser) {
                ksv_status = KSV_NO_MEMORY;
                goto ERROR_CSV;
            }

            if (KSV_UNPAIRED_QUOTE == ksv_status) {
            #if DEBUG
                PUTERR("Line to add to buffer: -->%s<--", line);
                PUTERR("Buffer before scan: -->%s<--", buf);
            #endif
                if (buffer_width - buffer_offset <= line_width) {
                #if DEBUG
                    PUTERR("Extend text buffer");
                #endif
                    while (buffer_width - buffer_offset <= line_width) {
                        buffer_width <<= 1;
                    }

                    more_buf = realloc(buf, buffer_width);

                    if (!more_buf) {
                    #if DEBUG
                        PUTERR("Failed to allocate memory for C string buffer");
                        PUTERR("Check available system memory");
                    #endif
                        ksv_status = KSV_NO_MEMORY;
                        goto ERROR_CSV;
                    }
                    else {
                        buf = more_buf;
                    }
                }

                strcat(buf, line);
                buffer_offset += line_width;
                buf[buffer_offset] = '\0';

            #if DEBUG
                PUTERR("Buffer to scan: -->%s<--", buf);
            #endif

                ksv_status = ksv_lexer_lex(lexer, buf);
            }
            else {
                ksv_status = ksv_lexer_lex(lexer, line);

                /* Clean text buffer. */
                buf[0] = '\0';
                buffer_offset = 0;
            }

            if (KSV_SUCCESS != ksv_status)
                goto ERROR_CSV;

            ksv_status = ksv_parser_parse(parser, lexer);
            if (KSV_UNPAIRED_QUOTE == ksv_status) {
            #if DEBUG
                PUTERR("Try to copy string to empty buffer");
            #endif
                if (0 == buffer_offset) {
                    if (buffer_width - buffer_offset <= line_width + 1) {
                        while (buffer_width - buffer_offset <= line_width + 1) {
                            buffer_width <<= 1;
                        }

                        more_buf = realloc(buf, buffer_width);

                        if (!more_buf) {
                        #if DEBUG
                            PUTERR("Failed to allocate memory for C string buffer");
                            PUTERR("Check available system memory");
                        #endif
                            ksv_status = KSV_NO_MEMORY;
                            goto ERROR_CSV;
                        }
                        else {
                            buf = more_buf;
                        }

                        strcpy(buf, line);
                        buffer_offset += line_width;
                        buf[buffer_offset] = '\0';

                    #if DEBUG
                        PUTERR("Copy string to buffer: -->%s<--", buf);
                    #endif
                    }
                }

                goto END_CSV_RECORD;
            }
            else if (KSV_SUCCESS != ksv_status) {
                goto ERROR_CSV;
            }

            ksv_ast_t *ast = ksv_parser_next(parser);
            char *field = NULL;
            while (ast) {
                switch (ksv_ast_type(ast)) {
                case KSV_AST_FIELD:
                    field = ksv_ast_string(ast);
                    if (!field) {
                        ksv_status = KSV_NO_MEMORY;
                        goto ERROR_CSV;
                    }

                    ksv_status = ksv_rows_push(self, field);
                    if (KSV_SUCCESS != ksv_status) {
                        goto ERROR_CSV;
                    }

                    self->col += 1;
                    break;
                case KSV_AST_DELIMITER:
                    /* Pass. */
                    break;
                case KSV_AST_EOL:
                    /* Stop reading CSV row. */
                    break;
                default:
                    assert(0 && "Unknown ksv ast");
                }

                ast = ksv_parser_next(parser);
            }

        END_CSV_RECORD:
        #if DEBUG
            PUTERR("Buffer before inner conditional: -->%s<--", buf);
        #endif
            ksv_lexer_delete(lexer);
            lexer = NULL;

            ksv_parser_delete(parser);
            parser = NULL;
        }

    #if DEBUG
        PUTERR("Buffer after inner conditional: -->%s<--", buf);
    #endif

        /* Only read first record. */
        if (KSV_UNPAIRED_QUOTE != ksv_status)
            break;
    }

    free(buf);
    free(line);

    return KSV_SUCCESS;

ERROR_CSV:
    if (parser)
        ksv_parser_delete(parser);

    if (lexer)
        ksv_lexer_delete(lexer);

    if (line)
        free(line);

    if (buf)
        free(buf);

    return ksv_status;
}

static KSV_STATUS ksv_header_expand(ksv_t *self);

static KSV_STATUS ksv_header_push(ksv_t *self, char *field)
{
    assert(self);

    KSV_STATUS s = ksv_header_expand(self);
    if (KSV_SUCCESS != s)
        return s;

    self->header[self->size_header] = field;
    self->size_header += 1;

    return KSV_SUCCESS;
}

static KSV_STATUS ksv_header_expand(ksv_t *self)
{
    assert(self);

    if (self->size_header < self->capacity_header)
        return KSV_SUCCESS;

    self->capacity_header <<= 1;
    char **old_header = self->header;
    char **new_header = \
        (char **) malloc(self->capacity_header * sizeof(char *));
    if (!new_header) {
    #if DEBUG
        PUTERR("Failed to reallocate memory for header of csv object");
        PUTERR("Check available system memory");
    #endif
        return KSV_NO_MEMORY;
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

    return KSV_SUCCESS;
}

static KSV_STATUS ksv_rows_expand(ksv_t *self);

static KSV_STATUS ksv_rows_push(ksv_t *self, char *field)
{
    assert(self);

    KSV_STATUS s = ksv_rows_expand(self);
    if (KSV_SUCCESS != s)
        return s;

    self->rows[self->size_rows] = field;
    self->size_rows += 1;

    return KSV_SUCCESS;
}

static KSV_STATUS ksv_rows_expand(ksv_t *self)
{
    assert(self);

    if (self->size_rows < self->capacity_rows)
        return KSV_SUCCESS;

    self->capacity_rows <<= 1;
    char **old_rows = self->rows;
    char **new_rows = \
        (char **) malloc(self->capacity_rows * sizeof(char *));
    if (!new_rows) {
    #if DEBUG
        PUTERR("Failed to allocate memory for rows of csv object");
        PUTERR("Check available system memory");
    #endif
        return KSV_NO_MEMORY;
    }

    {
        size_t i;
        for (i = 0; i < self->size_rows; ++i)
            new_rows[i] = old_rows[i];
    }

    {
        size_t i;
        for (i = self->size_rows; i < self->capacity_rows; ++i)
            new_rows[i] = NULL;
    }

    self->rows = new_rows;
    free(old_rows);

    return KSV_SUCCESS;
}
