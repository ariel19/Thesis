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
    bool change_va = false;
    best_segment bs, cs; // best segment, current segment

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
    // uint8_t align = (cls == classes::ELF32) ? sizeof(Elf32_Off) : sizeof(Elf64_Off);
    uint32_t pad_pre, pad_post;

    do {
        // figure out size of space we need after end of the segment to align virtual address
        // check if it's eligible to extend a segment
        if (cls == classes::ELF32) {
            Elf32_Phdr *ph = reinterpret_cast<Elf32_Phdr*>(load_seg.at(i).second),
                       *phn = load_seg.size() < (i + 1) ?
                        reinterpret_cast<Elf32_Phdr*>(load_seg.at(i + 1).second) :
                        nullptr;
            /*
            pad_pre = align - ((ph->p_vaddr + ph->p_memsz) % align ? : align);

            // 1. if there is no next LOAD segment after current
            // 2. enough space between end of current segment in memory and next segment in memory
            if ((!phn || round_address_down(ph->p_vaddr + ph->p_memsz + pad_pre + data.size(),
                                           ph->p_align) <
                        round_address_down(phn->p_vaddr, phn->p_align)))
                continue;

            // pad if size of file and memory images are different
            pad_pre += (ph->p_memsz - ph->p_filesz);
            */

            if (!find_pre_pad(ph, phn, data.size(), &pad_pre))
                continue;

            /*
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
                change_va = true;
            }
            */

            // TODO: take into consideration function return value
            find_post_pad(ph, phn, data.size(), pad_pre, &pad_post, &change_va);

            // TODO: make a choice, if new result is better than previous one
            // TODO: probably change to function
            if (!bs.ph || ((bs.post_pad + bs.pre_pad) >= (pad_post + pad_pre))) {
                bs.ph = ph;
                bs.post_pad = pad_post;
                bs.pre_pad = pad_pre;
                bs.change_vma = change_va;
            }
        }
        else {
            Elf64_Phdr *ph = reinterpret_cast<Elf64_Phdr*>(load_seg.at(i).second),
                       *phn = load_seg.size() < (i + 1) ?
                        reinterpret_cast<Elf64_Phdr*>(load_seg.at(i + 1).second) :
                        nullptr;

            if (!find_pre_pad(ph, phn, data.size(), &pad_pre))
                continue;

            // TODO: take into consideration function return value
            find_post_pad(ph, phn, data.size(), pad_pre, &pad_post, &change_va);

            // TODO: make a choice, if new result is better than previous one
            if (!bs.ph || ((bs.post_pad + bs.pre_pad) >= (pad_post + pad_pre))) {
                bs.ph = ph;
                bs.post_pad = pad_post;
                bs.pre_pad = pad_pre;
                bs.change_vma = change_va;
            }
        }
    } while(++i < size);

    // no suitable segments were found
    if (!bs.ph)
        return std::make_pair<void *, QByteArray>(nullptr, QByteArray());

    // construct a new QByteArray
    QByteArray d = construct_data(data, bs);
    if (!d.size())
        return std::make_pair<void *, QByteArray>(nullptr, QByteArray());

    return std::pair<void*, QByteArray>(nullptr, d);
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
    // size_t len = b_data.length();

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

bool ELF::find_pre_pad(const Elf32_Phdr *ph, const Elf32_Phdr *phn, const int dsize, uint32_t *pre_pad) {
    uint8_t align = sizeof(Elf32_Off);

    if (!ph)
        return false;

    try {
        *pre_pad = align - ((ph->p_vaddr + ph->p_memsz) % align ? : align);

        // 1. if there is no next LOAD segment after current
        // 2. enough space between end of current segment in memory and next segment in memory
        if ((!phn || round_address_down(ph->p_vaddr + ph->p_memsz + (*pre_pad) + dsize,
                                    ph->p_align) < round_address_down(phn->p_vaddr, phn->p_align)))
            return false;
        // pad if size of file and memory images are different
        *pre_pad += (ph->p_memsz - ph->p_filesz);
    }
    catch(const std::exception &) {
        return false;
    }
    return true;
}

