const std = @import("std");
const config = @import("config");
const c = @cImport({
    @cInclude("ff.h");
    @cInclude("diskio.h");
});
const logger = std.log.scoped(.fatfs);

pub const volume_count = c.FF_VOLUMES;

pub const PathChar = c.TCHAR;
pub const LBA = c.LBA_t;
pub const FileSize = c.FSIZE_t;
pub const Path = [:0]const PathChar;

pub const WORD = c.WORD;
pub const DWORD = c.DWORD;

pub const MkDirError = ErrorSet(&.{ FR_DISK_ERR, FR_INT_ERR, FR_NOT_READY, FR_NO_PATH, FR_INVALID_NAME, FR_DENIED, FR_EXIST, FR_WRITE_PROTECTED, FR_INVALID_DRIVE, FR_NOT_ENABLED, FR_NO_FILESYSTEM, FR_TIMEOUT, FR_NOT_ENOUGH_CORE });
pub fn mkdir(path: Path) MkDirError.Error!void {
    try MkDirError.throw(api.mkdir(path.ptr));
}

pub const UnlinkError = ErrorSet(&.{ FR_DISK_ERR, FR_INT_ERR, FR_NOT_READY, FR_NO_FILE, FR_NO_PATH, FR_INVALID_NAME, FR_DENIED, FR_WRITE_PROTECTED, FR_INVALID_DRIVE, FR_NOT_ENABLED, FR_NO_FILESYSTEM, FR_TIMEOUT, FR_LOCKED, FR_NOT_ENOUGH_CORE });
pub fn unlink(path: Path) UnlinkError.Error!void {
    try UnlinkError.throw(api.unlink(path.ptr));
}

pub const RenameError = ErrorSet(&.{ FR_DISK_ERR, FR_INT_ERR, FR_NOT_READY, FR_NO_FILE, FR_NO_PATH, FR_INVALID_NAME, FR_EXIST, FR_WRITE_PROTECTED, FR_INVALID_DRIVE, FR_NOT_ENABLED, FR_NO_FILESYSTEM, FR_TIMEOUT, FR_LOCKED, FR_NOT_ENOUGH_CORE });
pub fn rename(old_path: Path, new_path: Path) RenameError.Error!void {
    try RenameError.throw(api.rename(old_path.ptr, new_path.ptr));
}

pub const StatError = ErrorSet(&.{ FR_DISK_ERR, FR_INT_ERR, FR_NOT_READY, FR_NO_FILE, FR_NO_PATH, FR_INVALID_NAME, FR_INVALID_DRIVE, FR_NOT_ENABLED, FR_NO_FILESYSTEM, FR_TIMEOUT, FR_NOT_ENOUGH_CORE });
pub fn stat(path: Path) StatError.Error!FileInfo {
    var res: c.FILINFO = undefined;
    try StatError.throw(api.stat(path.ptr, &res));
    return FileInfo.fromFILINFO(res);
}

pub const ChmodError = ErrorSet(&.{ FR_DISK_ERR, FR_INT_ERR, FR_NOT_READY, FR_NO_FILE, FR_NO_PATH, FR_INVALID_NAME, FR_WRITE_PROTECTED, FR_INVALID_DRIVE, FR_NOT_ENABLED, FR_NO_FILESYSTEM, FR_TIMEOUT, FR_NOT_ENOUGH_CORE });
pub fn chmod(path: Path, attributes: u8, mask: u8) ChmodError.Error!void {
    try ChmodError.throw(api.unlink(path.ptr, attributes, mask));
}

pub const UTimeError = ErrorSet(&.{ FR_DISK_ERR, FR_INT_ERR, FR_NOT_READY, FR_NO_FILE, FR_NO_PATH, FR_INVALID_NAME, FR_WRITE_PROTECTED, FR_INVALID_DRIVE, FR_NOT_ENABLED, FR_NO_FILESYSTEM, FR_TIMEOUT, FR_NOT_ENOUGH_CORE });
pub fn utime(path: Path, file_info: c.FILINFO) UTimeError.Error!void {
    try UTimeError.throw(api.unlink(path.ptr, &file_info));
}

pub const ChDirError = ErrorSet(&.{ FR_DISK_ERR, FR_INT_ERR, FR_NOT_READY, FR_NO_PATH, FR_INVALID_NAME, FR_INVALID_DRIVE, FR_NOT_ENABLED, FR_NO_FILESYSTEM, FR_TIMEOUT, FR_NOT_ENOUGH_CORE });
pub fn chdir(path: Path) ChDirError.Error!void {
    try ChDirError.throw(api.chdir(path.ptr));
}

