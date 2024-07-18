#ifndef __TARGET_COMPILATION
#define __TARGET_COMPILATION

#include <stdbool.h>
#define RAYLIB_VERSION "5.0"

extern const char* ACHORDION_TARGET_NAME;

#ifdef RAYLIB_MODULES_DECLARATION
const char *raylib_modules[] = {
    "rcore",
    "raudio",
    "rglfw",
    "rmodels",
    "rshapes",
    "rtext",
    "rtextures",
    "utils",
};
#endif

bool build_raylib(void);
bool build_achordion(void);

#endif // __TARGET_COMPILATION
