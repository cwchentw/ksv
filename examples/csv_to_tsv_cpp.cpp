#include <cstdio>
#include <iostream>
#include "ksv.hpp"

int main(int argc, char *argv[])
{
    if (argc < 2) {
        std::cerr << "No input sheet" << std::endl;
        return 1;
    }

    FILE *fp = NULL;
    KSV *ksv = NULL;
    std::string field;

    char *path = argv[1];

    /* Open a CSV sheet. */
    fp = fopen(path, "r");
    if (!fp) {
        std::cerr << "Unable to read file" << std::endl;
        return 1;
    }

    /* Create a ksv object. */
    try {
        ksv = new KSV();
    }
    catch (std::exception &ex) {
        goto ERROR_MAIN;
    }

    /* Load the header row of the sheet. */
    if (!ksv->load_header(fp)) {
        std::cerr << "Failed to load sheet header" << std::endl;
        goto ERROR_MAIN;
    }

    /* Iterate over the header(s) of the header row. */
    field = ksv->next_header();
    while (!field.empty()) {
        std::cout << field << "\t";

        field = ksv->next_header();   
    }
    std::cout << "\b" << std::endl;

    /* Iterate over the sheet. */
    while (!feof(fp)) {
        /* Load one row from the sheet. */
        if (!ksv->load_record(fp)) {
            std::cerr << "Failed to load a sheet record" << std::endl;
            goto ERROR_MAIN;
        }

        /* Re-init the state of the ksv object. */
        ksv->restart();

        /* Iterate over the cell(s) of the row. */
        field = ksv->next_data_by_row();
        while (!field.empty()) {
            std::cout << field << "\t";

            field = ksv->next_data_by_row();
        }
        std::cout << "\b" << std::endl;
    }

    /* Release system resources. */
    delete ksv;
    fclose(fp);

    return 0;

ERROR_MAIN:
    if (ksv)
        delete ksv;

    if (fp)
        fclose(fp);

    return 1;
}
