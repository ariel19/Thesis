#include "elffile.h"

Elf32_Ehdr* ELF::getElfHeader() {
    try {
        return isValid() ?
               reinterpret_cast<Elf32_Ehdr*>(&(b_data.data()[elf_header_idx])) :
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

/* TODO: real parsing */
bool ELF::parse() {
    // check if file is ready for parsing
    if (!isValid())
        return false;

    char *data = b_data.data();
    size_t len = b_data.length();

    try {
        // get ELF_header
        Elf32_Ehdr* elf_hdr = reinterpret_cast<Elf32_Ehdr *>(data);
        if (!check_magic(elf_hdr))
            return false;



    }
    catch (const std::exception &) {
        return false;
    }

    return true;
}

ELF::ELF(QString _fname) :
    parsed(false), elf_file(_fname) {
    if (elf_file.open(QFile::ReadOnly)) {
        b_data = elf_file.readAll();
        parsed = parse();
    }
}

ELF::~ELF() {
    if (elf_file.isOpen())
        elf_file.close();
}

