#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ksv.h"
#include "ksv_argument.h"
#include "ksv_help.h"
#include "ksv_metadata.h"
#include "print.h"

KSV_STATUS show_sheet(FILE *stream, ksv_t *ksv);

int main(int argc, char *argv[])
{
    if (argc < 2) {
        ksv_help(stderr);
        return 1;
    }

    ksv_argument_t *arg;
    FILE *fp = NULL;
    ksv_t *ksv = NULL;

    arg = ksv_argument_parse(argc, argv);
    if (!arg) {
        PUTERR("Failed to parse command line arguments");
        return 1;
    }

    KSV_COMMAND_TYPE command = ksv_argument_command(arg);
    switch (command) {
    case KSV_COMMAND_VERSION:
        ksv_version();
        goto END_CSV;
    case KSV_COMMAND_LICENSE:
        ksv_license();
        goto END_CSV;
    case KSV_COMMAND_HELP:
        ksv_help(stdout);
        goto END_CSV;
    }

    /* Refactor it later. */
    const char *path = ksv_argument_path(arg);

#if _MSC_VER
    if (0 != fopen_s(&fp, path, "r")) {
        PUTERR("Failed to open file at %s", path);
        return 1;
    }
#else
    fp = fopen(path, "r");
    if (!fp) {
        PUTERR("Failed to open file at %s", path);
        return 1;
    }
#endif

    ksv = ksv_new_default();
    if (!ksv)
        goto ERROR_KSV;

    if (KSV_SUCCESS != ksv_load_stream_with_header_strictly(ksv, fp))
        goto ERROR_KSV;

#if DEBUG
    PUTERR("CSV dimension (col, row): (%lu, %lu)", ksv_col(ksv), ksv_row(ksv));
#endif

    if (KSV_COMMAND_SHOW == ksv_argument_command(arg)) {
        KSV_STATUS s = show_sheet(stdout, ksv);
        if (KSV_SUCCESS != s)
            goto ERROR_KSV;
    }

END_CSV:
    if (ksv)
        ksv_delete(ksv);

    if (fp)
        fclose(fp);

    if (arg)
        ksv_argument_delete(arg);

    return 0;

ERROR_KSV:
    if (ksv)
        ksv_delete(ksv);

    if (fp)
        fclose(fp);

    if (arg)
        ksv_argument_delete(arg);

    return 1;
}

