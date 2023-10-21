const std = @import("std");

var heap = std.mem.zeroes([32 * 1024 * 1024]u8);
var fballoc = std.heap.FixedBufferAllocator.init(heap[0..]);
pub var allocator = &fballoc.allocator();