pub const ChDriveError = ErrorSet(&.{FR_INVALID_DRIVE});
pub fn chdrive(path: Path) ChDriveError.Error!void {
    try ChDriveError.throw(api.chdrive(path.ptr));
}

pub const GetCwdError = ErrorSet(&.{ FR_DISK_ERR, FR_INT_ERR, FR_NOT_READY, FR_NOT_ENABLED, FR_NO_FILESYSTEM, FR_TIMEOUT, FR_NOT_ENOUGH_CORE });
pub fn getcwd(buffer: []PathChar) GetCwdError.Error!Path {
    try GetCwdError.throw(api.getcwd(buffer.ptr, try std.math.cast(c_uint, buffer.len)));
    return std.mem.sliceTo(buffer, 0);
}

pub const DiskFormat = enum(u8) {
    fat = c.FM_FAT,
    fat32 = c.FM_FAT32,
    exfat = c.FM_EXFAT,
    any = c.FM_ANY,
};

pub const FatTables = enum(u8) {
    one = 1,
    two = 2,
};

pub const FormatOptions = struct {
    /// Specifies a combination of FAT type flags, FM_FAT, FM_FAT32, FM_EXFAT and bitwise-or of these three,
    // FM_ANY. FM_EXFAT is ignored when exFAT is not enabled. These flags specify which type of FAT volume
    // to be created. If two or more types are specified, one out of them will be selected depends on the
    // volume size and au_size. The flag FM_SFD specifies to create the volume on the drive in SFD format.
    // The default value is FM_ANY.
    filesystem: DiskFormat,

    /// Specifies number of FAT copies on the FAT/FAT32 volume. Valid value for this member is 1 or 2.
    /// The default value (0) and any invaid value gives 1. If the FAT type is exFAT, this member has no effect.
    fats: FatTables = .one,

    /// Specifies alignment of the volume data area (file allocation pool, usually erase block boundary of flash memory media) in unit of sector. The valid value for this member is between 1 and 32768 inclusive in power of 2. If a zero (the default value) or any invalid value is given, the function obtains the block size from lower layer with disk_ioctl function.
    sector_align: c_uint,

    /// Specifies size of the allocation unit (cluter) in unit of byte. The valid value is power of 2 between
    /// sector size and 128 * sector size inclusive for FAT/FAT32 volume, or up to 16 MB for exFAT volume. If
    /// a zero (default value) or any invalid value is given, the function uses default allocation unit size
    /// depends on the volume size.
    cluster_size: u32 = 0,

    /// Specifies number of root directory entries on the FAT volume. Valid value for this member is up to 32768
    /// and aligned to sector size / 32. The default value (0) and any invaid value gives 512. If the FAT type is
    /// FAT32 or exFAT, this member has no effect.
    rootdir_size: c_uint = 512,

    use_partitions: bool = false,
};

pub const MkfsError = ErrorSet(&.{ FR_DISK_ERR, FR_NOT_READY, FR_WRITE_PROTECTED, FR_INVALID_DRIVE, FR_MKFS_ABORTED, FR_INVALID_PARAMETER, FR_NOT_ENOUGH_CORE });
pub fn mkfs(path: Path, options: FormatOptions, workspace: []u8) MkfsError.Error!void {
    const opts = c.MKFS_PARM{
        .fmt = @intFromEnum(options.filesystem) | if (!options.use_partitions) @as(u8, @intCast(c.FM_SFD)) else 0,
        .n_fat = @intFromEnum(options.fats),
        .@"align" = options.sector_align,
        .au_size = options.cluster_size,
        .n_root = options.rootdir_size,
    };
    try MkfsError.throw(api.mkfs(path.ptr, &opts, workspace.ptr, @as(c_uint, @intCast(@min(workspace.len, std.math.maxInt(c_uint))))));
}

pub const FileSystem = struct {
    const Self = @This();

    raw: c.FATFS,

    pub const MountError = ErrorSet(&.{ FR_INVALID_DRIVE, FR_DISK_ERR, FR_NOT_READY, FR_NOT_ENABLED, FR_NO_FILESYSTEM });
    pub fn mount(self: *Self, drive: Path, force_mount: bool) MountError.Error!void {
        try MountError.throw(api.mount(&self.raw, drive.ptr, @intFromBool(force_mount)));
    }

    pub fn unmount(drive: Path) MountError.Error!void {
        try MountError.throw(api.unmount(drive.ptr));
    }
};

