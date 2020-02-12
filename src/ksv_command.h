#ifndef KSV_COMMAND_H
#define KSV_COMMAND_H

typedef unsigned char KSV_COMMAND_TYPE;

#define  KSV_COMMAND_UNKNOWN    0
#define  KSV_COMMAND_VERSION    1
#define  KSV_COMMAND_LICENSE    2
#define  KSV_COMMAND_HELP       3
#define  KSV_COMMAND_WIDTH      4
#define  KSV_COMMAND_HEIGHT     5
#define  KSV_COMMAND_DIMENSION  6
#define  KSV_COMMAND_HEADER     7
#define  KSV_COMMAND_SHOW       8
#define  KSV_COMMAND_STATS      9

#define  KSV_STATS_COMMAND_DEVIATION  1
#define  KSV_STATS_COMMAND_QUARTILES  2
#define  KSV_STATS_COMMAND_QUINTILES  3

#define IS_KSV_COMMAND_EQUAL(a, b) (!((a) ^ (b)))

#endif  /* KSV_COMMAND_H */
