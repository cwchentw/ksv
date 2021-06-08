#include <stdio.h>
#include "ksv_help.h"
#include "ksv_metadata.h"

#ifndef END_OF_LINE
    #if _WIN32
        #define  END_OF_LINE  "\r\n"
    #elif __unix__ || __unix || unix || __APPLE__
        #define  END_OF_LINE  "\n"
    #else
        #error  "Unsupported file"
    #endif
#endif  /* END_OF_LINE */

/*-------1---------2---------3---------4---------5---------6---------7---------8*/
/*345678901234567890123456789012345678901234567890123456789012345678901234567890*/
void ksv_help(FILE *stream)
{
    fprintf(stream, "Usage: %s [command] [sheet.csv]%s", KSV_PROGRAM, END_OF_LINE);
    fprintf(stream, "       %s stats [command] [sheet.csv]%s", KSV_PROGRAM, END_OF_LINE);
    fprintf(stream, "%s", END_OF_LINE);
    fprintf(stream, "Command:%s", END_OF_LINE);
    fprintf(stream, "    version\tShow version info and exit%s", END_OF_LINE);
    fprintf(stream, "    license\tShow license info and exit%s", END_OF_LINE);
    fprintf(stream, "    help\tShow help info and exit%s", END_OF_LINE);
    fprintf(stream, "    width\tShow the width of CSV sheet%s", END_OF_LINE);
    fprintf(stream, "    height\tShow the height of CSV sheet%s", END_OF_LINE);
    fprintf(stream, "    dimension\tShow the dimension of CSV sheet%s", END_OF_LINE);
    fprintf(stream, "    header\tShow the header of CSV sheet%s", END_OF_LINE);
    fprintf(stream, "    table\tShow CSV sheet as console table%s", END_OF_LINE);
    fprintf(stream, "    stats\tBasic statistics for CSV sheet%s", END_OF_LINE);
    fprintf(stream, "%s", END_OF_LINE);
    fprintf(stream, "Stats command:%s", END_OF_LINE);
    fprintf(stream, "    deviation\tDeviation of numerical data%s", END_OF_LINE);
    fprintf(stream, "    quartile\tQuartiles of numerical data%s", END_OF_LINE);
    fprintf(stream, "    quintile\tQuintiles of numerical data%s", END_OF_LINE);
}
