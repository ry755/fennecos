const std = @import("std");

pub fn build(b: *std.build.Builder, exe: *std.build.Step.Compile) void {
    const config = Config{};
    exe.addModule("zfat", createModule(b, config));
    link(exe, config);

    b.installArtifact(exe);
}

fn sdkRoot() []const u8 {
    return std.fs.path.dirname(@src().file) orelse ".";
}

fn sdkPath(comptime rel: []const u8) std.Build.LazyPath {
    return .{ .cwd_relative = comptime sdkRoot() ++ rel };
}

pub fn createModule(b: *std.build.Builder, config: Config) *std.build.Module {
    const options = b.addOptions();

    options.addOption(bool, "has_rtc", (config.rtc != .static));

    // inline for (comptime std.meta.fields(Config)) |fld| {
    //     options.addOption(fld.field_type, fld.name, @field(config, fld.name));
    // }

    return b.createModule(.{
        .source_file = sdkPath("/kernel/src/fatfs.zig"),
        .dependencies = &.{
            .{ .name = "config", .module = options.createModule() },
        },
    });
}

pub fn link(exe: *std.build.LibExeObjStep, config: Config) void {
    exe.addCSourceFiles(&.{
        sdkPath("/kernel/src/fatfs/ff.c").cwd_relative,
        sdkPath("/kernel/src/fatfs/ffunicode.c").cwd_relative,
        sdkPath("/kernel/src/fatfs/ffsystem.c").cwd_relative,
        sdkPath("/kernel/src/fatfs/memcmp.c").cwd_relative,
        sdkPath("/kernel/src/fatfs/memcpy.c").cwd_relative,
        sdkPath("/kernel/src/fatfs/memmove.c").cwd_relative,
        sdkPath("/kernel/src/fatfs/memset.c").cwd_relative,
        sdkPath("/kernel/src/fatfs/strcat.c").cwd_relative,
        sdkPath("/kernel/src/fatfs/strchr.c").cwd_relative,
        sdkPath("/kernel/src/fatfs/strcmp.c").cwd_relative,
        sdkPath("/kernel/src/fatfs/strcpy.c").cwd_relative,
        sdkPath("/kernel/src/fatfs/strlen.c").cwd_relative,
    }, &.{"-std=c99"});
    exe.addIncludePath(sdkPath("/kernel/src/fatfs"));
    // exe.linkLibC();

    inline for (comptime std.meta.fields(Config)) |fld| {
        addConfigField(exe, config, fld.name);
    }

    switch (config.volumes) {
        .count => |count| {
            exe.defineCMacro("FF_VOLUMES", exe.step.owner.fmt("{d}", .{count}));
            exe.defineCMacro("FF_STR_VOLUME_ID", "0");
        },
        .named => |strings| {
            var list = std.ArrayList(u8).init(exe.step.owner.allocator);
            for (strings) |name| {
                if (list.items.len > 0) {
                    list.appendSlice(", ") catch @panic("out of memory");
                }
                list.writer().print("\"{}\"", .{
                    std.fmt.fmtSliceHexUpper(name),
                }) catch @panic("out of memory");
            }

            exe.defineCMacro("FF_VOLUMES", exe.step.owner.fmt("{d}", .{strings.len}));
            exe.defineCMacro("FF_STR_VOLUME_ID", "1");
            exe.defineCMacro("FF_VOLUME_STRS", list.items);
        },
    }

    switch (config.sector_size) {
        .static => |size| {
            const str = exe.step.owner.fmt("{d}", .{@intFromEnum(size)});
            exe.defineCMacro("FF_MIN_SS", str);
            exe.defineCMacro("FF_MAX_SS", str);
        },
        .dynamic => |range| {
            exe.defineCMacro("FF_MIN_SS", exe.step.owner.fmt("{d}", .{@intFromEnum(range.minimum)}));
            exe.defineCMacro("FF_MAX_SS", exe.step.owner.fmt("{d}", .{@intFromEnum(range.maximum)}));
        },
    }

    switch (config.rtc) {
        .dynamic => exe.defineCMacro("FF_FS_NORTC", "0"),
        .static => |date| {
            exe.defineCMacro("FF_FS_NORTC", "1");
            exe.defineCMacro("FF_NORTC_MON", exe.step.owner.fmt("{d}", .{date.month.numeric()}));
            exe.defineCMacro("FF_NORTC_MDAY", exe.step.owner.fmt("{d}", .{date.day}));
            exe.defineCMacro("FF_NORTC_YEAR", exe.step.owner.fmt("{d}", .{date.year}));
        },
    }
}

fn addConfigField(exe: *std.build.LibExeObjStep, config: Config, comptime field_name: []const u8) void {
    const value = @field(config, field_name);
    const Type = @TypeOf(value);
    const type_info = @typeInfo(Type);

    const str_value: []const u8 = if (Type == VolumeKind or Type == SectorSize or Type == RtcConfig)
        return // we don't emit these automatically
    else if (type_info == .Enum)
        exe.step.owner.fmt("{d}", .{@intFromEnum(value)})
    else if (type_info == .Int)
        exe.step.owner.fmt("{d}", .{value})
    else if (Type == bool)
        exe.step.owner.fmt("{d}", .{@intFromBool(value)})
    else if (Type == []const u8)
        value
    else {
        @compileError("Unsupported config type: " ++ @typeName(Type));
    };

    const macro_name = @field(macro_names, field_name);

    exe.defineCMacro(macro_name, str_value);
}

