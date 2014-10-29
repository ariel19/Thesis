#include "elffile.h"

void* ELF::get_ph_seg_offset(uint32_t idx) {
    try {
        void *ph = get_ph_header(idx);
        if (!ph || cls == classes::NONE)
            return nullptr;
        return cls == classes::ELF32 ?
               reinterpret_cast<void*>(reinterpret_cast<Elf32_Phdr*>(ph)->p_offset) :
               reinterpret_cast<void*>(reinterpret_cast<Elf64_Phdr*>(ph)->p_offset);
    }
    catch(const std::exception &) {
        return nullptr;
    }
}

bool ELF::get_ph_addresses() {
    try {
        ex_offset_t addr = ph_idx.at(0) + ph_size;

        // fill the rest of the list with file offsets
        for (esize_t i = 1; i < ph_num; ++i) {
            ph_idx.push_back(addr);
            addr += ph_size;
        }
    }
    catch(const std::exception &) {
        return false;
    }
    return true;
}

void* ELF::get_elf_header() {
    try {
        return is_valid() ?
               reinterpret_cast<void*>(&(b_data.data()[elf_header_idx])) :
               nullptr;
    }
    catch(const std::exception &) {
        return nullptr;
    }
}

void* ELF::get_ph_header(uint32_t idx) {
    try {
        // TODO: check wtf will happen if idx is out of range :)
        if (idx >= ph_idx.size())
            return nullptr;

        return is_valid() ?
               reinterpret_cast<void*>(&(b_data.data())[ph_idx.at(idx)]) :
               nullptr;
    }
    catch(const std::exception &) {
        return nullptr;
    }
}

bool ELF::check_magic(const Elf32_Ehdr *elf_hdr) const {
    if (!elf_hdr)
        return false;

    try {
        return (elf_hdr->e_ident[EI_MAG0] == ELFMAG0) &
               (elf_hdr->e_ident[EI_MAG1] == ELFMAG1) &
               (elf_hdr->e_ident[EI_MAG2] == ELFMAG2) &
               (elf_hdr->e_ident[EI_MAG3] == ELFMAG3);
    }
    catch(const std::exception &) {
        return nullptr;
    }
}

bool ELF::is_supported(const Elf32_Ehdr *elf_hdr) {
    if (!elf_hdr)
        return false;

    try {
        // check if file is 32-bit or 64-bit
        if (elf_hdr->e_ident[EI_CLASS] == ELFCLASS32)
            cls = classes::ELF32;
        else if (elf_hdr->e_ident[EI_CLASS] == ELFCLASS64)
            cls = classes::ELF64;
        else return false;

        // byte order is not little endian :)
        if (elf_hdr->e_ident[EI_DATA] != ELFDATA2LSB)
            return false;

        // non current ELF file version
        if (elf_hdr->e_ident[EI_VERSION] != EV_CURRENT)
            return false;

        // if file is non-x86_64 platform
        if (elf_hdr->e_machine != EM_X86_64)
            return false;

        // file is non-executable
        if (elf_hdr->e_type != ET_EXEC)
            return false;
    }
    catch(const std::exception &) {
        return false;
    }

    return true;
}

bool ELF::get_ph_info(const void *elf_hdr) {
    if (!elf_hdr)
        return false;
    try {
        if (cls == classes::NONE)
            return false;
        if (cls == classes::ELF32) {
            const Elf32_Ehdr* e_hdr = reinterpret_cast<const Elf32_Ehdr *>(elf_hdr);
            ph_size = e_hdr->e_phentsize;
            ph_num = e_hdr->e_phnum;
            ph_idx.push_back(e_hdr->e_phoff);
        }
        else {
            const Elf64_Ehdr* e_hdr = reinterpret_cast<const Elf64_Ehdr *>(elf_hdr);
            ph_size = e_hdr->e_phentsize;
            ph_num = e_hdr->e_phnum;
            ph_idx.push_back(e_hdr->e_phoff);
        }
        // fill ph_idx list with according values
        if (!get_ph_addresses())
            return false;
    }
    catch(const std::exception &) {
        return false;
    }
    return true;
}

/* TODO: real parsing */
bool ELF::parse() {
    // check if file is ready for parsing
    if (!is_open())
        return false;

    char *data = b_data.data();
    size_t len = b_data.length();

    try {
        // get ELF_header
        Elf32_Ehdr* elf_hdr = reinterpret_cast<Elf32_Ehdr *>(data);
        if (!check_magic(elf_hdr))
            return false;
        // check if file format is supported
        if (!is_supported(elf_hdr))
            return false;
        // get file info relevant to file
        if (!get_ph_info(reinterpret_cast<const void *>(data)))
            return false;
    }
    catch (const std::exception &) {
        return false;
    }

    return true;
}

ELF::ELF(QString _fname) :
    parsed(false), elf_file(_fname), cls(classes::NONE) {
    if (elf_file.open(QFile::ReadOnly)) {
        b_data = elf_file.readAll();
        parsed = parse();
    }
}

ELF::~ELF() {
    if (elf_file.isOpen())
        elf_file.close();
}

