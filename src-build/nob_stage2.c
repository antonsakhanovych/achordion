#include <stdbool.h>
#include <string.h>

#define NOB_IMPLEMENTATION
#include "../nob.h"
#include "../build/config.h"

#if defined(ACHORDION_TARGET_LINUX_X11)
#define TARGET_IMPLEMENTATION "nob_linux_x11.c"
#elif defined(ACHORDION_TARGET_LINUX_WAYLAND)
#define TARGET_IMPLEMENTATION "nob_linux_wayland.c"
#elif defined(ACHORDION_TARGET_MACOS)
#define TARGET_IMPLEMENTATION "nob_macos.c"
#else
#error "No achordion target defined. Check your ./build/config.h"
#endif

void log_available_subcommands(const char* program, Nob_Log_Level level)
{
    nob_log(level, "Usage: %s [subcommand]\n");
    nob_log(level, "Subcommands:\n");
    nob_log(level, "\tbuild(default):\n");
    nob_log(level, "\thelp\n");
}

int main(int argc, char** argv)
{
    nob_log(NOB_INFO, "--- Stage 2 ---");
    nob_log(NOB_INFO, "Target implementation: %s", TARGET_IMPLEMENTATION);
    const char* program = nob_shift_args(&argc, &argv);
    const char* subcommand = NULL;
    if(argc <= 0) {
        subcommand = "build";
    } else {
        subcommand = nob_shift_args(&argc, &argv);
    }
    if(strcmp(subcommand, "build") == 0) {
        // Build step3
        Nob_Cmd cmd = {0};
        const char* stage3_binary = "./build/nob_stage3";
        nob_cmd_append(&cmd, NOB_REBUILD_URSELF(stage3_binary, "./src-build/nob_stage3.c" ));
        nob_cmd_append(&cmd, "./src-build/"TARGET_IMPLEMENTATION);
        if(!nob_cmd_run_sync(cmd)) return 1;
        cmd.count = 0;
        nob_cmd_append(&cmd, stage3_binary);
        nob_log(NOB_INFO, "--- Stage 3 ---");
        if(!nob_cmd_run_sync(cmd)) return 1;
    } else if(strcmp(subcommand, "help") == 0) {
        log_available_subcommands(subcommand, NOB_INFO);
    } else {
        nob_log(NOB_ERROR, "Unknown subcommand: %s\n", subcommand);
        log_available_subcommands(subcommand, NOB_ERROR);
    }
    return 0;
}
