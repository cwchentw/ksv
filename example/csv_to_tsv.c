#include <stdio.h>
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

    fp = fopen(path, "r");
    if (!fp) {
        PUTERR("Unable to read file");
        return 1;
    }

    ksv = ksv_new_default();
    if (!ksv) {
        PUTERR("Failed to create ksv object");
        goto ERROR_MAIN;
    }

    if (KSV_SUCCESS != ksv_load_header(ksv, fp)) {
        PUTERR("Failed to load header");
        goto ERROR_MAIN;
    }

    /* Load header and convert it. */
    char *field = ksv_next_header(ksv);
    while (field) {
        PRINT("%s\t", field);

        field = ksv_next_header(ksv);
    }
    PUTS("\b");

    while (!feof(fp)) {
        if (KSV_SUCCESS != ksv_load_record(ksv, fp)) {
            PUTERR("Failed to load record");
            goto ERROR_MAIN;
        }

        ksv_restart(ksv);

        field = ksv_next_data_by_row(ksv);
        while (field) {
            PRINT("%s\t", field);

            field = ksv_next_data_by_row(ksv);
        }
        PUTS("\b");
    }

    fclose(fp);

    return 0;

ERROR_MAIN:
    if (ksv)
        ksv_delete(ksv);

    if (fp)
        fclose(fp);

    return 1;
}