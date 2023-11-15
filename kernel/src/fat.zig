const std = @import("std");

const DIRENT_READ_ONLY: u8 = 0x01;
const DIRENT_HIDDEN: u8 = 0x02;
const DIRENT_SYSTEM: u8 = 0x04;
const DIRENT_VOLUME_ID: u8 = 0x08;
const DIRENT_DIRECTORY: u8 = 0x10;
const DIRENT_ARCHIVE: u8 = 0x20;

pub const State = struct {
    read_sector: *const fn (sector: u32, buffer: [*]u8, count: u32) void,
    write_sector: *const fn (sector: u32, buffer: [*]u8, count: u32) void,
    fat_type: FatType,
    partition_offset: u32,
    fat_start_sector: u32,
    fat_size: u32, // in sectors
    rootdir_start_sector: u32,
    fat32_root_cluster: u32,
    bpb: Bpb,
};

pub const File = struct {
    dirent: Dirent_8_3,
    seek: u32,
};

const FatType = enum {
    fat12,
    fat16,
    fat32,
};

const Bpb = extern struct {
    jump: [3]u8 align(1),
    oem_id: [8]u8 align(1),
    bytes_per_sector: u16 align(1),
    sectors_per_cluster: u8 align(1),
    reserved_sectors: u16 align(1),
    fat_count: u8 align(1),
    root_dir_ents: u16 align(1),
    sector_count: u16 align(1), // if 0 then actual sector count is stored in large sector count field
    media_descriptor: u8 align(1),
    sectors_per_fat: u16 align(1), // FAT12 & 16 only
    sectors_per_track: u16 align(1),
    head_count: u16 align(1),
    hidden_sector_count: u32 align(1),
    large_sector_count: u32 align(1),
};

const Ebpb_12_16 = extern struct {
    drive_number: u8 align(1),
    nt_flags: u8 align(1),
    signature: u8 align(1),
    volume_id: u32 align(1),
    volume_label: [11]u8 align(1),
    fat_system_id: [8]u8 align(1),
};

const Ebpb_32 = extern struct {
    sectors_per_fat: u32 align(1),
    flags: u16 align(1),
    fat_version: u16 align(1),
    root_cluster_number: u32 align(1),
    fsinfo_sector: u16 align(1),
    bootbackup_sector: u16 align(1),
    reserved: [12]u8 align(1),
    ebpb_12_16: Ebpb_12_16 align(1),
};

const Dirent_8_3 = extern struct {
    name: [8]u8 align(1),
    extension: [3]u8 align(1),
    attr: u8 align(1),
    nt: u8 align(1),
    creation_time_fine: u8 align(1),
    creation_time: u16 align(1),
    creation_date: u16 align(1),
    access_date: u16 align(1),
    cluster_high: u16 align(1),
    modification_time: u16 align(1),
    modification_date: u16 align(1),
    cluster_low: u16 align(1),
    filesize: u32 align(1),
};

// TODO: implement support/detection for FAT12/16
pub fn initialize(
    state: *State,
    read_sector: *const fn (sector: u32, buffer: [*]u8, count: u32) void,
    write_sector: *const fn (sector: u32, buffer: [*]u8, count: u32) void,
    partition_offset: u32,
) void {
    state.*.read_sector = read_sector;
    state.*.write_sector = write_sector;
    state.*.partition_offset = partition_offset;

    // read the BPB
    var sector_buffer = std.mem.zeroes([512]u8);
    state.*.read_sector(state.*.partition_offset + 0, &sector_buffer, 1);
    state.*.bpb = std.mem.bytesToValue(Bpb, sector_buffer[0..@sizeOf(Bpb)]);

    // read the EBPB (FAT32)
    var ebpb: Ebpb_32 = undefined;
    state.*.read_sector(state.*.partition_offset + @sizeOf(Bpb), &sector_buffer, 1);
    ebpb = std.mem.bytesToValue(Ebpb_32, sector_buffer[0..@sizeOf(Ebpb_32)]);

    var sectors_per_fat: u32 = @intCast(state.*.bpb.sectors_per_fat);
    if (sectors_per_fat == 0)
        sectors_per_fat = ebpb.sectors_per_fat;
    state.*.fat_start_sector = state.*.bpb.reserved_sectors;
    state.*.fat_size = sectors_per_fat * state.*.bpb.fat_count;
    state.*.rootdir_start_sector = state.*.fat_start_sector + state.*.fat_size;
    state.*.fat32_root_cluster = ebpb.root_cluster_number;
}

fn read_fat_entry(state: *State, entry: u32) u32 {
    const fat_sector_number = state.*.bpb.reserved_sectors + (entry * 4 / state.*.bpb.bytes_per_sector);
    const fat_entry_offset = (entry * 4) % state.*.bpb.bytes_per_sector;
    var sector_buffer = std.mem.zeroes([512 / 4]u32);
    state.*.read_sector(state.*.partition_offset + fat_sector_number, &sector_buffer, 1);
    return sector_buffer[fat_entry_offset] & 0x0FFFFFFF;
}

fn cluster_to_sector(state: *State, cluster: u32) u32 {
    const root_dir_sectors = (32 * state.*.bpb.root_dir_ents + state.*.bpb.bytes_per_sector - 1) / state.*.bpb.bytes_per_sector;
    const data_start_sector = state.*.rootdir_start_sector + root_dir_sectors;
    const first_sector_of_cluster = data_start_sector + (cluster - 2) * state.*.bpb.sectors_per_cluster;
    return first_sector_of_cluster;
}

fn next_cluster(state: *State, cluster: u32) u32 {
    const byte_offset = cluster * 4;
    const fat_sector = state.*.fat_start_sector + (byte_offset / state.*.bpb.bytes_per_sector);
    const sector_offset = byte_offset % state.*.bpb.bytes_per_sector;
    var sector_buffer = std.mem.zeroes([512 / 4]u32);
    state.*.read_sector(state.*.partition_offset + (fat_sector * state.*.bytes_per_sector), &sector_buffer, 1);
    return sector_buffer[sector_offset];
}
