#include "version.ver"

#define _STR(x) #x
#define STR(x) _STR(x)

#define VERSION_NUMBER VERSION_MAJOR,VERSION_MINOR,VERSION_YEAR,VERSION_BUILD
#define VERSION_STRING STR(VERSION_MAJOR) "." STR(VERSION_MINOR) "." \
                                 STR(VERSION_YEAR) "." STR(VERSION_BUILD)
#define VERSION_COMPANY ""
#define VERSION_COPYRIGHT "(C) Thorlabs 2020"
#define VERSION_TRADEMARK ""

