#ifndef KSV_HELP_H
#define KSV_HELP_H

#include "ksv_metadata.h"
#include "print.h"

#define ksv_version(x) { PUTS("%s", KSV_VERSION); }
#define ksv_license(x) { PUTS("%s", KSV_LICENSE); }

#endif  /* KSV_HELP_H */
