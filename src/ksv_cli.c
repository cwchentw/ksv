#include <errno.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "arrayd.h"
#include "ksv.h"
#include "ksv_argument.h"
#include "ksv_help.h"
#include "ksv_metadata.h"
#include "print.h"

KSV_STATUS show_sheet(FILE *stream, ksv_t *ksv);
KSV_STATUS show_deviation(ksv_t *ksv, FILE *in);
KSV_STATUS show_quartiles(ksv_t *ksv, FILE *in);
KSV_STATUS show_quintiles(ksv_t *ksv, FILE *in);

#define CHECK_DIMENSION(cmd) \
    ((cmd) == KSV_COMMAND_WIDTH \
     || (cmd) == KSV_COMMAND_HEIGHT \
     || (cmd) == KSV_COMMAND_DIMENSION)

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

    if (KSV_COMMAND_UNKNOWN == command) {
        PUTERR("Invalid command");
        goto ERROR_KSV;
    }

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

    if (KSV_COMMAND_STATS == command
        && KSV_COMMAND_UNKNOWN == ksv_argument_subcommand(arg)) {
        PUTERR("Invalid stats command");
        goto ERROR_KSV;
    }

    const char *path = ksv_argument_path(arg);
    if (!path) {
       PUTERR("No valid path");
       goto ERROR_KSV;
    }

#if _MSC_VER
    if (0 != fopen_s(&fp, path, "r")) {
        PUTERR("Failed to open file at %s", path);
        goto ERROR_KSV;
    }
#else
    fp = fopen(path, "r");
    if (!fp) {
        PUTERR("Failed to open file at %s", path);
        goto ERROR_KSV;
    }
#endif

    ksv = ksv_new_default();
    if (!ksv)
        goto ERROR_KSV;



#if DEBUG
    PUTERR("CSV dimension (col, row): (%lu, %lu)", ksv_col(ksv), ksv_row(ksv));
