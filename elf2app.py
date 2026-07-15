#!/usr/bin/env python3

# based on https://mid.net.ua/git/mid/luma/src/branch/master/elftoexe.py

from elftools.elf.elffile import ELFFile
from elftools.elf.relocation import RelocationSection
import struct, sys, itertools, os
from dataclasses import dataclass
from collections import namedtuple

def bitsize(i):
    return max(len(bin(i)[2:]), 1)

def dump(x):
    data = b''.join(dump(c) for c in x.children) if type(x.children) == list else x.children
    l = len(data)
    l = l | (1 << ((bitsize(l) + 6) // 7 * 7))
    return x.id.to_bytes(length = (bitsize(x.id) + 7) // 8) + l.to_bytes(length = (bitsize(l) + 7) // 8) + data

header = b"APP\0"

@dataclass
class ELFReloc:
    offset: int
    reference: int

@dataclass
class ELFSegment:
    data: bytearray
    vaddr: int
    relocations: list[ELFReloc]

with open(sys.argv[1], "rb") as input, open(sys.argv[2], "wb") as output:
    elf = ELFFile(input)
    segments = []
    sections_base = min([sec["sh_offset"] for sec in elf.iter_sections() if sec.name])
    base_vaddr = min([seg["p_vaddr"] for seg in elf.iter_segments()])

    for seg in elf.iter_segments():
        if seg["p_type"] != "PT_LOAD":
            continue

        # If no section is in the segment, then ignore it
        if seg["p_offset"] < sections_base and seg["p_offset"] + seg["p_filesz"] < sections_base:
            continue

        input.seek(seg["p_offset"])

        segments.append(ELFSegment(vaddr = seg["p_vaddr"], data = bytearray(input.read(seg["p_filesz"]) + b'\0' * (seg["p_memsz"] - seg["p_filesz"])), relocations = []))

    rel_section = elf.get_section_by_name(".rel.dyn")
    if rel_section:
        for rel in rel_section.iter_relocations():
            assert rel["r_info"] % 256 == 8

            # The segment in which the relocation is found/required
            seg_in = next((s for s in reversed(segments) if s.vaddr <= rel["r_offset"]), None)

            offset = rel["r_offset"] - seg_in.vaddr

            val = struct.unpack("I", seg_in.data[offset : offset + 4])[0]

            # The segment pointed to by the relocation
            seg_ref = next((s for s in reversed(segments) if s.vaddr <= val), None)

            seg_in.data[offset : offset + 4] = struct.pack("I", val - seg_ref.vaddr)

            seg_in.relocations.append(ELFReloc(offset = rel["r_offset"] - seg_in.vaddr, reference = segments.index(seg_ref)))

    output.write(header)
    start = output.tell()
    output.seek(4, os.SEEK_CUR) # output.write(code_size)
    output.seek(4, os.SEEK_CUR) # output.write(code_ptr)
    output.seek(4, os.SEEK_CUR) # output.write(struct.pack("<I" ,len(relocation_table)))
    output.seek(4, os.SEEK_CUR) # output.write(relocation_table_ptr)

    code_ptr = output.tell()
    for seg in segments:
        output.write(seg.data)
    code_size = output.tell() - code_ptr
    relocation_table_ptr = output.tell()
    relocation_table_size = 0
    for seg in segments:
        seg_start = output.tell()
        print("segment at " + hex(seg_start) + " (vaddr = " + hex(seg.vaddr) + ")")
        for reloc in seg.relocations:
            reloc_location = reloc.offset + code_ptr
            output.write(struct.pack("<I", reloc_location))
            print("relocation at " + hex(reloc_location))
            relocation_table_size += 1

    output.seek(start)
    output.write(struct.pack("<I", code_size))
    output.write(struct.pack("<I", code_ptr))
    output.write(struct.pack("<I", relocation_table_size))
    output.write(struct.pack("<I", relocation_table_ptr))
