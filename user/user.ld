SECTIONS {
	. = 0x00000000;
	.text : { *(.text) *(.rodata) }
	.data : { *(.data) *(.bss) }

	/DISCARD/ : {
		*(.note.GNU-stack)
		*(.gnu_debuglink)
		*(.gnu.lto_*)
		*(.note.gnu.build-id)
		*(.eh_frame_hdr)
		*(.eh_frame_entry .eh_frame_entry.*)
		*(.eh_frame)
		*(.eh_frame.*)
		*(.interp)
	}
}
