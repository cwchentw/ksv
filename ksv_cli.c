#include <stdio.h>
#include "ksv.h"
#include "ksv_argument.h"
#include "print.h"

int main(int argc, char *argv[])
{
    if (argc < 2) {
        PUTERR("No valid input");
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

    ksv_delete(ksv);
    fclose(fp);
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
