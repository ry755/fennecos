const std = @import("std");

pub const fatfs = @import("zfat");

// FIXME: this is basically hardcoded to use IDE. that is bad!!

pub var global_fat: fatfs.FileSystem = undefined;
var disk_read_sector: *const fn (sector: u32, buffer: [*]u8, count: u32) void = undefined;
var disk_write_sector: *const fn (sector: u32, buffer: [*]const u8, count: u32) void = undefined;
var disk_partition_offset: u32 = 0;

var disk: fatfs.Disk = undefined;

pub fn initialize(
    disk_id: u32,
    read_sector: *const fn (sector: u32, buffer: [*]u8, count: u32) void,
    write_sector: *const fn (sector: u32, buffer: [*]const u8, count: u32) void,
    partition_offset: u32,
) void {
    disk_read_sector = read_sector;
    disk_write_sector = write_sector;
    disk_partition_offset = partition_offset;

    disk = fatfs.Disk{
        .getStatusFn = getStatusFn,
        .initializeFn = initializeFn,
        .readFn = readFn,
        .writeFn = writeFn,
        .ioctlFn = ioctlFn,
    };

    fatfs.disks[disk_id] = &disk;
}

fn getStatusFn(interface: *fatfs.Disk) fatfs.Disk.Status {
    _ = interface;
    return fatfs.Disk.Status{
        .initialized = true,
        .disk_present = true,
        .write_protected = false,
    };
}

fn initializeFn(interface: *fatfs.Disk) fatfs.Disk.Error!fatfs.Disk.Status {
    _ = interface;
    return fatfs.Disk.Status{
        .initialized = true,
        .disk_present = true,
        .write_protected = false,
    };
}

fn readFn(interface: *fatfs.Disk, buff: [*]u8, sector: fatfs.LBA, count: c_uint) fatfs.Disk.Error!void {
    _ = interface;
    disk_read_sector(sector, buff, count);
}

fn writeFn(interface: *fatfs.Disk, buff: [*]const u8, sector: fatfs.LBA, count: c_uint) fatfs.Disk.Error!void {
    _ = interface;
    disk_write_sector(sector, buff, count);
}

fn ioctlFn(interface: *fatfs.Disk, cmd: fatfs.IoCtl, buff: [*]u8) fatfs.Disk.Error!void {
    _ = buff;
    _ = cmd;
    _ = interface;
}
