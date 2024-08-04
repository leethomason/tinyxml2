const std = @import("std");

pub fn build(b: *std.Build) !void {
    const target = b.standardTargetOptions(.{});
    const optimize = b.standardOptimizeOption(.{});

    const lib = b.addStaticLibrary(.{
        .name = "tinyxml2",
        .target = target,
        .optimize = optimize,
    });
    lib.linkLibCpp();

    lib.addCSourceFiles(.{
        .files = &.{"./tinyxml2.cpp"},
    });
    lib.installHeader(b.path("./tinyxml2.h"), "tinyxml2.h");
    b.installArtifact(lib);

    const test_exe = b.addExecutable(.{
        .name = "xmltest",
        .target = target,
        .optimize = optimize,
    });

    test_exe.linkLibrary(lib);
    test_exe.addCSourceFile(.{ .file = b.path("./xmltest.cpp") });

    const run_tests = b.addRunArtifact(test_exe);
    const test_step = b.step("test", "Build and run xmltest.");
    test_step.dependOn(&run_tests.step);
}
