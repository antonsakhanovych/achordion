#include <stdbool.h>
#define NOB_IMPLEMENTATION
#include "./nob.h"

#define CONFIG_PATH "./build/config.h"

bool generate_default_config(const char* file_path);

int main(int argc, char** argv)
{
    NOB_GO_REBUILD_URSELF(argc, argv);
    const char *program = nob_shift_args(&argc, &argv);
    nob_log(NOB_INFO, "--- Stage 1 ---");
    if(!nob_mkdir_if_not_exists("build")) return 1;
    int config_exists = nob_file_exists(CONFIG_PATH);
    if(config_exists < 0) return 1;
    if(config_exists == 0) {
        if(!generate_default_config(CONFIG_PATH)) return 1;
    } else {
        nob_log(NOB_ERROR, "file %s already exists\n", CONFIG_PATH);
    }
    Nob_Cmd cmd = {0};
    const char* stage2_binary = "./build/nob_stage2";
    nob_cmd_append(&cmd, NOB_REBUILD_URSELF(stage2_binary, "./src-build/nob_stage2.c"));
    if(!nob_cmd_run_sync(cmd)) return 1;
    cmd.count = 0;
    nob_cmd_append(&cmd, stage2_binary);
    if(!nob_cmd_run_sync(cmd)) return 1;

    return 0;
}

typedef struct {
    const char* macro;
    bool enabled_default;
} Target_Flag;

static Target_Flag target_flags[] = {
    {
        .macro = "ACHORDION_TARGET_LINUX_X11",
#if (defined(linux) || defined(__linux) || defined(__linux__)) && !defined(WAYLAND)
        .enabled_default = true,
#else
        .enabled_default = false,
#endif
    },

    {
        .macro = "ACHORDION_TARGET_LINUX_WAYLAND",
#if (defined(linux) || defined(__linux) || defined(__linux__)) && defined(WAYLAND)
        .enabled_default = true,
#else
        .enabled_default = false,
#endif
    },
    {
        .macro = "ACHORDION_TARGET_MACOS",
#if defined(__APPLE__) || defined(__MACH__)
        .enabled_default = true,
#else
        .enabled_default = false,
#endif
    },
};

bool generate_default_config(const char* file_path)
{
    nob_log(NOB_INFO, "Generating %s", file_path);
    FILE* f = fopen(file_path,"wb");
    if(f == NULL) {
        nob_log(NOB_ERROR, "Could not generate %s: %s", file_path, strerror(errno));
        return false;
    }
    fprintf(f, "//// Build target pick one only!\n");
    for(size_t i = 0; i < NOB_ARRAY_LEN(target_flags); ++i) {
        if(target_flags[i].enabled_default) {
            fprintf(f, "#define %s\n", target_flags[i].macro);
        } else {
            fprintf(f, "// #define %s\n", target_flags[i].macro);
        }
    }
    fprintf(f, "\n");
    fclose(f);
    return true;
}
