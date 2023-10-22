const ALIGN = 1 << 0;
const MEMINFO = 1 << 1;
const VIDEOMODE = 1 << 2;
const MAGIC = 0x1BADB002;
const FLAGS = ALIGN | MEMINFO | VIDEOMODE;

const MultibootHeader = extern struct {
    magic: i32 = MAGIC,
    flags: i32,
    checksum: i32,
    padding_1: i32 = 0x00000000,
    padding_2: i32 = 0x00000000,
    padding_3: i32 = 0x00000000,
    padding_4: i32 = 0x00000000,
    padding_5: i32 = 0x00000000,
    video_mode_type: i32,
    video_mode_width: i32,
    video_mode_height: i32,
    video_mode_depth: i32,
};

export var multiboot align(4) linksection(".multiboot") = MultibootHeader{
    .flags = FLAGS,
    .checksum = -(MAGIC + FLAGS),
    .video_mode_type = 0x00000000,
    .video_mode_width = 640,
    .video_mode_height = 480,
    .video_mode_depth = 32,
};

pub const MultibootInfo = extern struct {
    flags: u32,
    mem_lower: u32,
    mem_upper: u32,
    boot_device: u32,
    cmdline: u32,
    mods_count: u32,
    mods_addr: u32,
    syms_placeholder_1: u32,
    syms_placeholder_2: u32,
    syms_placeholder_3: u32,
    syms_placeholder_4: u32,
    mmap_length: u32,
    mmap_addr: u32,
    drives_length: u32,
    drives_addr: u32,
    config_table: u32,
    boot_loader_name: u32,
    apm_table: u32,
    vbe_control_info: u32,
    vbe_mode_info: u32,
    vbe_mode: u16,
    vbe_interface_seg: u16,
    vbe_interface_off: u16,
    vbe_interface_len: u16,
    framebuffer_addr: u64,
    framebuffer_pitch: u32,
    framebuffer_width: u32,
    framebuffer_height: u32,
    framebuffer_bpp: u8,
    framebuffer_type: u8,
    framebuffer_red_field_position: u8,
    framebuffer_red_mask_size: u8,
    framebuffer_green_field_position: u8,
    framebuffer_green_mask_size: u8,
    framebuffer_blue_field_position: u8,
    framebuffer_blue_mask_size: u8,
};
