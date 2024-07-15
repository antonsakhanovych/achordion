const std = @import("std");

pub fn build(b: *std.Build) !void {
    const target = b.standardTargetOptions(.{});
    const optimize = b.standardOptimizeOption(.{});
    const exe = b.addExecutable(.{
        .name = "achordion",
        .target = target,
        .optimize = optimize,
        .link_libc = true,
    });
    exe.addCSourceFiles(.{
        .root = b.path("src"),
        .files = &[_][]const u8{"main.c"},
        .flags = &[_][]const u8{ "-std=c99", "-Wall", "-Wextra" },
    });

    const defaults = Options{};

    const raylib = b.dependency("raylib", .{
        .target = target,
        .optimize = optimize,
        .shared = b.option(bool, "shared", "Compile as shared library") orelse defaults.shared,
        .linux_display_backend = b.option(LinuxDisplayBackend, "linux_display_backend", "Linux display backend to use") orelse defaults.linux_display_backend,
        .opengl_version = b.option(OpenglVersion, "opengl_version", "OpenGL version to use") orelse defaults.opengl_version,
    });
    exe.linkLibrary(raylib.artifact("raylib"));
    b.installArtifact(exe);

    const run_cmd = b.addRunArtifact(exe);
    const run_step = b.step("run", "Run the program");
    if (b.args) |args| {
        run_cmd.addArgs(args);
    }
    run_step.dependOn(&run_cmd.step);
}

pub const Options = struct {
    shared: bool = false,
    linux_display_backend: LinuxDisplayBackend = .Both,
    opengl_version: OpenglVersion = .auto,

    raygui_dependency_name: []const u8 = "raygui",
};

pub const OpenglVersion = enum {
    auto,
    gl_1_1,
    gl_2_1,
    gl_3_3,
    gl_4_3,
    gles_2,
    gles_3,
};

pub const LinuxDisplayBackend = enum {
    X11,
    Wayland,
    Both,
};