pub const Dir = struct {
    const Self = @This();

    raw: c.DIR,

    pub const OpenDirError = ErrorSet(&.{ FR_DISK_ERR, FR_INT_ERR, FR_NOT_READY, FR_NO_PATH, FR_INVALID_NAME, FR_INVALID_OBJECT, FR_INVALID_DRIVE, FR_NOT_ENABLED, FR_NO_FILESYSTEM, FR_TIMEOUT, FR_NOT_ENOUGH_CORE, FR_TOO_MANY_OPEN_FILES });

    pub fn open(path: Path) OpenDirError.Error!Self {
        var dir = Self{ .raw = undefined };
        try OpenDirError.throw(api.opendir(&dir.raw, path.ptr));
        return dir;
    }

    pub fn close(dir: *Self) void {
        mapGenericError(api.closedir(&dir.raw)) catch |e| {
            logger.err("failed to close directory: {s}", .{@errorName(e)});
        };
        dir.* = undefined;
    }

    pub const ReadDirError = ErrorSet(&.{ FR_DISK_ERR, FR_INT_ERR, FR_INVALID_OBJECT, FR_TIMEOUT, FR_NOT_ENOUGH_CORE });

    pub fn next(dir: *Self) ReadDirError.Error!?FileInfo {
        var res: c.FILINFO = undefined;
        try ReadDirError.throw(api.readdir(&dir.raw, &res));
        if (res.fname[0] == 0)
            return null;
        return FileInfo.fromFILINFO(res);
    }

    pub fn rewind(dir: *Self) ReadDirError.Error!void {
        try ReadDirError.throw(api.readdir(&dir.raw, null));
    }
};

pub const Attributes = struct {
    read_only: bool,
    hidden: bool,
    system: bool,
    archive: bool,
};

pub const FileInfo = struct {
    pub fn fromFILINFO(info: c.FILINFO) FileInfo {
        return FileInfo{
            .size = info.fsize,
            .date = Date.fromFDate(info.fdate),
            .time = Time.fromFTime(info.ftime),
            .kind = if ((info.fattrib & c.AM_DIR) != 0) .Directory else .File,
            .attributes = Attributes{
                .read_only = ((info.fattrib) & c.AM_RDO) != 0,
                .hidden = ((info.fattrib) & c.AM_HID) != 0,
                .system = ((info.fattrib) & c.AM_SYS) != 0,
                .archive = ((info.fattrib) & c.AM_ARC) != 0,
            },
            .name_buffer = info.fname,
            .altname_buffer = if (@hasField(c.FILINFO, "altname")) info.altname else [1]u8{0},
        };
    }

    size: u64,
    date: Date,
    time: Time,
    kind: Kind,
    attributes: Attributes,

    name_buffer: [max_name_len + 1]u8,
    altname_buffer: [max_altname_len + 1]u8,

    pub fn name(self: *const FileInfo) []const u8 {
        return std.mem.sliceTo(&self.name_buffer, 0);
    }

    pub fn altName(self: *const FileInfo) []const u8 {
        return std.mem.sliceTo(&self.altname_buffer, 0);
    }

    const max_name_len = if (@hasDecl(c, "FF_LFN_BUF")) c.FF_LFN_BUF else 12;
    const max_altname_len = if (@hasDecl(c, "FF_SFN_BUF")) c.FF_SFN_BUF else 0;
};

pub const Kind = enum { File, Directory };

pub const Date = struct {
    year: u16,
    month: std.time.epoch.Month,
    day: u8,

    pub fn fromFDate(val: u16) Date {
        return Date{
            .year = 1980 + (val >> 9),
            .month = @as(std.time.epoch.Month, @enumFromInt(@as(u4, @truncate((val >> 5) & 0x0F)))),
            .day = @as(u8, @truncate((val >> 0) & 0x15)),
        };
    }

    pub fn format(date: Date, comptime fmt: []const u8, opt: std.fmt.FormatOptions, writer: anytype) !void {
        _ = fmt;
        _ = opt;
        try writer.print("{d:0>4}-{d:0>2}-{d:0>2}", .{
            date.year,
            @intFromEnum(date.month),
            date.day,
        });
    }
};