bool ELF::find_pre_pad(const Elf64_Phdr *ph, const Elf64_Phdr *phn, const int dsize, uint32_t *pre_pad) {
    uint8_t align = sizeof(Elf64_Off);

    if (!ph)
        return false;

    try {
        *pre_pad = align - ((ph->p_vaddr + ph->p_memsz) % align ? : align);

        // 1. if there is no next LOAD segment after current
        // 2. enough space between end of current segment in memory and next segment in memory
        if (!(!phn || round_address_down(ph->p_vaddr + ph->p_memsz + (*pre_pad) + dsize,
                                    ph->p_align) < round_address_down(phn->p_vaddr, phn->p_align)))
            return false;
        // pad if size of file and memory images are different
        *pre_pad += (ph->p_memsz - ph->p_filesz);
    }
    catch(const std::exception &) {
        return false;
    }
    return true;
}

bool ELF::find_post_pad(const Elf32_Phdr *ph, const Elf32_Phdr *phn, const int dsize,
                        const uint32_t pre_pad, uint32_t *post_pad, bool *change_vma) {

    // TODO: make align static
    uint8_t align = sizeof(Elf32_Off);
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
        // TODO: wtf post pad is taking into consideration???????
        else {
            *post_pad = align - ((pre_pad + dsize + *post_pad) % align ? : align);
            // TODO: change higher vaddr in file
            *change_vma = true;
        }
    }
    catch(const std::exception &) {
        return false;
    }
    return true;
}

bool ELF::find_post_pad(const Elf64_Phdr *ph, const Elf64_Phdr *phn, const int dsize,
                        const uint32_t pre_pad, uint32_t *post_pad, bool *change_vma) {

    // TODO: make align static
    uint8_t align = sizeof(Elf64_Off);
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
        // TODO: wtf post pad is taking into consideration???????
        else {
            *post_pad = align - ((pre_pad + dsize + *post_pad) % align ? : align);
            // TODO: change higher vaddr in file
            *change_vma = true;
        }
    }
    catch(const std::exception &) {
        return false;
    }
    return true;
}

QByteArray ELF::construct_data(const QByteArray &data, ELF::best_segment &bs) {
    if(!parsed)
        return QByteArray();

    uint32_t total_space = data.size() + bs.post_pad + bs.pre_pad;
    ex_offset_t file_off = 0, va = 0;

    if (cls == classes::ELF32) {
        Elf32_Phdr *ph = reinterpret_cast<Elf32_Phdr*>(bs.ph);
        // offset for new data in file
        file_off = ph->p_offset + ph->p_filesz;
        // virtual address of new data
        va = ph->p_vaddr + ph->p_filesz + bs.pre_pad;
    }
    if (cls == classes::ELF64) {
        Elf64_Phdr *ph = reinterpret_cast<Elf64_Phdr*>(bs.ph);
        // offset for new data in file
        file_off = ph->p_offset + ph->p_filesz;
        // virtual address of new data
        va = ph->p_vaddr + ph->p_filesz + bs.pre_pad;
    }

    // 1. copy data from part of file, till new offset part
    // 2. copy pre_pad size, data, post_pad size
    // 3. copy rest of data from file
    QByteArray new_b_data(b_data.data(), file_off);
    for (uint32_t i = 0; i < bs.pre_pad; ++i)
        new_b_data.append('\0');
    new_b_data.append(data);
    for (uint32_t i = 0; i < bs.post_pad; ++i)
        new_b_data.append('\0');
    new_b_data.append(b_data.data() + file_off);

    // fixing...
    fix_elf_header(new_b_data, file_off, total_space);
    fix_section_table(new_b_data, file_off, total_space);
    Elf64_Addr vaddr = fix_segment_table(new_b_data, file_off, bs.pre_pad + data.size());
    fix_vma(new_b_data);

    return new_b_data;
}

