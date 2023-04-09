#pragma once

#include <kernel/paging.h>
#include <kernel/process.h>

#include <stdint.h>

#define ELF_MAGIC 0x464C457FU

#define SHN_ABS 0xFFF1
#define DO_386_32(S, A)      ((S) + (A))
#define DO_386_PC32(S, A, P) ((S) + (A) - (P))
#define ELF32_R_SYM(INFO)    ((INFO) >> 8)
#define ELF32_R_TYPE(INFO)   ((uint8_t)(INFO))
#define ELF32_ST_BIND(INFO) ((INFO) >> 4)
#define ELF32_ST_TYPE(INFO) ((INFO) & 0x0F)

uint32_t parse_elf(page_directory_t *page_directory, uint8_t *buffer);

typedef struct elf_main_header_s {
    uint32_t magic;
    uint8_t ident[12];
    uint16_t type;
    uint16_t machine;
    uint32_t version;
    uint32_t entry;
    uint32_t phoff;
    uint32_t shoff;
    uint32_t flags;
    uint16_t ehsize;
    uint16_t phentsize;
    uint16_t phnum;
    uint16_t shentsize;
    uint16_t shnum;
    uint16_t shstrndx;
} elf_main_header_t;

typedef struct elf_section_header_s {
    uint32_t name;
    uint32_t type;
    uint32_t flags;
    uint32_t addr;
    uint32_t offset;
    uint32_t size;
    uint32_t link;
    uint32_t info;
    uint32_t addralign;
    uint32_t entsize;
} elf_section_header_t;

typedef struct elf_program_header_s {
    uint32_t type;
    uint32_t offset;
    uint32_t vaddr;
    uint32_t paddr;
    uint32_t filesz;
    uint32_t memsz;
    uint32_t flags;
    uint32_t align;
} elf_program_header_t;

typedef struct elf_symbol_s {
    uint32_t name;
    uint32_t value;
    uint32_t size;
    uint8_t info;
    uint8_t other;
    uint16_t shndx;
} elf_symbol_t;

typedef struct elf_relocation_s {
    uint32_t offset;
    uint32_t info;
} elf_relocation_t;

enum elf_type {
    ELF_NONE = 0, // unknown
    ELF_REL  = 1, // relocatable
    ELF_EXEC = 2  // executable
};

enum elf_sht_types {
    SHT_NULL     = 0, // null section
    SHT_PROGBITS = 1, // program information
    SHT_SYMTAB   = 2, // symbol table
    SHT_STRTAB   = 3, // string table
    SHT_RELA     = 4, // relocation (w/ addend)
    SHT_NOBITS   = 8, // not present in file
    SHT_REL      = 9  // relocation (no addend)
};

enum elf_sht_attributes {
    SHF_WRITE = 0x01, // writable section
    SHF_ALLOC = 0x02  // exists in memory
};

enum elf_rel_types {
    R_386_NONE = 0, // no relocation
    R_386_32   = 1, // symbol + offset
    R_386_PC32 = 2  // symbol + offset - section offset
};

enum elf_symbol_bindings {
    STB_LOCAL  = 0, // local scope
    STB_GLOBAL = 1, // global scope
    STB_WEAK   = 2  // weak
};

enum elf_symbol_types {
    STT_NOTYPE = 0, // no type
    STT_OBJECT = 1, // variables, arrays, etc.
    STT_FUNC   = 2  // functions
};

enum elf_pht_types {
    PT_NULL = 0,
    PT_LOAD = 1
};