pub const Time = struct {
    hour: u8,
    minute: u8,
    second: u8,

    pub fn fromFTime(val: u16) Time {
        return Time{
            .hour = @as(u8, @truncate((val >> 11))),
            .minute = @as(u8, @truncate((val >> 5) & 0x3F)),
            .second = 2 * @as(u8, @truncate((val >> 0) & 0x1F)),
        };
    }

    pub fn format(time: Time, comptime fmt: []const u8, opt: std.fmt.FormatOptions, writer: anytype) !void {
        _ = fmt;
        _ = opt;
        try writer.print("{d:0>2}:{d:0>2}:{d:0>2}", .{
            time.hour,
            time.minute,
            time.second,
        });
    }
};

pub const File = struct {
    const Self = @This();

    raw: c.FIL,

    pub const Mode = enum(u8) {
        open_existing = c.FA_OPEN_EXISTING,
        create_new = c.FA_CREATE_NEW,
        create_always = c.FA_CREATE_ALWAYS,
        open_always = c.FA_OPEN_ALWAYS,
        open_append = c.FA_OPEN_APPEND,
    };

    pub const Access = enum(u8) {
        read_only = c.FA_READ,
        write_only = c.FA_WRITE,
        read_write = c.FA_READ | c.FA_WRITE,
    };

    pub const OpenFlags = struct {
        access: Access = .read_only,
        mode: Mode = .open_existing,
    };

    pub const OpenError = ErrorSet(&.{ FR_DISK_ERR, FR_INT_ERR, FR_NOT_READY, FR_NO_FILE, FR_NO_PATH, FR_INVALID_NAME, FR_DENIED, FR_EXIST, FR_INVALID_OBJECT, FR_WRITE_PROTECTED, FR_INVALID_DRIVE, FR_NOT_ENABLED, FR_NO_FILESYSTEM, FR_TIMEOUT, FR_LOCKED, FR_NOT_ENOUGH_CORE, FR_TOO_MANY_OPEN_FILES });

    pub fn open(path: Path, flags: OpenFlags) OpenError.Error!Self {
        const int_flags = @intFromEnum(flags.mode) | @intFromEnum(flags.access);

        var file = Self{ .raw = undefined };
        try OpenError.throw(api.open(&file.raw, path.ptr, int_flags));
        return file;
    }

    /// Creates a new file. If the file is existing, it will be truncated and overwritten.
    /// File access is read and write.
    pub fn create(path: Path) OpenError.Error!Self {
        return open(path, .{
            .mode = .create_always,
            .access = .read_write,
        });
    }

    /// Opens a file. The function fails if the file is not existing.
    /// File access is read only.
    pub fn openRead(path: Path) OpenError.Error!Self {
        return open(path, .{
            .mode = .open_existing,
            .access = .read_only,
        });
    }

    /// Opens the file if it is existing. If not, a new file will be created.
    /// File access is read and write.
    pub fn openWrite(path: Path) OpenError.Error!Self {
        return open(path, .{
            .mode = .open_always,
            .access = .read_write,
        });
    }

    pub fn close(file: *Self) void {
        mapGenericError(api.close(&file.raw)) catch |e| {
            logger.err("failed to close file: {s}", .{@errorName(e)});
        };
        file.* = undefined;
    }

    pub const SyncError = ErrorSet(&.{ FR_DISK_ERR, FR_INT_ERR, FR_INVALID_OBJECT, FR_TIMEOUT });
    pub fn sync(file: *Self) SyncError.Error!void {
        try SyncError.throw(api.sync(&file.raw));
    }

    pub const TruncateError = ErrorSet(&.{ FR_DISK_ERR, FR_INT_ERR, FR_DENIED, FR_INVALID_OBJECT, FR_TIMEOUT });
    pub fn truncate(file: *Self) TruncateError.Error!void {
        try TruncateError.throw(api.truncate(&file.raw));
    }

    pub const SeekToError = ErrorSet(&.{ FR_DISK_ERR, FR_INT_ERR, FR_INVALID_OBJECT, FR_TIMEOUT });
    pub fn seekTo(file: *Self, offset: FileSize) !void {
        try SeekToError.throw(api.lseek(&file.raw, offset));
    }

    pub const ExpandError = ErrorSet(&.{ FR_DISK_ERR, FR_INT_ERR, FR_INVALID_OBJECT, FR_DENIED, FR_TIMEOUT });
    pub fn expand(file: *Self, new_size: FileSize, force_allocate: bool) !void {
        try ExpandError.throw(api.expand(&file.raw, new_size, @intFromBool(force_allocate)));
    }

    // pub fn forward(self: *Self, streamer: fn([*]const u8,

    pub fn endOfFile(self: Self) bool {
        return (api.eof(&self.raw) != 0);
    }

    pub fn hasError(self: Self) bool {
        return (api.@"error"(&self.raw) != 0);
    }

    pub fn tell(self: Self) FileSize {
        return api.tell(&self.raw);
    }

    pub fn size(self: Self) FileSize {
        return api.size(&self.raw);
    }

    pub fn rewind(self: *Self) !void {
        try self.seekTo(0);
    }

    pub const WriteError = ErrorSet(&.{ error.Overflow, FR_DISK_ERR, FR_INT_ERR, FR_DENIED, FR_INVALID_OBJECT, FR_TIMEOUT });
    pub fn write(file: *Self, data: []const u8) WriteError.Error!usize {
        var written: c_uint = 0;
        try WriteError.throw(api.write(&file.raw, data.ptr, std.math.cast(c_uint, data.len) orelse return error.Overflow, &written));
        return written;
    }

    pub const ReadError = ErrorSet(&.{ error.Overflow, FR_DISK_ERR, FR_INT_ERR, FR_DENIED, FR_INVALID_OBJECT, FR_TIMEOUT });
    pub fn read(file: *Self, data: []u8) ReadError.Error!usize {
        var written: c_uint = 0;
        try ReadError.throw(api.read(&file.raw, data.ptr, std.math.cast(c_uint, data.len) orelse return error.Overflow, &written));
        return written;
    }

    pub const Reader = std.io.Reader(*Self, ReadError.Error, read);
    pub fn reader(file: *Self) Reader {
        return Reader{ .context = file };
    }

    pub const Writer = std.io.Writer(*Self, WriteError.Error, write);
    pub fn writer(file: *Self) Writer {
        return Writer{ .context = file };
    }
};