#endif

    if (CHECK_DIMENSION(ksv_argument_command(arg))) {
        if (KSV_SUCCESS != ksv_load_header(ksv, fp))
            goto ERROR_KSV;

        size_t width = ksv_col(ksv);
        size_t height = 0;
        while (!feof(fp)) {
            if (KSV_SUCCESS != ksv_load_record(ksv, fp))
                goto ERROR_KSV;

            if (width != ksv_col(ksv)) {
                PUTERR("Unequal sheet width after row %lu", height);
                goto ERROR_KSV;
            }

            height += 1;
        }

        if (IS_KSV_COMMAND_EQUAL(
            KSV_COMMAND_WIDTH,
            ksv_argument_command(arg))) {
            PUTS("%lu", width);
        }
        else if (IS_KSV_COMMAND_EQUAL(
            KSV_COMMAND_HEIGHT,
            ksv_argument_command(arg))) {
            PUTS("%lu", height);
        }
        else if (IS_KSV_COMMAND_EQUAL(
            KSV_COMMAND_DIMENSION,
            ksv_argument_command(arg))) {
            PUTS("%lu %lu", width, height);
        }
    }
    else if (KSV_COMMAND_HEADER == ksv_argument_command(arg)) {
        if (KSV_SUCCESS != ksv_load_header(ksv, fp))
            goto ERROR_KSV;

        char *field = ksv_next_header(ksv);
        while (field) {
            PUTS("%s", field);

            field = ksv_next_header(ksv);
        }
    }
    else if (KSV_COMMAND_SHOW == ksv_argument_command(arg)) {
        if (KSV_SUCCESS != ksv_load_table_with_header_strictly(ksv, fp))
            goto ERROR_KSV;

        KSV_STATUS s = show_sheet(stdout, ksv);
        if (KSV_SUCCESS != s)
            goto ERROR_KSV;
    }
    else if (KSV_COMMAND_STATS == ksv_argument_command(arg)) {
        if (IS_KSV_COMMAND_EQUAL(
            KSV_COMMAND_UNKNOWN, ksv_argument_subcommand(arg))) {
            PUTERR("Invalid stats command");
            goto ERROR_KSV;
        }
        else if (IS_KSV_COMMAND_EQUAL(
            KSV_STATS_COMMAND_DEVIATION, ksv_argument_subcommand(arg))) {
            if (KSV_SUCCESS != show_deviation(ksv, fp)) {
                goto ERROR_KSV;
            }
        }
        else if (IS_KSV_COMMAND_EQUAL(
            KSV_STATS_COMMAND_QUARTILES, ksv_argument_subcommand(arg))) {
            if (KSV_SUCCESS != show_quartiles(ksv, fp)) {
                goto ERROR_KSV;
            }
        }
        else if (IS_KSV_COMMAND_EQUAL(
            KSV_STATS_COMMAND_QUINTILES, ksv_argument_subcommand(arg))) {
            if (KSV_SUCCESS != show_quintiles(ksv, fp)) {
                goto ERROR_KSV;
            }
        }
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

            if (ksv_has_header(ksv)) {
                char *header = ksv_next_header(ksv);
                sz = sz > strlen(header) ? sz : strlen(header);
            }

            char *field = ksv_next_data_by_column(ksv);
        #if DEBUG
            if (field)
                PUTERR("Field to compare: -->%s<--", field);
        #endif
            while (field) {
                sz = sz > strlen(field) ? sz : strlen(field);

                field = ksv_next_data_by_column(ksv);
            #if DEBUG
                if(field)
                    PUTERR("Field to compare: -->%s<--", field);
            #endif
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

    /* Print horizontal line if sheet header exists. */
    if (ksv_has_header(ksv)) {
        size_t i;
        for (i = 0; i < total; ++i)
            line[i] = '-';

        line[0] = '+';
        size_t temp = 1;
        for (i = 0; i < ksv_col(ksv); i+=1) {
            line[temp+ss[i]] = '+';
            temp += ss[i]+1;
        }

        line[total] = '\0';

        fprintf(stream, "%s%s", line, END_OF_LINE);

        line[0] = '\0';
    }

    /* Print header if it exists. */
    if (ksv_has_header(ksv)) {
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

        line[total] = '\0';

        fprintf(stream, "%s%s", line, END_OF_LINE);

        line[0] = '\0';
    }

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

KSV_STATUS show_deviation(ksv_t *ksv, FILE *in)
{
    KSV_STATUS status = ksv_load_header(ksv, in);
    if (KSV_SUCCESS != status) {
        return status;
    }

    size_t col = ksv_col(ksv);

    char **headers = NULL;
    BOOL *is_valid = NULL;
    arrayd_t **arr = NULL;

    headers = (char **) malloc(col * sizeof(char *));
    if (!headers) {
        status = KSV_NO_MEMORY;
        goto ERROR_KSV;
    }

    ksv_restart(ksv);
    {
        size_t i;
        for (i = 0; i < col; ++i)
            headers[i] = ksv_next_header(ksv);
    }

    is_valid = (BOOL *) malloc(col * sizeof(BOOL));
    if (!is_valid) {
        return KSV_NO_MEMORY;
    }

    {
        size_t i;
        for (i = 0; i < col; ++i)
            is_valid[i] = TRUE;
    }

    arr = (arrayd_t **) malloc(col * sizeof(arrayd_t *));
    if (!arr) {
        free(is_valid);
        return KSV_NO_MEMORY;
    }

    {
        size_t i;
        for (i = 0; i < col; ++i) {
            arr[i] = arrayd_new();
            if (!(arr[i])) {
                for (i = 0; i < col; ++i) {
                    if (arr[i])
                        arrayd_delete(arr[i]);
                }

                return KSV_NO_MEMORY;
            }
        }
    }

    size_t n = 0;
    while (!feof(in)) {
        if (KSV_SUCCESS != ksv_load_record(ksv, in)) {
            PUTERR("Failed to load a sheet record");
            goto ERROR_KSV;
        }

        ksv_restart(ksv);

        char *field;
        size_t i;
        for (i = 0; i < col; ++i) {
            field = ksv_next_data_by_row(ksv);

            if (!(is_valid[i]))
                continue;

            char *ptr;
            double result = strtod(field, &ptr);

            if (0 == result) {
                if (errno == ERANGE || ptr) {
                    is_valid[i] = FALSE;
                }
            }

            if (!arrayd_push(arr[i], result)) {
                status = KSV_FAILURE;
                goto ERROR_KSV;
            }
        }

        ++n;
    }

    if (n < 30) {
        PUTERR("Too few data to show");
        status = KSV_FAILURE;
        goto ERROR_KSV;
    }

    {
        size_t i;
        for (i = 0; i < col; ++i) {
            PUTS("%lu:%s", i+1, headers[i]);
            if (!is_valid[i]) {
                PUTS("    Not numerical");
                continue;
            }

            double sum = 0.0;
            {
                size_t j;
                for (j = 0; j < n; ++j)
                    sum += arrayd_at(arr[i], j);
            }

            double mean = sum / n;

            double sqr = 0.0;
            {
                size_t j;
                for (j = 0; j < n; ++j) {
                    double temp = arrayd_at(arr[i], j) - mean;
                    sqr += temp * temp;
                }
            }

            double sd = sqrt(sqr / (n - 1));

            PUTS("   -2 SD: %f", mean - 2 * sd);
            PUTS("   -1 SD: %f", mean - sd);
            PUTS("    mean: %f", mean);
            PUTS("   +1 SD: %f", mean + sd);
            PUTS("   +2 SD: %f", mean + 2 * sd);

            if (i < col - 1)
                PUTS("");
        }
    }

    {
        size_t i;
        for (i = 0; i < col; ++i)
            arrayd_delete(arr[i]);

        free(arr);
    }

    free(is_valid);
    free(headers);

    return KSV_SUCCESS;

ERROR_KSV:
    if (arr) {
        size_t i;
        for (i = 0; i < col; ++i)
            arrayd_delete(arr[i]);

        free(arr);
    }

    if (is_valid)
        free(is_valid);

    if (headers)
        free(headers);

    return status;
}

KSV_STATUS show_quartiles(ksv_t *ksv, FILE *in)
{
    KSV_STATUS status = ksv_load_header(ksv, in);
    if (KSV_SUCCESS != status) {
        return status;
    }

    size_t col = ksv_col(ksv);

    char **headers = NULL;
    BOOL *is_valid = NULL;
    arrayd_t **arr = NULL;

    headers = (char **) malloc(col * sizeof(char *));
    if (!headers) {
        status = KSV_NO_MEMORY;
        goto ERROR_KSV;
    }

    ksv_restart(ksv);
    {
        size_t i;
        for (i = 0; i < col; ++i)
            headers[i] = ksv_next_header(ksv);
    }

    is_valid = (BOOL *) malloc(col * sizeof(BOOL));
    if (!is_valid) {
        return KSV_NO_MEMORY;
    }

    {
        size_t i;
        for (i = 0; i < col; ++i)
            is_valid[i] = TRUE;
    }

    arr = (arrayd_t **) malloc(col * sizeof(arrayd_t *));
    if (!arr) {
        free(is_valid);
        return KSV_NO_MEMORY;
    }

    {
        size_t i;
        for (i = 0; i < col; ++i) {
            arr[i] = arrayd_new();
            if (!(arr[i])) {
                for (i = 0; i < col; ++i) {
                    if (arr[i])
                        arrayd_delete(arr[i]);
                }

                return KSV_NO_MEMORY;
            }
        }
    }

    size_t n = 0;
    while (!feof(in)) {
        if (KSV_SUCCESS != ksv_load_record(ksv, in)) {
            PUTERR("Failed to load a sheet record");
            goto ERROR_KSV;
        }

        ksv_restart(ksv);

        char *field;
        size_t i;
        for (i = 0; i < col; ++i) {
            field = ksv_next_data_by_row(ksv);

            if (!(is_valid[i]))
                continue;

            char *ptr;
            double result = strtod(field, &ptr);

            if (0 == result) {
                if (errno == ERANGE || ptr) {
                    is_valid[i] = FALSE;
                }
            }

            if (!arrayd_push(arr[i], result)) {
                status = KSV_FAILURE;
                goto ERROR_KSV;
            }
        }

        ++n;
    }

    if (n < 4) {
        PUTERR("Too few data to show");
        status = KSV_FAILURE;
        goto ERROR_KSV;
    }

    {
        size_t i;
        for (i = 0; i < col; ++i)
            arrayd_sort(arr[i]);
    }

    {
        size_t i;
        for (i = 0; i < col; ++i) {
            PUTS("%lu:%s", i+1, headers[i]);

            if (!is_valid[i]) {
                PUTS("    Not numerical");
            }
            else {
                size_t j;
                BOOL q[] = {FALSE, FALSE, FALSE, FALSE};
                for (j = 0; j < n; ++j) {
                    double ratio = ((double) j+1) / ((double) n);

                    if (ratio < 0.25) {
                        if (!q[0]) {
                            PUTS("      0%%: %f", arrayd_at(arr[i], j));
                            q[0] = TRUE;
                        }
                    }
                    else if (ratio < 0.5) {
                        if (!q[1]) {
                            PUTS("     25%%: %f", arrayd_at(arr[i], j));
                            q[1] = TRUE;
                        }
                    }
                    else if (ratio < 0.75) {
                        if (!q[2]) {
                            PUTS("     50%%: %f", arrayd_at(arr[i], j));
                            q[2] = TRUE;
                        }
                    }
                    else if (ratio < 1.0) {
                        if (!q[3]) {
                            PUTS("     75%%: %f", arrayd_at(arr[i], j));
                            q[3] = TRUE;
                        }
                    }
                    else if (j == n - 1) {
                        PUTS("    100%%: %f", arrayd_at(arr[i], j));
                    }
                }
            }

            if (i < col - 1)
                PUTS("");
        }
    }

    {
        size_t i;
        for (i = 0; i < col; ++i)
            arrayd_delete(arr[i]);

        free(arr);
    }

    free(is_valid);
    free(headers);

    return KSV_SUCCESS;

ERROR_KSV:
    if (arr) {
        size_t i;
        for (i = 0; i < col; ++i)
            arrayd_delete(arr[i]);

        free(arr);
    }

    if (is_valid)
        free(is_valid);

    if (headers)
        free(headers);

    return status;
}

KSV_STATUS show_quintiles(ksv_t *ksv, FILE *in)
{
    KSV_STATUS status = ksv_load_header(ksv, in);
    if (KSV_SUCCESS != status) {
        return status;
    }

    size_t col = ksv_col(ksv);

    char **headers = NULL;
    BOOL *is_valid = NULL;
    arrayd_t **arr = NULL;

    headers = (char **) malloc(col * sizeof(char *));
    if (!headers) {
        status = KSV_NO_MEMORY;
        goto ERROR_KSV;
    }

    ksv_restart(ksv);
    {
        size_t i;
        for (i = 0; i < col; ++i)
            headers[i] = ksv_next_header(ksv);
    }

    is_valid = (BOOL *) malloc(col * sizeof(BOOL));
    if (!is_valid) {
        return KSV_NO_MEMORY;
    }

    {
        size_t i;
        for (i = 0; i < col; ++i)
            is_valid[i] = TRUE;
    }

    arr = (arrayd_t **) malloc(col * sizeof(arrayd_t *));
    if (!arr) {
        free(is_valid);
        return KSV_NO_MEMORY;
    }

    {
        size_t i;
        for (i = 0; i < col; ++i) {
            arr[i] = arrayd_new();
            if (!(arr[i])) {
                for (i = 0; i < col; ++i) {
                    if (arr[i])
                        arrayd_delete(arr[i]);
                }

                return KSV_NO_MEMORY;
            }
        }
    }

    size_t n = 0;
    while (!feof(in)) {
        if (KSV_SUCCESS != ksv_load_record(ksv, in)) {
            PUTERR("Failed to load a sheet record");
            goto ERROR_KSV;
        }

        ksv_restart(ksv);

        char *field;
        size_t i;
        for (i = 0; i < col; ++i) {
            field = ksv_next_data_by_row(ksv);

            if (!(is_valid[i]))
                continue;

            char *ptr;
            double result = strtod(field, &ptr);

            if (0 == result) {
                if (errno == ERANGE || ptr) {
                    is_valid[i] = FALSE;
                }
            }

            if (!arrayd_push(arr[i], result)) {
                status = KSV_FAILURE;
                goto ERROR_KSV;
            }
        }

        ++n;
    }

    if (n < 5) {
        PUTERR("Too few data to show");
        status = KSV_FAILURE;
        goto ERROR_KSV;
    }

    {
        size_t i;
        for (i = 0; i < col; ++i)
            arrayd_sort(arr[i]);
    }

    {
        size_t i;
        for (i = 0; i < col; ++i) {
            PUTS("%lu:%s", i+1, headers[i]);

            if (!is_valid[i]) {
                PUTS("    Not numerical");
            }
            else {
                size_t j;
                BOOL q[] = {FALSE, FALSE, FALSE, FALSE, FALSE};
                for (j = 0; j < n; ++j) {
                    double ratio = ((double) j+1) / ((double) n);

                    if (ratio < 0.2) {
                        if (!q[0]) {
                            PUTS("      0%%: %f", arrayd_at(arr[i], j));
                            q[0] = TRUE;
                        }
                    }
                    else if (ratio < 0.4) {
                        if (!q[1]) {
                            PUTS("     20%%: %f", arrayd_at(arr[i], j));
                            q[1] = TRUE;
                        }
                    }
                    else if (ratio < 0.6) {
                        if (!q[2]) {
                            PUTS("     40%%: %f", arrayd_at(arr[i], j));
                            q[2] = TRUE;
                        }
                    }
                    else if (ratio < 0.8) {
                        if (!q[3]) {
                            PUTS("     60%%: %f", arrayd_at(arr[i], j));
                            q[3] = TRUE;
                        }
                    }
                    else if (ratio < 1.0) {
                        if (!q[4]) {
                            PUTS("     80%%: %f", arrayd_at(arr[i], j));
                            q[4] = TRUE;
                        }
                    }
                    else if (j == n - 1) {
                        PUTS("    100%%: %f", arrayd_at(arr[i], j));
                    }
                }
            }

            if (i < col - 1)
                PUTS("");
        }
    }

    {
        size_t i;
        for (i = 0; i < col; ++i)
            arrayd_delete(arr[i]);

        free(arr);
    }

    free(is_valid);
    free(headers);

    return KSV_SUCCESS;

ERROR_KSV:
    if (arr) {
        size_t i;
        for (i = 0; i < col; ++i)
            arrayd_delete(arr[i]);

        free(arr);
    }

    if (is_valid)
        free(is_valid);

    if (headers)
        free(headers);

    return status;
}
