#include "nob_compile.h"
#include <stdio.h>


int main(int argc, char** argv)
{
    if(!build_raylib()) return 1;
    if(!build_achordion()) return 1;
    return 0;
}
