#include "elffile.h"
#include <utility>

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

std::pair<void*, QByteArray> ELF::extend_segment(const QByteArray &data) {
    // TODO: make a static object instead of constructing object during eeach return statement
    if (!parsed)
        return std::make_pair<void *, QByteArray>(nullptr, QByteArray());

    // go through all executable segments and find the best one to extend
    QList<std::pair<esize_t, void*> > load_seg; // all LOAD segments
    Elf32_Phdr *ph = nullptr;
    void *ph_without_meta = nullptr;
    for (esize_t i = 0; i < ph_num; ++i) {
        // doesn't matter which arch app is compiled for :)
        ph_without_meta = get_ph_header(i);
        ph = reinterpret_cast<Elf32_Phdr*>(ph_without_meta);
        if (!ph)
            return std::make_pair<void *, QByteArray>(nullptr, QByteArray());
        if (ph->p_type == PT_LOAD)
            load_seg.push_back(std::make_pair(i, ph_without_meta));
    }

    // there are no any LOAD segments in a file
    if (!load_seg.size())
        return std::make_pair<void *, QByteArray>(nullptr, QByteArray());

    int size = load_seg.size() - 1, i = 0;
    uint8_t align = (cls == classes::ELF32) ? sizeof(Elf32_Off) : sizeof(Elf64_Off),
            pad_pre, pad_post;

    do {
        // figure out size of space we need after end of the segment to align virtual address
        // check if it's eligible to extend a segment
        if (cls == classes::ELF32) {
            Elf32_Phdr *ph = reinterpret_cast<Elf32_Phdr*>(load_seg.at(i).second),
                       *phn = load_seg.size() < (i + 1) ?
                        reinterpret_cast<Elf32_Phdr*>(load_seg.at(i + 1).second) :
                        nullptr;

            pad_pre = align - ((ph->p_vaddr + ph->p_memsz) % align ? : align);

            // 1. if there is no next LOAD segment after current
            // 2. enough space between end of current segment in memory and next segment in memory
            if ((!phn || round_address_down(ph->p_vaddr + ph->p_memsz + pad_pre + data.size(),
                                           ph->p_align) <
                        round_address_down(phn->p_vaddr, phn->p_align)))
                continue;

            // pad if size of file and memory images are different
            pad_pre += (ph->p_memsz - ph->p_filesz);

            // pad next segment in file if exists to vaddr % align == offset % align
            if (phn) {
                pad_post = phn->p_align + (phn->p_offset % phn->p_align) -
                           ((phn->p_offset + data.size() + pad_pre) % phn->p_align);
            }
            // if next segment is absebt we need to pad size of new data to the size of pointer
            // need to increase a highest vaddr in file, because there is no next segment in file
            else {
                pad_post = align - ((pad_pre + data.size() + pad_post) % align ? : align);
                // TODO: change higher vaddr in file
            }

            // TODO: make a choice, if new result is better than previous one

        }
        else {
            Elf64_Phdr *ph = reinterpret_cast<Elf64_Phdr*>(load_seg.at(i).second);
            pad_pre = align - ((ph->p_vaddr + ph->p_memsz) % align ? : align);

            // TODO: the same as for 64 bits
        }
    } while(++i < size);

    // TODO: return value
}

bool ELF::write_to_file(const QString &fname, const QByteArray &data) const {
    QFile of(fname);
    if (!of.open(QFile::WriteOnly))
        return false;
    try {
        of.write(data.data(), data.length());
        of.close();
    }
    catch(const std::exception &) {
        of.close();
        return false;
    }
    return true;
}

bool ELF::write_to_file(const QString &fname) const {
    return write_to_file(fname, b_data);
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

Elf64_Xword ELF::round_address_down(ex_offset_t addr, ex_offset_t align) const {
    return addr - (addr % align);
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

