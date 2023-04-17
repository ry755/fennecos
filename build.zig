const std = @import("std");
const Builder = std.build.Builder;

pub fn build(b: *Builder) !void {
    // define a freestanding x86 cross-compilation target
    var target: std.zig.CrossTarget = .{
        .cpu_arch = .x86,
        .os_tag = .freestanding,
        .abi = .none,
    };

    // disable CPU features that require additional initialization
    // like MMX, SSE/2 and AVX. that requires us to enable the soft-float feature
    const Features = std.Target.x86.Feature;
    target.cpu_features_sub.addFeature(@enumToInt(Features.mmx));
    target.cpu_features_sub.addFeature(@enumToInt(Features.sse));
    target.cpu_features_sub.addFeature(@enumToInt(Features.sse2));
    target.cpu_features_sub.addFeature(@enumToInt(Features.avx));
    target.cpu_features_sub.addFeature(@enumToInt(Features.avx2));
    target.cpu_features_add.addFeature(@enumToInt(Features.soft_float));

    // build the kernel itself
    const optimize = b.standardOptimizeOption(.{});
    const kernel = b.addExecutable(.{
        .name = "fennecos.elf",
        .root_source_file = .{ .path = "kernel/src/main.zig" },
        .target = target,
        .optimize = optimize,
    });
    kernel.code_model = .kernel;
    kernel.setLinkerScriptPath(.{ .path = "kernel/linker.ld" });
    kernel.addAssemblyFile("kernel/src/boot.s");
    kernel.override_dest_dir = .{ .custom = "../base_image/boot/" };
    b.installArtifact(kernel);
}