pub const Disk = struct {
    const Self = @This();

    getStatusFn: *const fn (self: *Self) Status,
    initializeFn: *const fn (self: *Self) Self.Error!Status,
    readFn: *const fn (self: *Self, buff: [*]u8, sector: c.LBA_t, count: c.UINT) Self.Error!void,
    writeFn: *const fn (self: *Self, buff: [*]const u8, sector: c.LBA_t, count: c.UINT) Self.Error!void,
    ioctlFn: *const fn (self: *Self, cmd: IoCtl, buff: [*]u8) Self.Error!void,

    pub fn getStatus(self: *Self) Status {
        return self.getStatusFn(self);
    }

    pub fn initialize(self: *Self) Self.Error!Status {
        return self.initializeFn(self);
    }

    pub fn read(self: *Self, buff: [*]u8, sector: c.LBA_t, count: c.UINT) Self.Error!void {
        return self.readFn(self, buff, sector, count);
    }

    pub fn write(self: *Self, buff: [*]const u8, sector: c.LBA_t, count: c.UINT) Self.Error!void {
        return self.writeFn(self, buff, sector, count);
    }

    pub fn ioctl(self: *Self, cmd: IoCtl, buff: [*]u8) Self.Error!void {
        return self.ioctlFn(self, cmd, buff);
    }

    fn mapResult(value: Self.Error!void) c.DRESULT {
        if (value) |_| {
            return c.RES_OK;
        } else |err| return switch (err) {
            error.IoError => c.RES_ERROR,
            error.WriteProtected => c.RES_WRPRT,
            error.DiskNotReady => c.RES_NOTRDY,
            error.InvalidParameter => c.RES_PARERR,
        };
    }

    pub const Error = error{
        IoError,
        WriteProtected,
        DiskNotReady,
        InvalidParameter,
    };

    pub const Status = struct {
        initialized: bool,
        disk_present: bool,
        write_protected: bool,

        fn toInteger(self: @This()) c.DSTATUS {
            var i: c.DSTATUS = 0;
            if (!self.initialized) i |= @as(u8, @intCast(c.STA_NOINIT));
            if (!self.disk_present) i |= @as(u8, @intCast(c.STA_NODISK));
            if (self.write_protected) i |= @as(u8, @intCast(c.STA_PROTECT));
            return i;
        }
    };
};

pub var disks: [c.FF_VOLUMES]?*Disk = .{null} ** c.FF_VOLUMES;

pub const WRITE = c.FA_WRITE;
pub const CREATE_ALWAYS = c.FA_CREATE_ALWAYS;
pub const OK = c.FR_OK;

