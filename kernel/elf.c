// based on these pages: https://wiki.osdev.org/ELF and https://wiki.osdev.org/ELF_Tutorial

#include <kernel/allocator.h>
#include <kernel/elf.h>
#include <kernel/paging.h>

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

extern page_directory_t *current_page_directory;

static inline elf_section_header_t *elf_sheader(elf_main_header_t *header) {
    return (elf_section_header_t *) ((int) header + header->shoff);
}

static inline elf_section_header_t *elf_section(elf_main_header_t *header, uint32_t n) {
    return &elf_sheader(header)[n];
}

static void *elf_lookup_symbol(const char *name) {
    (void) name;
    return NULL;
}

static uint32_t elf_get_symval(elf_main_header_t *header, int32_t table, uint32_t index) {
    if (!table || !index) return 0;
    elf_section_header_t *symtab = elf_section(header, table);

    uint32_t symtab_entries = symtab->size / symtab->entsize;
    if(index >= symtab_entries) {
        kprintf("ELF symbol index out of range (%d:%u)\n", table, index);
        return 0;
    }

    int symaddr = (int) header + symtab->offset;
    elf_symbol_t *symbol = &((elf_symbol_t *) symaddr)[index];

    if (!symbol->shndx) {
        // external symbol, look up value
        elf_section_header_t *strtab = elf_section(header, symtab->link);
        const char *name = (const char *) header + strtab->offset + symbol->name;

        void *target = elf_lookup_symbol(name);

        if (target == NULL) {
            // extern symbol not found
            if (ELF32_ST_BIND(symbol->info) & STB_WEAK) {
                // weak symbol initialized as 0
                return 0;
            } else {
                kprintf("undefined external symbol in ELF: %s\n", name);
                return 0;
            }
        } else {
            return (int) target;
        }

    } else if(symbol->shndx == SHN_ABS) {
        // absolute symbol
        return symbol->value;
    } else {
        // internally defined symbol
        elf_section_header_t *target = elf_section(header, symbol->shndx);
        return (int) header + symbol->value + target->offset;
    }
}

static bool relocate_elf_entry(elf_main_header_t *header, elf_relocation_t *rel, elf_section_header_t *reltab) {
    elf_section_header_t *target = elf_section(header, reltab->info);

    int addr = (int) header + target->offset;
    int *ref = (int *) (addr + rel->offset);

    // symbol value
    uint32_t symval = 0;
    if (ELF32_R_SYM(rel->info)) {
        symval = elf_get_symval(header, reltab->link, ELF32_R_SYM(rel->info));
        if (symval == (uint32_t) -1) return false;
    }

    // relocate based on type
    switch(ELF32_R_TYPE(rel->info)) {
        case R_386_NONE:
            // no relocation
            break;
        case R_386_32:
            // symbol + offset
            *ref = DO_386_32(symval, *ref);
            kprintf("relocated symbol + offset\n");
            break;
        case R_386_PC32:
            // symbol + offset - section offset
            *ref = DO_386_PC32(symval, *ref, (int) ref);
            kprintf("relocated symbol + offset - section offset\n");
            break;
        default:
            kprintf("unsupported ELF relocation type: %d\n", ELF32_R_TYPE(rel->info));
            return false;
    }
    return true;
}

static void load_elf_stage_1(elf_main_header_t *header) {
    elf_section_header_t *shdr = elf_sheader(header);

    // iterate over section headers
    for (uint32_t i = 0; i < header->shnum; i++) {
        elf_section_header_t *section = &shdr[i];

        // if the section isn't present in the file
        if (section->type == SHT_NOBITS) {
            // skip if it the section is empty
            if (!section->size) continue;
            // if the section should appear in memory
            if (section->flags & SHF_ALLOC) {
                // allocate and zero some memory
                void *mem = allocate(section->size);
                memset(mem, 0, section->size);

                // assign the memory offset to the section offset
                section->offset = (int) mem - (int) header;
                kprintf("allocated %d bytes for an ELF section\n", section->size);
            }
        }
    }
}

static bool load_elf_stage_2(elf_main_header_t *header) {
    elf_section_header_t *shdr = elf_sheader(header);

    // iterate over section headers
    for (uint32_t i = 0; i < header->shnum; i++) {
        elf_section_header_t *section = &shdr[i];

        // if this is a relocation section
        if (section->type == SHT_REL) {
            // process each entry in the table
            for (uint32_t j = 0; j < section->size / section->entsize; j++) {
                elf_relocation_t *reltab = &((elf_relocation_t *)((int) header + section->offset))[j];
                if (!relocate_elf_entry(header, reltab, section)) {
                    kprintf("failed to relocate symbol\n");
                    return false;
                }
            }
        }
    }
    return true;
}

static bool verify_elf(elf_main_header_t *header) {
    if (!header) return false;

    // check for the magic bytes
    if (header->magic != ELF_MAGIC) {
        kprintf("failed to verify ELF magic bytes\n");
        return false;
    }

    // ensure the elf file is EXEC
    if (header->type != ELF_EXEC) {
        kprintf("ELF is not of type EXEC\n");
        return false;
    }
    return true;
}

void parse_elf(page_directory_t *page_directory, process_context_t *process_context) {
    page_directory_t *old_page_directory = current_page_directory;
    switch_page_directory(page_directory);
    elf_main_header_t *header = (elf_main_header_t *) 0x08048000;
    if (!verify_elf(header)) {
        switch_page_directory(old_page_directory);
        return;
    }
    load_elf_stage_1(header);
    if (!load_elf_stage_2(header)) {
        switch_page_directory(old_page_directory);
        return;
    }
    kprintf("ELF header->entry: 0x%x\n", header->entry);
    process_context->eip = header->entry;
}
