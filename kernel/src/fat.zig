const std = @import("std");
const fatfs = @import("zfat");

var global_fat: fatfs.FileSystem = undefined;

// FIXME: this is wrong
pub fn initialize(
    disk_id: u32,
    read_sector: *const fn (sector: u32, buffer: [*]u8, count: u32) void,
    write_sector: *const fn (sector: u32, buffer: [*]u8, count: u32) void,
    partition_offset: u32,
) void {
    _ = disk_id;
    _ = partition_offset;

    const disk = fatfs.Disk{
        .getStatusFn = fn (interface: *fatfs.Disk) fatfs.Disk.Status{
            return fatfs.Disk.Status{
                .initialized = true,
                .disk_present = true,
                .write_protected = false,
            },
        },
        .initializeFn = fn (interface: *fatfs.Disk) fatfs.Disk.Error!fatfs.Disk.Status{
            return fatfs.Disk.Status{
                .initialized = true,
                .disk_present = true,
                .write_protected = false,
            },
        },
        .readFn = fn (interface: *fatfs.Disk, buff: [*]u8, sector: fatfs.LBA, count: c_uint) fatfs.Disk.Error!void{
            read_sector(sector, buff, count),
        },
        .writeFn = fn (interface: *fatfs.Disk, buff: [*]u8, sector: fatfs.LBA, count: c_uint) fatfs.Disk.Error!void{
            write_sector(sector, buff, count),
        },
        .ioctlFn = fn (interface: *fatfs.Disk, cmd: fatfs.IoCtl, buff: [*]u8) fatfs.Disk.Error!void{},
    };
    _ = disk;

    //fatfs.disks[disk_id] = &fatfs.Disk{
    //    .getStatusFn = get_status,
    //    .initializeFn = init,
    //    .readFn = read,
    //    .writeFn = write,
    //    .ioctlFn = ioctl,
    //};
}
