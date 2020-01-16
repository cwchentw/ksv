#include <stdio.h>
#include "ksv.h"
#include "ksv_argument.h"
#include "ksv_help.h"
#include "ksv_metadata.h"
#include "print.h"

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
    const char *path = argv[1];

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