void ELF::fix_elf_header(QByteArray &data, ex_offset_t file_off, uint32_t insert_space) {
    if (cls == classes::ELF32) {
        Elf32_Ehdr *eh = reinterpret_cast<Elf32_Ehdr*>(data.data());
        if (eh->e_phoff >= file_off)
            eh->e_phoff += insert_space;
        if (eh->e_shoff >= file_off)
            eh->e_shoff += insert_space;
    }
    else {
        Elf64_Ehdr *eh = reinterpret_cast<Elf64_Ehdr*>(data.data());
        if (eh->e_phoff >= file_off)
            eh->e_phoff += insert_space;
        if (eh->e_shoff >= file_off)
            eh->e_shoff += insert_space;
    }
}

void ELF::fix_section_table(QByteArray &data, ex_offset_t file_off, uint32_t insert_space) {
    if (cls == classes::ELF32) {
        Elf32_Ehdr *eh = reinterpret_cast<Elf32_Ehdr*>(data.data());
        // if section header table exists
        if (eh->e_shoff) {
            Elf32_Shdr *sh = reinterpret_cast<Elf32_Shdr*>(data.data() + eh->e_shoff);
            for (Elf32_Word i = 0; i < eh->e_shnum; ++i) {
                if (!sh)
                    return;
                if (sh->sh_offset >= file_off)
                    sh->sh_offset += insert_space;

                // next section header
                sh = reinterpret_cast<Elf32_Shdr*>(reinterpret_cast<int8_t*>(sh) + eh->e_shentsize);
            }
        }
    }
    else {
        Elf64_Ehdr *eh = reinterpret_cast<Elf64_Ehdr*>(data.data());
        // if section header table exists
        if (eh->e_shoff) {
            Elf64_Shdr *sh = reinterpret_cast<Elf64_Shdr*>(data.data() + eh->e_shoff);
            for (Elf64_Word i = 0; i < eh->e_shnum; ++i) {
                if (!sh)
                    return;
                if (sh->sh_offset >= file_off)
                    sh->sh_offset += insert_space;

                // next section header
                sh = reinterpret_cast<Elf64_Shdr*>(reinterpret_cast<int8_t*>(sh) + eh->e_shentsize);
            }
        }
    }
}

Elf64_Addr ELF::fix_segment_table(QByteArray &data, ex_offset_t file_off, uint32_t payload_size) {
    Elf64_Addr va = 0;

    if (cls == classes::ELF32) {
        Elf32_Phdr *ph = nullptr;
        foreach (ex_offset_t fo, ph_idx) {
            ph = reinterpret_cast<Elf32_Phdr*>(data.data() + fo);
            if (!ph)
                return 0;

            if (ph->p_offset >= file_off)
                ph->p_offset += payload_size;

            // check if current one is extended segment
            if (ph->p_type == PT_LOAD && ph->p_offset + ph->p_filesz == file_off) {
                ph->p_filesz += payload_size;
                ph->p_memsz = ph->p_filesz;
                // set executable flag on segment (may provide to vulnerabilities)
                ph->p_flags |= PF_W | PF_X;

                va = ph->p_vaddr + ph->p_memsz;
            }
        }
    } else {
        Elf64_Phdr *ph = nullptr;
        foreach (ex_offset_t fo, ph_idx) {
            ph = reinterpret_cast<Elf64_Phdr*>(data.data() + fo);
            if (!ph)
                return 0;

            if (ph->p_offset >= file_off)
                ph->p_offset += payload_size;

            // check if current one is extended segment
            if (ph->p_type == PT_LOAD && ph->p_offset + ph->p_filesz == file_off) {
                ph->p_filesz += payload_size;
                ph->p_memsz = ph->p_filesz;
                // set executable flag on segment (may provide to vulnerabilities)
                ph->p_flags |= PF_W | PF_X;

                va = ph->p_vaddr + ph->p_memsz;
            }
        }
    }
    return va;
}

void ELF::fix_vma(QByteArray &data) {

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

