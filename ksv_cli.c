#include <stdio.h>
#include "ksv.h"
#include "print.h"

int main(int argc, char *argv[])
{
    if (argc < 2) {
        PUTERR("No valid input");
        return 1;
    }

    FILE *fp = NULL;
    ksv_t *ksv = NULL;

    /* Refactor it later. */
    const char *path = argv[1];

    fp = fopen(path, "r");
    if (!fp) {
        PUTERR("Failed to open file at %s", path);
        return 1;
    }

    ksv = ksv_new_default();
    if (!ksv)
        goto ERROR_KSV;

    if (!ksv_load_stream_with_header_strictly(ksv, fp))
        goto ERROR_KSV;

    ksv_delete(ksv);
    fclose(fp);

    return 0;

ERROR_KSV:
    if (ksv)
        ksv_delete(ksv);

    if (fp)
        fclose(fp);

    return 1;
}