pub const Config = struct {
    read_only: bool = false,
    minimize: MinimizeLevel = .default,
    find: bool = false,
    mkfs: bool = false,
    fastseek: bool = false,
    expand: bool = false,
    chmod: bool = false,
    label: bool = false,
    forward: bool = false,
    strfuncs: StringFuncConfig = .disabled,
    printf_lli: bool = false,
    printf_float: bool = false,
    strf_encoding: StrfEncoding = .oem,
    max_long_name_len: u8 = 255,
    code_page: CodePage = .us,
    long_file_name: bool = true,
    long_file_name_encoding: LfnEncoding = .oem,
    long_file_name_buffer_size: u32 = 255,
    short_file_name_buffer_size: u32 = 12,
    relative_path_api: RelativePathApi = .disabled,
    volumes: VolumeKind = .{ .count = 1 },
    sector_size: SectorSize = .{ .static = .@"512" },
    multi_partition: bool = false,
    lba64: bool = false,
    min_gpt_sectors: u32 = 0x10000000,
    use_trim: bool = false,
    tiny: bool = false,
    exfat: bool = false,
    rtc: RtcConfig = .{ .static = .{ .day = 1, .month = .jan, .year = 2024 } },
    filesystem_trust: Trust = .trust_all,
    lock: u32 = 0,
    reentrant: bool = false,

    sync_type: []const u8 = "HANDLE", // default to windows
    timeout: u32 = 1000,
};

pub const Trust = enum(u2) {
    // bit0=0: Use free cluster count in the FSINFO if available.
    // bit0=1: Do not trust free cluster count in the FSINFO.
    // bit1=0: Use last allocated cluster number in the FSINFO if available.
    // bit1=1: Do not trust last allocated cluster number in the FSINFO.

    trust_all = 0,
    trust_last_sector = 0b01,
    trust_free_clusters = 0b10,
    scan_all = 0b11,
};

pub const RtcConfig = union(enum) {
    dynamic,
    static: struct {
        day: u32,
        month: std.time.epoch.Month,
        year: u32,
    },
};

pub const VolumeKind = union(enum) {
    count: u5, // 1 … 10
    named: []const []const u8,
};

pub const SectorOption = enum(u32) {
    @"512" = 512,
    @"1024" = 1024,
    @"2048" = 2048,
    @"4096" = 4096,
};

pub const SectorSize = union(enum) {
    static: SectorOption,
    dynamic: struct {
        minimum: SectorOption,
        maximum: SectorOption,
    },
};

pub const RelativePathApi = enum(u2) {
    disabled = 0,
    enabed = 1,
    enabled_with_getcwd = 2,
};

pub const LfnEncoding = enum(u2) {
    oem = 0, // ANSI/OEM in current CP (TCHAR = char)
    utf16 = 1, // Unicode in UTF-16 (TCHAR = WCHAR)
    utf8 = 2, // Unicode in UTF-8 (TCHAR = char)
    utf32 = 3, // Unicode in UTF-32 (TCHAR = DWORD)
};

pub const StrfEncoding = enum(u2) {
    oem = 0,
    utf16_le = 1,
    utf16_be = 2,
    utf8 = 3,
};

pub const StringFuncConfig = enum(u2) {
    disabled = 0,
    enabled = 1,
    enabled_with_crlf = 2,
};

pub const MinimizeLevel = enum(u2) {
    default = 0,
    no_advanced = 1,
    no_dir_iteration = 2,
    no_lseek = 3,
};

const macro_names = struct {
    pub const read_only = "FF_FS_READONLY";
    pub const minimize = "FF_FS_MINIMIZE";
    pub const find = "FF_USE_FIND";
    pub const mkfs = "FF_USE_MKFS";
    pub const fastseek = "FF_USE_FASTSEEK";
    pub const expand = "FF_USE_EXPAND";
    pub const chmod = "FF_USE_CHMOD";
    pub const label = "FF_USE_LABEL";
    pub const forward = "FF_USE_FORWARD";
    pub const strfuncs = "FF_USE_STRFUNC";
    pub const printf_lli = "FF_PRINT_LLI";
    pub const printf_float = "FF_PRINT_FLOAT";
    pub const strf_encoding = "FF_STRF_ENCODE";
    pub const long_file_name = "FF_USE_LFN";
    pub const max_long_name_len = "FF_MAX_LFN";
    pub const code_page = "FF_CODE_PAGE";
    pub const long_file_name_encoding = "FF_LFN_UNICODE";
    pub const long_file_name_buffer_size = "FF_LFN_BUF";
    pub const short_file_name_buffer_size = "FF_SFN_BUF";
    pub const relative_path_api = "FF_FS_RPATH";
    pub const multi_partition = "FF_MULTI_PARTITION";
    pub const lba64 = "FF_LBA64";
    pub const min_gpt_sectors = "FF_MIN_GPT";
    pub const use_trim = "FF_USE_TRIM";
    pub const tiny = "FF_FS_TINY";
    pub const exfat = "FF_FS_EXFAT";
    pub const filesystem_trust = "FF_FS_NOFSINFO";
    pub const lock = "FF_FS_LOCK";
    pub const reentrant = "FF_FS_REENTRANT";
    pub const timeout = "FF_FS_TIMEOUT";
    pub const sync_type = "FF_SYNC_t";
};

pub const CodePage = enum(c_int) {
    dynamic = 0, // Include all code pages above and configured by f_setcp()

    us = 437,
    arabic_ms = 720,
    greek = 737,
    kbl = 771,
    baltic = 775,
    latin_1 = 850,
    latin_2 = 852,
    cyrillic = 855,
    turkish = 857,
    portuguese = 860,
    icelandic = 861,
    hebrew = 862,
    canadian_french = 863,
    arabic_ibm = 864,
    nordic = 865,
    russian = 866,
    greek_2 = 869,
    japanese_dbcs = 932,
    simplified_chinese_dbcs = 936,
    korean_dbcs = 949,
    traditional_chinese_dbcs = 950,
};
