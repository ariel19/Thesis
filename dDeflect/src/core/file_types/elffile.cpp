#include <core/file_types/elffile.h>
#include <utility>
#include <cstring>

#define section_type_stringify(sec_type) \
    QString(".%1").arg(QString((std::string(#sec_type).substr(std::string(#sec_type).find_last_of(':') != std::string::npos ? \
    std::string(#sec_type).find_last_of(':') + 1 : 0)).c_str()).toLower())

/// init section type dictionary
QMap<ELF::SectionType, ELF::section_info> ELF::section_type = {
    { ELF::SectionType::CTORS,      ELF::section_info(section_type_stringify(ELF::SectionType::CTORS),      SHT_PROGBITS)   },
    { ELF::SectionType::INIT,       ELF::section_info(section_type_stringify(ELF::SectionType::INIT),       SHT_PROGBITS)   },
    { ELF::SectionType::INIT_ARRAY, ELF::section_info(section_type_stringify(ELF::SectionType::INIT_ARRAY), SHT_INIT_ARRAY) },
    { ELF::SectionType::TEXT,       ELF::section_info(section_type_stringify(ELF::SectionType::TEXT),       SHT_PROGBITS)   }
};

void* ELF::get_ph_seg_offset(uint32_t idx) {
    try {
        void *ph = __get_ph_header(idx);
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

bool ELF::extend_segment(const QByteArray &_data, bool only_x, Elf64_Addr &va, Elf64_Off &file_off) {
    if (!parsed)
        return false;

    QByteArray data(_data);
    // go through all executable segments and find the best one to extend
    QList<std::pair<esize_t, void*> > load_seg; // all LOAD segments
    Elf32_Phdr *ph = nullptr;
    void *ph_without_meta = nullptr;
    best_segment bs; // best segment
    int dsize = data.size(), i;

    dsize += 4 - (dsize % 4 ? : 4);
    dsize -= data.size();
    // pad data
    for (i = 0; i < dsize; ++i)
        data.append('\x00');

    for (esize_t i = 0; i < ph_num; ++i) {
        // doesn't matter which arch app is compiled for :)
        ph_without_meta = __get_ph_header(i);
        ph = reinterpret_cast<Elf32_Phdr*>(ph_without_meta);
        if (!ph)
            return false;
        if (ph->p_type == PT_LOAD)
            load_seg.push_back(std::make_pair(i, ph_without_meta));
    }

    // there are no any LOAD segments in a file
    if (!load_seg.size())
        return false;

    int size = load_seg.size();

    if (cls != classes::ELF32 && cls != classes::ELF64)
        return false;

    i = 0;
    do {
        // figure out size of space we need after end of the segment to align virtual address
        // check if it's eligible to extend a segment
        if (cls == classes::ELF32) {
            if (!__extend_segment_eligible<Elf32_Phdr, Elf32_Off>(bs, only_x, load_seg, i, data.size()))
                continue;
        }
        else {
            if (!__extend_segment_eligible<Elf64_Phdr, Elf64_Off>(bs, only_x, load_seg, i, data.size()))
                continue;
        }
    } while(++i < size);

    // no suitable segments were found
    if (!bs.ph)
        return false;

    // construct a new QByteArray
    std::pair<QByteArray, Elf64_Addr> d = __construct_data(data, bs, file_off);
    va = d.second;

    QByteArray old_b_data = b_data;
    b_data = d.first;

    if (!__parse()) {
        b_data = old_b_data;
        return false;
    }

    return true;
}

bool ELF::__write_to_file(const QString &fname, const QByteArray &data) const {
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
    return __write_to_file(fname, b_data);
}

bool ELF::__set_entry_point(const Elf64_Addr &entry_point, QByteArray &data, Elf64_Addr *old_ep) {
    Elf32_Ehdr *eh_86 = nullptr;
    Elf64_Ehdr *eh_64 = nullptr;

    switch(cls) {
    case classes::ELF32:
        eh_86 = reinterpret_cast<Elf32_Ehdr*>(data.data());
        try {
        if (old_ep)
            *old_ep = eh_86->e_entry;
        eh_86->e_entry = entry_point;
    }
        catch(const std::exception &) {
            return false;
        }
        return true;
    case classes::ELF64:
        eh_64 = reinterpret_cast<Elf64_Ehdr*>(data.data());
        try {
        if (old_ep)
            *old_ep = eh_64->e_entry;
        eh_64->e_entry = entry_point;
    }
        catch(const std::exception &) {
            return false;
        }
        return true;
    default:
        return false;
    }
}

bool ELF::set_entry_point(const Elf64_Addr &entry_point, Elf64_Addr *old_ep) {
    if (!parsed)
        return false;
    return __set_entry_point(entry_point, b_data, old_ep);
}

bool ELF::__get_entry_point(const QByteArray &data, Elf64_Addr &old_ep) const {
    switch(cls) {
    case classes::ELF32:
        try {
        old_ep = reinterpret_cast<const Elf32_Ehdr*>(data.data())->e_entry;
    }
        catch(const std::exception &) {
            return false;
        }
        return true;
    case classes::ELF64:
        try {
        old_ep = reinterpret_cast<const Elf64_Ehdr*>(data.data())->e_entry;
    }
        catch(const std::exception &) {
            return false;
        }
        return true;
    default:
        return false;
    }
}

bool ELF::get_entry_point(Elf64_Addr &old_ep) const {
    if (!parsed)
        return false;
    return __get_entry_point(b_data, old_ep);
}

bool ELF::get_section_content(ELF::SectionType sec_type, QPair<QByteArray, Elf64_Addr> &section_data) const {
    if (!parsed)
        return false;

    switch (cls) {
    case classes::ELF32:
        return __get_section_content<Elf32_Ehdr, Elf32_Shdr>(sec_type, section_data);
    case classes::ELF64:
        return __get_section_content<Elf64_Ehdr, Elf64_Shdr>(sec_type, section_data);
    default:
        return false;
    }
}

template <typename ElfHeaderType, typename ElfSectionHeaderType>
bool ELF::__get_section_content(ELF::SectionType sec_type, QPair<QByteArray, Elf64_Addr> &section_data) const {
    const ElfHeaderType *eh = reinterpret_cast<const ElfHeaderType*>(b_data.data());
    // if section header table exists
    if (!eh->e_shoff)
        return false;

    if (!section_type.contains(sec_type))
        return false;

    const ElfSectionHeaderType *sh =
            reinterpret_cast<const ElfSectionHeaderType*>(b_data.data() + eh->e_shoff);

    if (!sh)
        return false;

    const ElfSectionHeaderType *shstrtab = &sh[eh->e_shstrndx];

    // get section header string table
    if (!shstrtab)
        return false;

    const char *pshstrtab = reinterpret_cast<const char*>(b_data.data() + shstrtab->sh_offset);

    if (!pshstrtab)
        return false;

    // word size is the same for x64 and x86
    for (Elf32_Word i = 0; i < eh->e_shnum; ++i) {
        if (!sh)
            return false;

        // check if section is ours
        if (sh->sh_type == section_type[sec_type].sh_type &&
                section_type[sec_type].sh_name == QString(pshstrtab + sh->sh_name)) {

            section_data = QPair<QByteArray, Elf64_Addr>(QByteArray(b_data.data() + sh->sh_offset, sh->sh_size), sh->sh_addr);
            return true;
        }

        // next section header
        sh = reinterpret_cast<const ElfSectionHeaderType*>(reinterpret_cast<const char*>(sh) + eh->e_shentsize);
    }
    return false;
}

bool ELF::get_section_file_off(SectionType sec_type, Elf64_Addr &file_off) {
    if (!parsed)
        return false;

    switch (cls) {
    case classes::ELF32:
        return __get_section_file_off<Elf32_Ehdr, Elf32_Shdr>(sec_type, file_off);
    case classes::ELF64:
        return __get_section_file_off<Elf64_Ehdr, Elf64_Shdr>(sec_type, file_off);
    default:
        return false;
    }
}

template <typename ElfHeaderType, typename ElfSectionHeaderType>
bool ELF::__get_section_file_off(ELF::SectionType sec_type, Elf64_Addr &file_off) {
    const ElfHeaderType *eh = reinterpret_cast<const ElfHeaderType*>(b_data.data());
    // if section header table exists
    if (!eh->e_shoff)
        return false;

    if (!section_type.contains(sec_type))
        return false;

    const ElfSectionHeaderType *sh =
            reinterpret_cast<const ElfSectionHeaderType*>(b_data.data() + eh->e_shoff);

    if (!sh)
        return false;

    const ElfSectionHeaderType *shstrtab = &sh[eh->e_shstrndx];

    // get section header string table
    if (!shstrtab)
        return false;

    const char *pshstrtab = reinterpret_cast<const char*>(b_data.data() + shstrtab->sh_offset);

    if (!pshstrtab)
        return false;

    // word size is the same for x64 and x86
    for (Elf32_Word i = 0; i < eh->e_shnum; ++i) {
        if (!sh)
            return false;

        // check if section is ours
        if (sh->sh_type == section_type[sec_type].sh_type &&
                section_type[sec_type].sh_name == QString(pshstrtab + sh->sh_name)) {

            file_off = sh->sh_offset;
            return true;
        }

        // next section header
        sh = reinterpret_cast<const ElfSectionHeaderType*>(reinterpret_cast<const char*>(sh) + eh->e_shentsize);
    }
    return false;
}

bool ELF::set_section_content(ELF::SectionType sec_type, const QByteArray &section_data, const char filler) {
    // TODO:  kind of stupid check, everyone can specify data he wants
    if (!parsed)
        return false;

    switch (cls) {
    case classes::ELF32:
        return __set_section_content<Elf32_Ehdr, Elf32_Shdr>(sec_type, section_data, filler);
    case classes::ELF64:
        return __set_section_content<Elf64_Ehdr, Elf64_Shdr>(sec_type, section_data, filler);
    default:
        return false;
    }
}

bool ELF::set_relative_address(Elf64_Off file_off, Elf32_Addr rva) {
    if (!parsed || b_data.size() < file_off + sizeof(rva))
        return false;
    std::memcpy(b_data.data() + file_off, &rva, sizeof(rva));
    return true;
}

// TODO: should be the same with __get_section_content
template <typename ElfHeaderType, typename ElfSectionHeaderType>
bool ELF::__set_section_content(ELF::SectionType sec_type, const QByteArray &section_data, const char filler) {
    const ElfHeaderType *eh = reinterpret_cast<const ElfHeaderType*>(b_data.data());
    // if section header table exists
    if (!eh->e_shoff)
        return false;

    if (!section_type.contains(sec_type))
        return false;

    const ElfSectionHeaderType *sh =
            reinterpret_cast<const ElfSectionHeaderType*>(b_data.data() + eh->e_shoff);

    if (!sh)
        return false;

    const ElfSectionHeaderType *shstrtab = &sh[eh->e_shstrndx];

    // get section header string table
    if (!shstrtab)
        return false;

    const char *pshstrtab = reinterpret_cast<const char*>(b_data.data() + shstrtab->sh_offset);

    if (!pshstrtab)
        return false;
    // word size is the same for x64 and x86
    for (Elf32_Word i = 0; i < eh->e_shnum; ++i) {
        if (!sh)
            return false;

        // check if section is ours
        if (sh->sh_type == section_type[sec_type].sh_type &&
                section_type[sec_type].sh_name == QString(pshstrtab + sh->sh_name)) {

            // check if there is enough space to change data
            if (sh->sh_size < section_data.size())
                return false;

            // TODO: do with replace??? :)

            // fill with new data
            // pad section data. with nops, idk why, just with nops :)
            QByteArray new_section_data(b_data.data(), sh->sh_offset);
            new_section_data.append(section_data);
            // fill rest with nops
            new_section_data.append(QByteArray(sh->sh_size - section_data.size(), filler));
            new_section_data.append(b_data.data() + sh->sh_offset + sh->sh_size,
                                    b_data.size() - sh->sh_offset - sh->sh_size);

            QByteArray old_b_data = b_data;

            b_data = new_section_data;

            if (!__parse()) {
                b_data = old_b_data;
                return false;
            }

            return true;
        }

        // next section header
        sh = reinterpret_cast<const ElfSectionHeaderType*>(reinterpret_cast<const char*>(sh) + eh->e_shentsize);
    }
    return false;
}

bool ELF::__get_ph_addresses() {
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

void* ELF::__get_elf_header() {
    try {
        return is_valid() ?
                    reinterpret_cast<void*>(&(b_data.data()[elf_header_idx])) :
                    nullptr;
    }
    catch(const std::exception &) {
        return nullptr;
    }
}

void* ELF::__get_ph_header(uint32_t idx) {
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

bool ELF::__check_magic(const Elf32_Ehdr *elf_hdr) const {
    if (!elf_hdr)
        return false;

    try {
        return  (elf_hdr->e_ident[EI_MAG0] == ELFMAG0) &
                (elf_hdr->e_ident[EI_MAG1] == ELFMAG1) &
                (elf_hdr->e_ident[EI_MAG2] == ELFMAG2) &
                (elf_hdr->e_ident[EI_MAG3] == ELFMAG3);
    }
    catch(const std::exception &) {
        return false;
    }
}

bool ELF::__is_supported(const Elf32_Ehdr *elf_hdr) {
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
        if (elf_hdr->e_machine != EM_X86_64 &&
                elf_hdr->e_machine != EM_386)
            return false;

        // file is non-executable
        if (elf_hdr->e_type != ET_EXEC && elf_hdr->e_type != ET_DYN)
            return false;
    }
    catch(const std::exception &) {
        return false;
    }

    return true;
}

template <typename ElfProgramHeader>
void ELF::__best_segment_choose(best_segment &bs, bool only_x, ElfProgramHeader *ph,
                         uint32_t pad_post, uint32_t pad_pre, bool change_va) {
    if (!bs.ph || ((bs.post_pad + bs.pre_pad) >= (pad_post + pad_pre))) {
        if (!only_x || (only_x && (ph->p_flags & PF_X))) {
            bs.ph = ph;
            bs.post_pad = pad_post;
            bs.pre_pad = pad_pre;
            bs.change_vma = change_va;
        }
    }
}

template <typename ElfProgramHeaderType, typename ElfOffsetType>
bool ELF::__extend_segment_eligible(best_segment &bs, bool only_x, const QList<std::pair<esize_t, void *> > &load_seg,
                                  int i, const int data_size) {

    bool change_va = false;
    uint32_t pad_pre, pad_post;

    ElfProgramHeaderType *ph = reinterpret_cast<ElfProgramHeaderType*>(load_seg.at(i).second),
            *phn = ((i + 1) < load_seg.size()) ?
                reinterpret_cast<ElfProgramHeaderType*>(load_seg.at(i + 1).second) :
                nullptr;

    if (!__find_pre_pad<ElfProgramHeaderType, ElfOffsetType>(ph, phn, data_size, &pad_pre))
        return false;

    if (!__find_post_pad<ElfProgramHeaderType, ElfOffsetType>(ph, phn, data_size,
                                                            pad_pre, &pad_post, &change_va))
        return false;

    __best_segment_choose<ElfProgramHeaderType>(bs, only_x, ph, pad_post, pad_pre, change_va);
    return true;
}

template <typename ElfHeaderType>
bool ELF::__get_ph_address(const void *elf_hdr) {
    try {
        const ElfHeaderType *e_hdr = reinterpret_cast<const ElfHeaderType*>(elf_hdr);
        ph_size = e_hdr->e_phentsize;
        ph_num = e_hdr->e_phnum;
        ph_idx.push_back(e_hdr->e_phoff);
        return true;
    }
    catch (const std::exception &) {
        return false;
    }
}

bool ELF::__get_ph_info(const void *elf_hdr) {
    if (!elf_hdr)
        return false;
    switch(cls) {
    case classes::ELF32:
        if (!__get_ph_address<Elf32_Ehdr>(elf_hdr))
            return false;
        break;
    case classes::ELF64:
        if (!__get_ph_address<Elf64_Ehdr>(elf_hdr))
            return false;
        break;
    default:
        return false;
    }

    // fill ph_idx list with according values
    return __get_ph_addresses();
}

bool ELF::__parse() {
    const char *data = b_data.data();

    try {
        // get ELF_header
        const Elf32_Ehdr *elf_hdr = reinterpret_cast<const Elf32_Ehdr*>(data);
        if (!__check_magic(elf_hdr))
            return false;
        // check if file format is supported
        if (!__is_supported(elf_hdr))
            return false;
        // get file info relevant to file
        if (!__get_ph_info(reinterpret_cast<const void*>(data)))
            return false;
    }
    catch (const std::exception &) {
        return false;
    }

    return true;
}

Elf64_Xword ELF::__round_address_down(ex_offset_t addr, ex_offset_t align) const {
    return addr - (addr % align);
}

template <typename ElfProgramHeaderType, typename ElfOffsetType>
bool ELF::__find_pre_pad(const ElfProgramHeaderType *ph, const ElfProgramHeaderType *phn,
                         const int dsize, uint32_t *pre_pad) {

    uint8_t align = sizeof(ElfOffsetType);

    if (!ph)
        return false;

    try {
        *pre_pad = align - ((ph->p_vaddr + ph->p_memsz) % align ? : align);

        // 1. if there is no next LOAD segment after current
        // 2. enough space between end of current segment in memory and next segment in memory
        if (!(!phn || __round_address_down(ph->p_vaddr + ph->p_memsz + (*pre_pad) + dsize,
                                    ph->p_align) < __round_address_down(phn->p_vaddr, phn->p_align)))
            return false;
        // pad if size of file and memory images are different
        *pre_pad += (ph->p_memsz - ph->p_filesz);
    }
    catch(const std::exception &) {
        return false;
    }
    return true;

    return true;
}

template <typename ElfProgramHeaderType, typename ElfOffsetType>
bool ELF::__find_post_pad(const ElfProgramHeaderType *ph, const ElfProgramHeaderType *phn,
                          const int dsize, const uint32_t pre_pad, uint32_t *post_pad,
                          bool *change_vma) {

    uint8_t align = sizeof(ElfOffsetType);
    if (!ph)
        return false;

    try {
        // pad next segment in file if exists to vaddr % align == offset % align
        if (phn) {
            *post_pad = phn->p_align + (phn->p_offset % phn->p_align) -
                    ((phn->p_offset + dsize + pre_pad) % phn->p_align);
        }
        // if next segment is absebt we need to pad size of new data to the size of pointer
        // need to increase a highest vaddr in file, because there is no next segment in file
        else {
            *post_pad = align - ((pre_pad + dsize + *post_pad) % align ? : align);
            *change_vma = true;
        }
    }
    catch(const std::exception &) {
        return false;
    }
    return true;

}

template <typename ElfProgramHeaderType>
std::pair<ex_offset_t, ex_offset_t> ELF::__get_new_data_va_fo(ELF::best_segment &bs) {
    ElfProgramHeaderType *ph = reinterpret_cast<ElfProgramHeaderType*>(bs.ph);
    // offset for new data in file
    // virtual address of new data
    return std::make_pair(ph->p_offset + ph->p_filesz, ph->p_vaddr + ph->p_filesz + bs.pre_pad);
}

std::pair<QByteArray, Elf64_Addr> ELF::__construct_data(const QByteArray &data, ELF::best_segment &bs, Elf64_Off &fo) {
    static std::pair<QByteArray, Elf64_Addr> failed(QByteArray(),  0);
    if (!parsed)
        return failed;

    uint32_t total_space = data.size() + bs.post_pad + bs.pre_pad;
    std::pair<ex_offset_t, ex_offset_t> fo_va;
    ex_offset_t file_off = 0, va = 0;

    switch(cls) {
    case classes::ELF32:
        fo_va = __get_new_data_va_fo<Elf32_Phdr>(bs);
        break;
    case classes::ELF64:
        fo_va = __get_new_data_va_fo<Elf64_Phdr>(bs);
        break;
    default:
        return std::make_pair(QByteArray(), 0);
    }

    file_off = fo_va.first;
    va = fo_va.second;

    // 1. copy data from part of file, till new offset part
    // 2. copy pre_pad size, data, post_pad size
    // 3. copy rest of data from file
    QByteArray new_b_data(b_data.data(), file_off);
    for (uint32_t i = 0; i < bs.pre_pad; ++i)
        new_b_data.append('\0');
    new_b_data.append(data);
    for (uint32_t i = 0; i < bs.post_pad; ++i)
        new_b_data.append('\0');
    new_b_data.append(b_data.data() + file_off, b_data.size() - file_off);

    fo = file_off + bs.pre_pad;

    // fixing...
    Elf64_Addr vaddr;
    switch(cls) {
    case classes::ELF32:
        __fix_elf_header<Elf32_Ehdr>(new_b_data, file_off, total_space);
        __fix_section_table<Elf32_Ehdr, Elf32_Shdr>(new_b_data, file_off, total_space);
        vaddr = __fix_segment_table<Elf32_Phdr>(new_b_data, file_off, total_space, bs.pre_pad + data.size());
        __fix_vma<Elf32_Dyn, Elf32_Sym, Elf32_Word>(new_b_data, bs, file_off, vaddr);
        break;
    case classes::ELF64:
        __fix_elf_header<Elf64_Ehdr>(new_b_data, file_off, total_space);
        __fix_section_table<Elf64_Ehdr, Elf64_Shdr>(new_b_data, file_off, total_space);
        vaddr = __fix_segment_table<Elf64_Phdr>(new_b_data, file_off, total_space, bs.pre_pad + data.size());
        __fix_vma<Elf64_Dyn, Elf64_Sym, Elf64_Word>(new_b_data, bs, file_off, vaddr);
        break;
    default:
        return failed;
    }

    return std::make_pair(new_b_data, va);
}

template <typename ElfHeaderType>
void ELF::__fix_elf_header(QByteArray &data, ex_offset_t file_off, uint32_t insert_space) {
    ElfHeaderType *eh = reinterpret_cast<ElfHeaderType*>(data.data());
    if (eh->e_phoff >= file_off)
        eh->e_phoff += insert_space;
    if (eh->e_shoff >= file_off)
        eh->e_shoff += insert_space;
}

template <typename ElfHeaderType, typename ElfSectionHeaderType>
void ELF::__fix_section_table(QByteArray &data, const ex_offset_t file_off, const uint32_t insert_space) {
    ElfHeaderType *eh = reinterpret_cast<ElfHeaderType*>(data.data());
    // if section header table exists
    if (eh->e_shoff) {
        ElfSectionHeaderType *sh =
                reinterpret_cast<ElfSectionHeaderType*>(data.data() + eh->e_shoff);
        // word size is the same for x64 and x86
        for (Elf32_Word i = 0; i < eh->e_shnum; ++i) {
            if (!sh)
                return;
            if (sh->sh_offset >= file_off)
                sh->sh_offset += insert_space;

            // next section header
            sh = reinterpret_cast<ElfSectionHeaderType*>(reinterpret_cast<int8_t*>(sh) + eh->e_shentsize);
        }
    }
}

template <typename ElfProgramHeaderType>
Elf64_Addr ELF::__fix_segment_table(QByteArray &data, const ex_offset_t file_off,
                                    const uint32_t insert_space, const uint32_t payload_size) {
    Elf64_Addr va = 0;

    ElfProgramHeaderType *ph = nullptr;
    foreach (ex_offset_t fo, ph_idx) {
        ph = reinterpret_cast<ElfProgramHeaderType*>(data.data() + fo);
        if (!ph)
            return 0;

        if (ph->p_offset >= file_off)
            ph->p_offset += insert_space;

        // check if current one is extended segment
        if (ph->p_type == PT_LOAD && ph->p_offset + ph->p_filesz == file_off) {
            ph->p_filesz += payload_size;
            ph->p_memsz = ph->p_filesz;
            // set executable flag on segment (may provide to vulnerabilities)
            // TODO: fuck of W
            ph->p_flags |= PF_X;

            va = ph->p_vaddr + ph->p_memsz;
        }
    }

    return va;
}

bool ELF::get_segment_prot_flags(const Elf64_Addr vaddr, unsigned int &prot_flags) const {
    if (!parsed)
        return false;

    switch (cls) {
    case classes::ELF32:
        return __get_segment_prot_flags<Elf32_Phdr>(vaddr, prot_flags);
    case classes::ELF64:
        return __get_segment_prot_flags<Elf64_Phdr>(vaddr, prot_flags);
    default:
        return false;
    }

    return true;
}

template <typename ElfProgramHeaderType>
bool ELF::__get_segment_prot_flags(const Elf64_Addr vaddr, unsigned int &prot_flags) const {

    const ElfProgramHeaderType *ph = nullptr;
    foreach (ex_offset_t fo, ph_idx) {
        ph = reinterpret_cast<const ElfProgramHeaderType*>(b_data.data() + fo);
        if (!ph)
            return false;

        if (ph->p_vaddr <= vaddr && ph->p_align + ph->p_vaddr > vaddr) {
            prot_flags = ph->p_flags;
            return true;
        }
    }

    return false;
}

bool ELF::get_segment_align(const Elf64_Addr vaddr, Elf64_Addr &align) const {
    if (!parsed)
        return false;

    switch (cls) {
    case classes::ELF32:
        return __get_segment_align<Elf32_Phdr>(vaddr, align);
    case classes::ELF64:
        return __get_segment_align<Elf64_Phdr>(vaddr, align);
    default:
        return false;
    }

    return true;
}

template <typename ElfProgramHeaderType>
bool ELF::__get_segment_align(const Elf64_Addr vaddr, Elf64_Addr &align) const {

    const ElfProgramHeaderType *ph = nullptr;
    foreach (ex_offset_t fo, ph_idx) {
        ph = reinterpret_cast<const ElfProgramHeaderType*>(b_data.data() + fo);
        if (!ph)
            return false;

        if (ph->p_vaddr <= vaddr && ph->p_align + ph->p_vaddr > vaddr) {
            align = ph->p_align;
            return true;
        }
    }

    return false;
}

bool ELF::get_load_segment_info(int prot_flags, QPair<QByteArray, Elf64_Addr> &segment_data) const {
    if (!parsed)
        return false;

    switch (cls) {
    case classes::ELF32:
        return __get_load_segment_info<Elf32_Phdr>(prot_flags, segment_data);
    case classes::ELF64:
        return __get_load_segment_info<Elf64_Phdr>(prot_flags, segment_data);
    default:
        return false;
    }

    return true;
}

bool ELF::get_relative_address(Elf64_Off file_off, int32_t &rva) const {
    if (!parsed || b_data.size() < file_off + sizeof(rva))
        return false;

    std::memcpy(&rva, b_data.data() + file_off, sizeof(rva));
    return true;
}

template <typename ElfProgramHeaderType>
bool ELF::__get_load_segment_info(int prot_flags, QPair<QByteArray, Elf64_Addr> &segment_data) const {

    const ElfProgramHeaderType *ph = nullptr;
    foreach (ex_offset_t fo, ph_idx) {
        ph = reinterpret_cast<const ElfProgramHeaderType*>(b_data.data() + fo);
        if (!ph)
            return false;

        if (ph->p_type == PT_LOAD && prot_flags & ph->p_flags) {
            segment_data = QPair<QByteArray, Elf64_Addr>(QByteArray(b_data.data() + ph->p_offset, ph->p_memsz), ph->p_vaddr);
            return true;
        }
    }

    return false;
}

template <typename ElfDynType, typename ElfSymType, typename ElfWordType>
void ELF::__fix_vma(QByteArray &data, const best_segment &bs,
                    ex_offset_t file_off, const Elf64_Addr &new_vma) {

    if (!bs.change_vma)
        return;

    ElfDynType *dyn = reinterpret_cast<ElfDynType*>(data.data() + file_off + bs.pre_pad);
    ElfSymType *base_sym = nullptr,
            *sym = nullptr;
    ElfWordType *buckets = nullptr,
            *chains = nullptr;
    char *dyn_str = nullptr;

    struct {
        ElfWordType buckets_no;
        ElfWordType chains_no;
    } *hash;

    // TODO: implement
    if (!dyn)
        return;

    // find 3 types of segments before end of dynamic segment
    for (; dyn->d_tag != DT_NULL; ++dyn) {
        switch(dyn->d_tag) {
        case DT_STRTAB:
            //dyn_str = get_file_offset(data, dyn->d_un.d_ptr);
            break;
        case DT_SYMTAB:
            //base_sym = get_file_offset(data, dyn->d_un.d_ptr);
            break;
        case DT_HASH:
            //hash = get_file_offset(data, dyn->d_un.d_ptr);
            //buckets = (Elf32_Word *)((char *)hash + sizeof(*hash));
            //chains = (buckets + hash->buckets_no);
            break;
        default:
            break;
        }
    }

    if (dyn && base_sym && hash && buckets && chains) {

    }
    /*
      if (pcDynStr && ptBaseElfSym && ptHashHeader && ptBuckets && ptChains) {
         for (ulChainIndex = ptBuckets[calc_elf_hash("_end") % ptHashHeader->tNoBuckets];
              ulChainIndex; ulChainIndex = ptChains[ulChainIndex]) {
            ptElfSym = ptBaseElfSym + ulChainIndex;

            if ((ptElfSym->st_name) &&
                (!strcmp("_end", pcDynStr + ptElfSym->st_name))) {
               printf("Moving _end from 0x%08x to 0x%08x\n",
                      ptElfSym->st_value, tEndVma);
               ptElfSym->st_value = tEndVma;
               break;
            }
         }
      }

    */
}

ELF::ELF(QByteArray _data) :
    BinaryFile(_data),
    cls(classes::NONE) {
    parsed = __parse();
}

ELF::~ELF() {

}
