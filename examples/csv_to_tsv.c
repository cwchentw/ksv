#include <stdio.h>
#include <stdlib.h>
#include "ksv.h"
#include "print.h"

int main(int argc, char *argv[])
{
    if (argc < 2) {
        PUTERR("No input sheet");
        return 1;
    }

    FILE *fp = NULL;
    ksv_t *ksv = NULL;

    char *path = argv[1];

    /* Open a CSV sheet. */
    fp = fopen(path, "r");
    if (!fp) {
        PUTERR("Unable to read file");
        return 1;
    }

    /* Create a ksv object. */
    ksv = ksv_new_default();
    if (!ksv) {
        PUTERR("Failed to create ksv object");
        goto ERROR_MAIN;
    }

    /* Load the header row of the sheet. */
    if (KSV_SUCCESS != ksv_load_header(ksv, fp)) {
        PUTERR("Failed to load sheet header");
        goto ERROR_MAIN;
    }

    /* Iterate over the header(s) of the header row. */
    char *field = ksv_next_header(ksv);
    while (field) {
        PRINT("%s\t", field);

        field = ksv_next_header(ksv);
    }
    PUTS("\b");

    /* Iterate over the sheet. */
    while (!feof(fp)) {
        /* Load one row from the sheet. */
        if (KSV_SUCCESS != ksv_load_record(ksv, fp)) {
            PUTERR("Failed to load a sheet record");
            goto ERROR_MAIN;
        }

        /* Re-init the state of the ksv object. */
        ksv_start(ksv);

        /* Iterate over the cell(s) of the row. */
        field = ksv_next_data_by_row(ksv);
        while (field) {
            PRINT("%s\t", field);

            field = ksv_next_data_by_row(ksv);
        }
        PUTS("\b");
    }

    /* Release system resources. */
    ksv_delete(ksv);
    fclose(fp);

    return 0;

ERROR_MAIN:
    if (ksv)
        ksv_delete(ksv);

    if (fp)
        fclose(fp);

    return 1;
}
