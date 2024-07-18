#define NOB_IMPLEMENTATION
#include "../nob.h"
#define RAYLIB_MODULES_DECLARATION
#include "nob_compile.h"

const char* ACHORDION_TARGET_NAME = "linux-wayland";

bool build_achordion(void)
{
    Nob_Cmd cmd = {0};
    nob_cmd_append(&cmd, "cc");
    nob_cmd_append(&cmd, "-Wall", "-Wextra", "-ggdb");
    nob_cmd_append(&cmd, "-I./raylib/raylib-"RAYLIB_VERSION"/src/");
    nob_cmd_append(&cmd, "-o", "./build/achordion");
    nob_cmd_append(&cmd, "./src/achordion.c");
    nob_cmd_append(&cmd,
                   nob_temp_sprintf("-L./build/raylib/%s", ACHORDION_TARGET_NAME));
#ifndef SHARED
    nob_cmd_append(&cmd, "-l:libraylib.a");
#else
    nob_cmd_append(&cmd, "-l:libraylib.so");
    nob_cmd_append(&cmd, "-fPIC");
#endif
    nob_cmd_append(&cmd, "-lm", "-lpthread", "-ldl");
    if(!nob_cmd_run_sync(cmd)) return false;
    return true;
}

static const char* wayland_protocols[] = {
    "wayland.xml",
    "xdg-shell.xml",
    "xdg-decoration-unstable-v1.xml",
    "viewporter.xml",
    "relative-pointer-unstable-v1.xml",
    "pointer-constraints-unstable-v1.xml",
    "fractional-scale-v1.xml",
    "xdg-activation-v1.xml",
    "idle-inhibit-unstable-v1.xml",
};
static const char* wayland_basenames[] = {
    "wayland-client-protocol",
    "xdg-shell-client-protocol",
    "xdg-decoration-unstable-v1-client-protocol",
    "viewporter-client-protocol",
    "relative-pointer-unstable-v1-client-protocol",
    "pointer-constraints-unstable-v1-client-protocol",
    "fractional-scale-v1-client-protocol",
    "xdg-activation-v1-client-protocol",
    "idle-inhibit-unstable-v1-client-protocol",
};

void wayland_do(Nob_Cmd* cmd, const char* output_type, const char* protocol_path, const char* output_path)
{
    nob_cmd_append(cmd, "wayland-scanner");
    nob_cmd_append(cmd, output_type);
    nob_cmd_append(cmd, protocol_path);
    nob_cmd_append(cmd, output_path);
}


bool build_raylib(void)
{
    bool result = true;

    const char* wl_input_dir = "./raylib/raylib-"RAYLIB_VERSION"/src/external/glfw/deps/wayland";
    if(!nob_mkdir_if_not_exists("./build/raylib")) {
        nob_return_defer(false);
    }
    const char* build_path = nob_temp_sprintf("./build/raylib/%s", ACHORDION_TARGET_NAME);
    if(!nob_mkdir_if_not_exists(build_path)) {
        nob_return_defer(false);
    }

    const char* wl_output_dir = nob_temp_sprintf("%s/wayland", build_path);
    if(!nob_mkdir_if_not_exists(wl_output_dir)) {
        nob_return_defer(false);
    }
    const char* wl_header_dir = nob_temp_sprintf("%s/client-headers", wl_output_dir);
    if(!nob_mkdir_if_not_exists(wl_header_dir)) {
        nob_return_defer(false);
    }

    const char* wl_code_dir= nob_temp_sprintf("%s/private-code", wl_output_dir);
    if(!nob_mkdir_if_not_exists(wl_code_dir)) {
        nob_return_defer(false);
    }

    Nob_Cmd cmd = {0};
    Nob_File_Paths object_files = {0};
    Nob_Procs procs = {0};

    nob_log(NOB_INFO, "Generating wayland files");
    // Generate wayland files
    for(size_t i = 0; i < NOB_ARRAY_LEN(wayland_protocols); ++i) {
        const char* input_path = nob_temp_sprintf("%s/%s", wl_input_dir, wayland_protocols[i]);
        const char* output_header = nob_temp_sprintf("%s/%s.h", wl_header_dir, wayland_basenames[i]);
        const char* output_code = nob_temp_sprintf("%s/%s-code.h", wl_code_dir, wayland_basenames[i]);
        // Generate headers
        if(nob_needs_rebuild(output_header, &input_path, 1)) {
            cmd.count = 0;
            wayland_do(&cmd, "client-header", input_path, output_header);
            Nob_Proc proc = nob_cmd_run_async(cmd);
            nob_da_append(&procs, proc);
        }
        if(nob_needs_rebuild(output_code, &input_path, 1)) {
            cmd.count = 0;
            wayland_do(&cmd, "private-code", input_path, output_code);
            Nob_Proc proc = nob_cmd_run_async(cmd);
            nob_da_append(&procs, proc);
        }
    }
    if(!nob_procs_wait(procs)) nob_return_defer(false);

    nob_log(NOB_INFO, "Compiling raylib");
    procs.count = 0;
    // Compile raylib
    for(size_t i = 0; i < NOB_ARRAY_LEN(raylib_modules); ++i) {
        const char* input_path = nob_temp_sprintf("./raylib/raylib-"RAYLIB_VERSION"/src/%s.c", raylib_modules[i]);
        const char* output_path = nob_temp_sprintf("%s/%s.o", build_path, raylib_modules[i]);
        nob_da_append(&object_files, output_path);
        if(nob_needs_rebuild(output_path, &input_path, 1)) {
            cmd.count = 0;
            nob_cmd_append(&cmd, "cc");
            nob_cmd_append(&cmd, "-ggdb");
            nob_cmd_append(&cmd, "-fPIC");
            nob_cmd_append(&cmd, "-D_GLFW_WAYLAND", "-DPLATFORM_DESKTOP");
            nob_cmd_append(&cmd, "-I./raylib/raylib-"RAYLIB_VERSION"/src/external/glfw/include");
            nob_cmd_append(&cmd, "-I",  wl_header_dir);
            nob_cmd_append(&cmd, "-I", wl_code_dir);
            nob_cmd_append(&cmd, "-c", input_path);
            nob_cmd_append(&cmd, "-o", output_path);
            Nob_Proc proc = nob_cmd_run_async(cmd);
            nob_da_append(&procs, proc);
        }
    }
    cmd.count = 0;
    if(!nob_procs_wait(procs)) nob_return_defer(false);

#ifndef SHARED
    const char* libraylib_path = nob_temp_sprintf("%s/libraylib.a", build_path);
    if(nob_needs_rebuild(libraylib_path, object_files.items, object_files.count)) {
        nob_cmd_append(&cmd, "ar", "-crs", libraylib_path);
        for(size_t i = 0; i < object_files.count; ++i) {
            nob_cmd_append(&cmd, object_files.items[i]);
        }
        if(!nob_cmd_run_sync(cmd)) nob_return_defer(false);
    }
#else
    const char* libraylib_path = nob_temp_sprintf("%s/libraylib.so", build_path);
    if(nob_needs_rebuild(libraylib_path, object_files.items, object_files.count)) {
        nob_cmd_append(&cmd, "cc");
        nob_cmd_append(&cmd, "-shared");
        nob_cmd_append(&cmd, "-o", libraylib_path);
        for(size_t i = 0; i < object_files.count; ++i) {
            nob_cmd_append(&cmd, object_files.items[i]);
        }
        if(!nob_cmd_run_sync(cmd)) nob_return_defer(false);
    }
#endif

defer:
    nob_cmd_free(cmd);
    nob_da_free(object_files);
    return result;
}