pub const api = struct {
    pub const open = c.f_open; // Open/Create a file
    pub const close = c.f_close; // Close an open file
    pub const read = c.f_read; // Read data from the file
    pub const write = c.f_write; // Write data to the file
    pub const lseek = c.f_lseek; // Move read/write pointer, Expand size
    pub const truncate = c.f_truncate; // Truncate file size
    pub const sync = c.f_sync; // Flush cached data
    pub const forward = c.f_forward; // Forward data to the stream
    pub const expand = c.f_expand; // Allocate a contiguous block to the file
    pub const gets = c.f_gets; // Read a string
    pub const putc = c.f_putc; // Write a character
    pub const puts = c.f_puts; // Write a string
    pub const printf = c.f_printf; // Write a formatted string
    pub const tell = c.f_tell; // Get current read/write pointer
    pub const eof = c.f_eof; // Test for end-of-file
    pub const size = c.f_size; // Get size
    pub const @"error" = c.f_error; // Test for an error

    // Directory Access
    pub const opendir = c.f_opendir; // Open a directory
    pub const closedir = c.f_closedir; // Close an open directory
    pub const readdir = c.f_readdir; // Read a directory item
    pub const findfirst = c.f_findfirst; // Open a directory and read the first item matched
    pub const findnext = c.f_findnext; // Read a next item matched

    // File and Directory Management
    pub const stat = c.f_stat; // Check existance of a file or sub-directory
    pub const unlink = c.f_unlink; // Remove a file or sub-directory
    pub const rename = c.f_rename; // Rename/Move a file or sub-directory
    pub const chmod = c.f_chmod; // Change attribute of a file or sub-directory
    pub const utime = c.f_utime; // Change timestamp of a file or sub-directory
    pub const mkdir = c.f_mkdir; // Create a sub-directory
    pub const chdir = c.f_chdir; // Change current directory
    pub const chdrive = c.f_chdrive; // Change current drive
    pub const getcwd = c.f_getcwd; // Retrieve the current directory and drive

    // Volume Management and System Configuration
    pub const mount = c.f_mount; // Register the work area of the volume
    pub const unmount = c.f_unmount; // Unregister the work area of the volume
    pub const mkfs = c.f_mkfs; // Create an FAT volume on the logical drive
    pub const fdisk = c.f_fdisk; // Create partitions on the physical drive
    pub const getfree = c.f_getfree; // Get free space on the volume
    pub const getlabel = c.f_getlabel; // Get volume label
    pub const setlabel = c.f_setlabel; // Set volume label
    pub const setcp = c.f_setcp; // Set active code page
};

const RtcExport = struct {

    // Current local time shall be returned as bit-fields packed into a DWORD value. The bit fields are as follows:
    // bit31:25  Year origin from the 1980 (0..127, e.g. 37 for 2017)
    // bit24:21  Month (1..12)
    // bit20:16  Day of the month (1..31)
    // bit15:11  Hour (0..23)
    // bit10:5  Minute (0..59)
    // bit4:0  Second / 2 (0..29, e.g. 25 for 50)
    export fn get_fattime() c.DWORD {
        const timestamp = std.time.timestamp() - std.time.epoch.dos;

        const epoch_secs = std.time.epoch.EpochSeconds{
            .secs = @as(u64, @intCast(timestamp)),
        };

        const epoch_day = epoch_secs.getEpochDay();
        const day_secs = epoch_secs.getDaySeconds();

        const year_and_day = epoch_day.calculateYearDay();
        const month_and_day = year_and_day.calculateMonthDay();

        const year: u32 = year_and_day.year;
        const month: u32 = @intFromEnum(month_and_day.month);
        const day: u32 = month_and_day.day_index + 1;

        const hour: u32 = day_secs.getHoursIntoDay();
        const minute: u32 = day_secs.getMinutesIntoHour();
        const second: u32 = day_secs.getSecondsIntoMinute();

        return 0 |
            (year << 25) | // bit31:25  Year origin from the 1980 (0..127, e.g. 37 for 2017)
            (month << 21) | // bit24:21  Month (1..12)
            (day << 16) | // bit20:16  Day of the month (1..31)
            (hour << 11) | // bit15:11  Hour (0..23)
            (minute << 5) | // bit10:5  Minute (0..59)
            ((second / 2) << 0) // bit4:0  Second / 2 (0..29, e.g. 25 for 50)
        ;
    }
};

comptime {
    if (config.has_rtc) {
        // @compileLog("...", config.has_rtc);
        _ = RtcExport;
    }
}