KSV_STATUS show_sheet(FILE *stream, ksv_t *ksv)
{
    ksv_restart(ksv);

    size_t *ss = (size_t *) malloc(ksv_col(ksv) * sizeof(size_t));
    if (!ss)
        return KSV_NO_MEMORY;

    {
        size_t i;
        for (i = 0; i < ksv_col(ksv); ++i)
            ss[i] = 0;
    }

    {
        size_t i = 0;
        BOOL is_column_valid = TRUE;
        while (is_column_valid) {
            size_t sz = 0;
            char *header = ksv_next_header(ksv);
            sz = sz > strlen(header) ? sz : strlen(header);

            char *field = ksv_next_data_by_column(ksv);
            while (field) {
                sz = sz > strlen(field) ? sz : strlen(field);

                field = ksv_next_data_by_column(ksv);
            }

            ss[i] = sz;

        #if DEBUG
            PUTERR("Column size at(%lu): %lu", i, ss[i]);
        #endif

            ++i;

            if (!ksv_next_column(ksv))
                is_column_valid = FALSE;
        }
    }

    size_t total = 1;  /* First vertial line. */

    {
        size_t i;
        for (i = 0; i < ksv_col(ksv); ++i)
            total += \
                ss[i]  /* Column size. */
                + 1;   /* Subsequent vertial line. */
    }

    /* 1: Trailing zero. */
    char *line = (char *) malloc((total + 1) * sizeof(char));
    if (!line) {
        free(ss);
        return KSV_NO_MEMORY;
    }

    line[0] = '\0';

    /* Print top horizontal line */
    {
        size_t i;
        for (i = 0; i < total; ++i)
            line[i] = '-';

        line[0] = '+';
        size_t temp = 1;
        for (i = 0; i < ksv_col(ksv); i+=1) {
            line[temp+ss[i]] = '+';
            temp += ss[i]+1;
        }
    }
    line[total] = '\0';

    fprintf(stream, "%s%s", line, END_OF_LINE);

    line[0] = '\0';

    /* Print header. */
    {
        ksv_restart(ksv);

        line[0] = '|';  /* First vertial line. */
        line[1] = '\0';
        size_t i;
        size_t temp = 1;  /* First vertial line. */
        for (i = 0; i < ksv_col(ksv); i++) {
            char *header = ksv_next_header(ksv);

        #if DEBUG
            PUTERR("Header to print: -->%s<--", header);
        #endif

            size_t sz = strlen(header);
            strcpy(line+temp, header);

            if (sz < ss[i]) {
                size_t j;
                for (j = sz; j < ss[i]; ++j)
                    line[temp+j] = ' ';

                line[temp+ss[i]] = '|';  /* Subsequent vertical line. */
                line[temp+ss[i]+1] = '\0';
                temp += ss[i] + 1;
            }
            else {
                line[temp+sz] = '|';
                line[temp+sz+1] = '\0';
                temp += sz + 1;
            }
        }
    }
    line[total] = '\0';

    fprintf(stream, "%s%s", line, END_OF_LINE);

    line[0] = '\0';

    /* Print horizontal line */
    {
        size_t i;
        for (i = 0; i < total; ++i)
            line[i] = '-';

        line[0] = '+';
        size_t temp = 1;
        for (i = 0; i < ksv_col(ksv); i+=1) {
            line[temp+ss[i]] = '+';
            temp += ss[i]+1;
        }
    }

    fprintf(stream, "%s%s", line, END_OF_LINE);

    line[0] = '\0';

    /* Print data. */
    {
        ksv_restart(ksv);

        BOOL is_row_valid = TRUE;
        while (is_row_valid) {
            line[0] = '|';
            line[1] = '\0';

            char *field = ksv_next_data_by_row(ksv);

            size_t i = 0;
            size_t temp = 1;
            while (field) {
                size_t sz = strlen(field);

            #if DEBUG
                PUTERR("Line: -->%s<--", line);
                PUTERR("Data to print: -->%s<--", field);
            #endif

                if (sz < ss[i]) {
                #if DEBUG
                    PUTERR("ss: %lu sz: %lu", ss[i], sz);
                #endif
                    size_t j;
                    for (j = 0; j < ss[i] - sz; ++j) {
                    #if DEBUG
                        PUTERR("temp: %lu j: %lu", temp, j);
                    #endif
                        line[temp+j] = ' ';
                    }

                    line[temp+ss[i]-sz] = '\0';
                    temp += ss[i] - sz;

                #if DEBUG
                    PUTERR("Line: -->%s<--", line);
                #endif

                    strcpy(line+temp, field);

                    line[temp+strlen(field)] = '|';
                    line[temp+strlen(field)+1] = '\0';
                
                #if DEBUG
                    PUTERR("Line: -->%s<--", line);
                #endif

                    temp += strlen(field)+1;
                }
                else {
                    strcpy(line+temp, field);
                    line[temp+sz] = '|';
                    line[temp+sz+1] = '\0';
                    temp += sz + 1;
                }

                ++i;
                field = ksv_next_data_by_row(ksv);
            }
            line[total] = '\0';

            fprintf(stream, "%s%s", line, END_OF_LINE);

            line[0] = '\0';

            if (!ksv_next_row(ksv))
                is_row_valid = FALSE;
        }
    }

    line[0] = '\0';

    /* Print horizontal line */
    {
        size_t i;
        for (i = 0; i < total; ++i)
            line[i] = '-';

        line[0] = '+';
        size_t temp = 1;
        for (i = 0; i < ksv_col(ksv); i+=1) {
            line[temp+ss[i]] = '+';
            temp += ss[i]+1;
        }
    }

    fprintf(stream, "%s%s", line, END_OF_LINE);

    free(line);
    free(ss);

    return KSV_SUCCESS;
}