export fn disk_status(
    pdrv: c.BYTE, // Physical drive nmuber to identify the drive */
) c.DSTATUS {
    logger.debug("disk.status({})", .{pdrv});

    const disk = disks[pdrv] orelse return c.STA_NOINIT;
    return disk.getStatus().toInteger();
}

export fn disk_initialize(pdrv: c.BYTE) c.DSTATUS {
    logger.debug("disk.initialize({})", .{pdrv});

    const disk = disks[pdrv] orelse return c.STA_NOINIT;

    if (disk.initialize()) |status| {
        return status.toInteger();
    } else |err| {
        logger.err("disk.initialize({}) failed: {s}", .{ pdrv, @errorName(err) });
        return c.STA_NOINIT;
    }
}

export fn disk_read(
    pdrv: c.BYTE, // Physical drive nmuber to identify the drive */
    buff: [*]c.BYTE, // Data buffer to store read data */
    sector: c.LBA_t, // Start sector in LBA */
    count: c.UINT, // Number of sectors to read */
) c.DRESULT {
    const disk = disks[pdrv] orelse return c.RES_NOTRDY;
    logger.debug("disk.read({}, {*}, {}, {})", .{ pdrv, buff, sector, count });
    return Disk.mapResult(disk.read(buff, sector, count));
}

export fn disk_write(
    pdrv: c.BYTE, // Physical drive nmuber to identify the drive */
    buff: [*]const c.BYTE, // Data to be written */
    sector: c.LBA_t, // Start sector in LBA */
    count: c.UINT, // Number of sectors to write */
) c.DRESULT {
    const disk = disks[pdrv] orelse return c.RES_NOTRDY;
    logger.debug("disk.write({}, {*}, {}, {})", .{ pdrv, buff, sector, count });
    return Disk.mapResult(disk.write(buff, sector, count));
}

export fn disk_ioctl(
    pdrv: c.BYTE, // Physical drive nmuber (0..) */
    cmd: c.BYTE, // Control code */
    buff: [*]u8, // Buffer to send/receive control data */
) c.DRESULT {
    const disk = disks[pdrv] orelse return c.RES_NOTRDY;
    logger.debug("disk.ioctl({}, {}, {*})", .{ pdrv, cmd, buff });
    return Disk.mapResult(disk.ioctl(@as(IoCtl, @enumFromInt(cmd)), buff));
}

pub const IoCtl = enum(u8) {
    /// Complete pending write process (needed at FF_FS_READONLY == 0).
    ///
    /// Makes sure that the device has finished pending write process. If the disk I/O layer or
    /// storage device has a write-back cache, the dirty cache data must be committed to the medium
    /// immediately. Nothing to do for this command if each write operation to the medium is
    /// completed in the disk_write function.
    sync = @as(u8, @intCast(c.CTRL_SYNC)),

    /// Get media size (needed at FF_USE_MKFS == 1)
    /// Retrieves number of available sectors, the largest allowable LBA + 1, on the drive into the
    /// LBA_t variable that pointed by buff. This command is used by f_mkfs and f_fdisk function to
    /// determine the size of volume/partition to be created. It is required when FF_USE_MKFS == 1.
    get_sector_count = @as(u8, @intCast(c.GET_SECTOR_COUNT)),

    /// Get sector size (needed at FF_MAX_SS != FF_MIN_SS)
    /// Retrieves sector size, minimum data unit for generic read/write, into the WORD variable that
    /// pointed by buff. Valid sector sizes are 512, 1024, 2048 and 4096. This command is required
    /// only if FF_MAX_SS > FF_MIN_SS. When FF_MAX_SS == FF_MIN_SS, this command will be never used
    /// and the read/write function must work in FF_MAX_SS bytes/sector.
    get_sector_size = @as(u8, @intCast(c.GET_SECTOR_SIZE)),

    /// Get erase block size (needed at FF_USE_MKFS == 1)
    /// Retrieves erase block size in unit of sector of the flash memory media into the DWORD variable
    /// that pointed by buff. The allowable value is 1 to 32768 in power of 2. Return 1 if the value is
    /// unknown or non flash memory media. This command is used by only f_mkfs function and it attempts
    /// to align data area on the suggested block boundary. It is required when FF_USE_MKFS == 1.
    /// Note that FatFs does not have FTL (flash translation layer). Either disk I/O layter or storage
    /// device must have an FTL in it.
    get_block_size = @as(u8, @intCast(c.GET_BLOCK_SIZE)),

    /// Inform device that the data on the block of sectors is no longer used (needed at FF_USE_TRIM == 1)
    /// Informs the disk I/O layter or the storage device that the data on the block of sectors is no longer
    /// needed and it can be erased. The sector block is specified in an LBA_t array {<Start LBA>, <End LBA>}
    /// that pointed by buff. This is an identical command to Trim of ATA device. Nothing to do for this
    /// command if this funcion is not supported or not a flash memory device. FatFs does not check the result
    /// code and the file function is not affected even if the sector block was not erased well. This command
    /// is called on remove a cluster chain and in the f_mkfs function. It is required when FF_USE_TRIM == 1.
    trim = @as(u8, @intCast(c.CTRL_TRIM)),

    _,
};

pub const GlobalError = error{
    DiskErr,
    IntErr,
    NotReady,
    NoFile,
    NoPath,
    InvalidName,
    Denied,
    Exist,
    InvalidObject,
    WriteProtected,
    InvalidDrive,
    NotEnabled,
    NoFilesystem,
    MkfsAborted,
    Timeout,
    Locked,
    OutOfMemory,
    TooManyOpenFiles,
    InvalidParameter,
};

const FR_DISK_ERR = error.DiskErr;
const FR_INT_ERR = error.IntErr;
const FR_NOT_READY = error.NotReady;
const FR_NO_FILE = error.NoFile;
const FR_NO_PATH = error.NoPath;
const FR_INVALID_NAME = error.InvalidName;
const FR_DENIED = error.Denied;
const FR_EXIST = error.Exist;
const FR_INVALID_OBJECT = error.InvalidObject;
const FR_WRITE_PROTECTED = error.WriteProtected;
const FR_INVALID_DRIVE = error.InvalidDrive;
const FR_NOT_ENABLED = error.NotEnabled;
const FR_NO_FILESYSTEM = error.NoFilesystem;
const FR_MKFS_ABORTED = error.MkfsAborted;
const FR_TIMEOUT = error.Timeout;
const FR_LOCKED = error.Locked;
const FR_NOT_ENOUGH_CORE = error.OutOfMemory;
const FR_TOO_MANY_OPEN_FILES = error.TooManyOpenFiles;
const FR_INVALID_PARAMETER = error.InvalidParameter;

fn ErrorSet(comptime options: []const anyerror) type {
    return struct {
        pub const Error: type = @Type(.{
            .ErrorSet = blk: {
                var names: [options.len]std.builtin.Type.Error = undefined;
                for (&names, options) |*name, err| {
                    name.* = .{ .name = @errorName(err) };
                }
                break :blk &names;
            },
        });

        pub fn throw(error_code: c.FRESULT) Error!void {
            const mapped_error = if (mapGenericError(error_code)) |_| {
                return;
            } else |err| err;

            inline for (options) |error_option| {
                if (mapped_error == error_option)
                    return error_option; // must return the comptime known value for inference
            }

            std.debug.panic("unexpected error: {s}", .{@errorName(mapped_error)});
        }
    };
}

pub fn mapGenericError(code: c.FRESULT) GlobalError!void {
    return switch (code) {
        c.FR_OK => {},
        c.FR_DISK_ERR => error.DiskErr,
        c.FR_INT_ERR => error.IntErr,
        c.FR_NOT_READY => error.NotReady,
        c.FR_NO_FILE => error.NoFile,
        c.FR_NO_PATH => error.NoPath,
        c.FR_INVALID_NAME => error.InvalidName,
        c.FR_DENIED => error.Denied,
        c.FR_EXIST => error.Exist,
        c.FR_INVALID_OBJECT => error.InvalidObject,
        c.FR_WRITE_PROTECTED => error.WriteProtected,
        c.FR_INVALID_DRIVE => error.InvalidDrive,
        c.FR_NOT_ENABLED => error.NotEnabled,
        c.FR_NO_FILESYSTEM => error.NoFilesystem,
        c.FR_MKFS_ABORTED => error.MkfsAborted,
        c.FR_TIMEOUT => error.Timeout,
        c.FR_LOCKED => error.Locked,
        c.FR_NOT_ENOUGH_CORE => error.OutOfMemory,
        c.FR_TOO_MANY_OPEN_FILES => error.TooManyOpenFiles,
        c.FR_INVALID_PARAMETER => error.InvalidParameter,
        else => unreachable,
    };
}
