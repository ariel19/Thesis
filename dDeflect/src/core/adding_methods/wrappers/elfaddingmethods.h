#ifndef ELFADDINGMETHODS_H
#define ELFADDINGMETHODS_H

#include <core/adding_methods/wrappers/daddingmethods.h>

class ELFAddingMethods : public DAddingMethods
{
public:
    ELFAddingMethods(ELF *f);
    ~ELFAddingMethods();

    /**
     * @brief Metoda zabezpiecza plik, podany jako argument za pomocą wyspecyfikowanej metody.
     * @param elf plik do zabezpieczania.
     * @param inject_desc opis metody wstrzykiwania kodu.
     * @return True, jeżeli operacja się powiodła, False w innych przypadkach.
     */
    template <typename RegistersType>
    bool secure_elf(ELF &elf, const InjectDescription<RegistersType> &inject_desc);

private:
    enum class PlaceholderMnemonics {
        DDETECTIONHANDLER,
        DDETECTIONMETHOD,
        DDRET
    };

    /**
     * @brief Typy placeholderów.
     */
    enum class PlaceholderTypes {
        PARAM_PRE,
        PARAM_POST,
        PLACEHOLDER_PRE,
        PLACEHOLDER_POST
    };

    QMap<PlaceholderTypes, QString> placeholder_id;
    QMap<PlaceholderMnemonics, QString> placeholder_mnm;

    /**
     * @brief Metoda odpowiada za generowanie kodu dla dowolnego opakowania.
     * @param wrap klasa opisująca kawałek kodu do wygenerowania.
     * @param code wygenerowany kod.
     * @return True, jeżeli operacja się powiodła, False w innych przypadkach.
     */
    template <typename RegistersType>
    bool wrapper_gen_code(Wrapper<RegistersType> *wrap, QString &code);

    /**
     * @brief Metoda odpowiada za wypełnianie parametrów w podanym kodzie.
     * @param code kod.
     * @param params parametry.
     * @return ilośc zamienionych parametrów.
     */
    uint64_t fill_params(QString &code, const QMap<QString, QString> &params);

    /**
     * @brief Metoda odpowiada za wypełnianie placeholdera w podanym kodzie, za pomocą podanego kodu.
     * @param code kod.
     * @param gen_code kod, którym zostanie zamieniony placeholder.
     * @param plc_mnm placeholder.
     * @return ilośc zamienionych parametrów.
     */
    uint64_t fill_placeholders(QString &code, const QString &gen_code, PlaceholderMnemonics plc_mnm);

    /**
     * @brief Metoda odpowiada za kompilację kodu źródłowego assembly.
     * @param code2compile kod, który musi zostać skompilowany.
     * @param compiled_code skompilowany kod.
     * @return True, jeżeli operacja się powiodła, False w innych przypadkach.
     */
    bool compile(const QString &code2compile, QByteArray &compiled_code);

    /**
     * @brief Metoda odpowiada za pobieranie adresów z wyspecyfikowanych danych.
     * @param data tablica z adresami.
     * @param addr_size wielkość adresu w bajtach.
     * @param addr_list lista adresów.
     * @param except_list lista adresów, które nie trzeba dołączać do listy wynikowej.
     * @return True, jeżeli operacja się powiodła, False w innych przypadkach.
     */
    bool get_addresses(const QByteArray &data, uint8_t addr_size, QList<Elf64_Addr> &addr_list,
                       const QList<Elf64_Addr> &except_list);

    /**
     * @brief Metoda odpowiada za generowanie kodu dla funkcji, która zmienia prawa dostępu do strony pamięci.
     * @param vaddr adres wirtualny.
     * @param mem_size wielkość pamięci.
     * @param flags prawa dostępu do pamięci.
     * @param code wygenerowany kod.
     * @return True, jeżeli operacja się powiodła, False w innych przypadkach.
     */
    bool set_prot_flags_gen_code_x86(Elf32_Addr vaddr, Elf32_Word mem_size, Elf32_Word flags, QString &code);

    /**
     * @brief Metoda odpowiada za generowanie kodu dla funkcji, która zmienia prawa dostępu do strony pamięci.
     * @param vaddr adres wirtualny.
     * @param mem_size wielkość pamięci.
     * @param flags prawa dostępu do pamięci.
     * @param code wygenerowany kod.
     * @return True, jeżeli operacja się powiodła, False w innych przypadkach.
     */
    bool set_prot_flags_gen_code_x64(Elf64_Addr vaddr, Elf64_Xword mem_size, Elf64_Word flags, QString &code);
};

template <typename RegistersType>
bool ELFAddingMethods::wrapper_gen_code(Wrapper<RegistersType> *wrap, QString &code) {
    if (!wrap)
        return false;

    // check if ret is in used registers (if it is remove)
    if (wrap->used_regs.indexOf(wrap->ret) != -1)
        wrap->used_regs.removeAll(wrap->ret);

    // generate push registers
    code.append(AsmCodeGenerator::push_regs<RegistersType>(wrap->used_regs));

    code.append(wrap->code);
    // fill params
    uint64_t filled_params = fill_params(code, wrap->static_params);

    // generate pop registers
    QList<RegistersType> rused_args;
    rused_args.reserve(wrap->used_regs.size());
    std::reverse_copy(wrap->used_regs.begin(), wrap->used_regs.end(), std::back_inserter(rused_args));

    code.append(AsmCodeGenerator::pop_regs<RegistersType>(rused_args));

    return true;
}

template <typename RegistersType>
bool ELFAddingMethods::secure_elf(ELF &elf, const InjectDescription<RegistersType> &inject_desc) {
    QString code2compile,
            code_ddetect_handler,
            code_ddetect;
    QByteArray compiled_code;

    if (!elf.is_valid())
        return false;

    // check platform version
    if (std::is_same<RegistersType, Registers_x86>::value)
        code2compile.append(QString("%1\n").arg(arch_type[ArchitectureType::BITS32]));
    else if(std::is_same<RegistersType, Registers_x64>::value)
        code2compile.append(QString("%1\n").arg(arch_type[ArchitectureType::BITS64]));
    else return false;

    // add to params
    if (!inject_desc.adding_method)
        return false;

    // adding a param value for (?^_^ddret^_^?)
    inject_desc.adding_method->static_params[placeholder_mnm[PlaceholderMnemonics::DDRET]] = elf.is_x86() ?
                AsmCodeGenerator::get_reg<Registers_x86>(static_cast<Registers_x86>(inject_desc.adding_method->ret)) :
                AsmCodeGenerator::get_reg<Registers_x64>(static_cast<Registers_x64>(inject_desc.adding_method->ret));

    // 0. take code from input
    if (!wrapper_gen_code<RegistersType>(inject_desc.adding_method, code2compile))
        return false;

    // 1. generate code for handler
    if (!wrapper_gen_code<RegistersType>(inject_desc.adding_method->detect_handler, code_ddetect_handler))
        return false;

    // 2. generate code for debugger detection method
    switch (inject_desc.cm) {
    case CallingMethod::OEP: {
        OEPWrapper<RegistersType> *oepwrapper =
                dynamic_cast<OEPWrapper<RegistersType>*>(inject_desc.adding_method);
        if (!oepwrapper)
            return false;
        if (!wrapper_gen_code<RegistersType>(oepwrapper->oep_action, code_ddetect))
            return false;
        break;
    }
    case CallingMethod::Thread: {
        ThreadWrapper<RegistersType> *twrapper =
                dynamic_cast<ThreadWrapper<RegistersType>*>(inject_desc.adding_method);
        if (!twrapper)
            return false;
        if (!wrapper_gen_code<RegistersType>(twrapper->thread_actions[0], code_ddetect))
            return false;
        break;
    }
    case CallingMethod::Trampoline:
    case CallingMethod::INIT_ARRAY:
    case CallingMethod::CTORS :
    case CallingMethod::INIT: {
        TrampolineWrapper<RegistersType> *trmwrapper =
                dynamic_cast<TrampolineWrapper<RegistersType>*>(inject_desc.adding_method);
        if (!trmwrapper)
            return false;
        if (!wrapper_gen_code<RegistersType>(trmwrapper->tramp_action, code_ddetect))
            return false;
        break;
    }

    default:
        return false;
    }

    // 3. merge code
    fill_placeholders(code2compile, code_ddetect_handler, PlaceholderMnemonics::DDETECTIONHANDLER);

    fill_placeholders(code2compile, code_ddetect, PlaceholderMnemonics::DDETECTIONMETHOD);

    // qDebug() << code2compile;

    Elf64_Addr oldep;
    if (!elf.get_entry_point(oldep))
        return false;

    // add jump to old original entry point
    switch(inject_desc.cm) {
    case CallingMethod::Thread:
    case CallingMethod::OEP:
        if (elf.is_x86()) {
            code2compile.append(AsmCodeGenerator::mov_reg_const<Registers_x86>(Registers_x86::EAX, oldep));
            code2compile.append(AsmCodeGenerator::jmp_reg<Registers_x86>(Registers_x86::EAX));
        }
        else {
            code2compile.append(AsmCodeGenerator::mov_reg_const<Registers_x64>(Registers_x64::RAX, oldep));
            code2compile.append(AsmCodeGenerator::jmp_reg<Registers_x64>(Registers_x64::RAX));
        }
        break;
    case CallingMethod::CTORS:
        break;
    case CallingMethod::INIT:
        break;
    case CallingMethod::INIT_ARRAY:
        break;
    default:
        return false;
    }

    // 4. compile code
    if (!compile(code2compile, compiled_code))
        return false;

    // 5. secure elf file

    QByteArray nf;
    Elf64_Addr nva;

    switch(inject_desc.cm) {
    case CallingMethod::Thread:
    case CallingMethod::OEP: {
        nf = elf.extend_segment(compiled_code, inject_desc.change_x_only, nva);
        if (!nf.length())
            return false;

        if (!elf.set_entry_point(nva, nf))
            return false;

        qDebug() << "new entry point: " << QString("0x%1").arg(nva, 0, 16);
        break;
    }
    case CallingMethod::CTORS: {
        QPair<QByteArray, Elf64_Addr> section_data;
        if (!elf.get_section_content(elf.get_elf_content(), ELF::SectionType::CTORS, section_data))
            return false;

        QList<Elf64_Addr> addresses;
        uint8_t addr_size = elf.is_x86() ? sizeof(Elf32_Addr) : sizeof(Elf64_Addr);
        if (!get_addresses(section_data.first, addr_size, addresses, { 0, 0xffffffff, 0xffffffffffffffff }))
            return false;

        // no place to store our pointer
        if (!addresses.size())
            return false;

        // TODO: address should be randomized, not always 0
        int idx = 0;

        // compiled_code += jmp to old address

        if (elf.is_x86()) {
            compiled_code.append(CodeDefines<Registers_x86>::movValueToReg<Elf32_Addr>(addresses.at(idx), Registers_x86::EAX));
            compiled_code.append(CodeDefines<Registers_x86>::jmpReg(Registers_x86::EAX));
        }
        else {
            compiled_code.append(CodeDefines<Registers_x64>::movValueToReg<Elf64_Addr>(addresses.at(idx), Registers_x64::RAX));
            compiled_code.append(CodeDefines<Registers_x64>::jmpReg(Registers_x64::RAX));
        }

        nf = elf.extend_segment(compiled_code, inject_desc.change_x_only, nva);
        if (!nf.length())
            return false;

        // set section content, set filler for elf function
        section_data.first.replace(idx * addr_size, addr_size, QByteArray(reinterpret_cast<const char *>(&nva), addr_size));

        if (!elf.set_section_content(nf, ELF::SectionType::CTORS, section_data.first))
            return false;

        qDebug() << "data added at: " << QString("0x%1").arg(nva, 0, 16);

        break;
    }
    case CallingMethod::INIT: {
        QPair<QByteArray, Elf64_Addr> section_data;
        if (!elf.get_section_content(elf.get_elf_content(), ELF::SectionType::INIT, section_data))
            return false;

        int prot_flags;
        Elf64_Addr align;

        // get relevant info
        if (!elf.get_segment_prot_flags(section_data.second, prot_flags))
            return false;
        if (!elf.get_segment_align(section_data.second, align))
            return false;

        // whole code will look like:
        // detection code
        // jmp to copy to init code
        // init code
        // change memory protection +w
        // copy to init code
        // restore old memory protection
        // jmp to init code

        // compiled_code = debugger detection + jmp to copy routine
        compiled_code.append(CodeDefines<Registers_x86>::saveRegister(Registers_x86::ESI));
        compiled_code.append(CodeDefines<Registers_x86>::saveRegister(Registers_x86::EDI));

        compiled_code.append(CodeDefines<Registers_x86>::callRelative(section_data.first.size()));
        // compiled_code = debugger detection + jmp to copy routine + previous init
        compiled_code.append(section_data.first);

        // copy routine
        if (elf.is_x86()) {
            // mov esi, src
            compiled_code.append(CodeDefines<Registers_x86>::restoreRegister(Registers_x86::ESI));
            // mov edi, dst
            compiled_code.append(CodeDefines<Registers_x86>::movValueToReg<Elf32_Addr>(section_data.second, Registers_x86::EDI));
        }
        else {
            // mov rsi, src
            compiled_code.append(CodeDefines<Registers_x64>::restoreRegister(Registers_x64::RSI));
            // mov rdi, dst
            compiled_code.append(CodeDefines<Registers_x64>::movValueToReg<Elf64_Addr>(section_data.second, Registers_x64::RDI));
        }

        // TODO: change memory protection here to copy a data

        // change memory protect for page with init section
        QString mprotect_code;
        QByteArray mprotect_compiled;
        Elf64_Addr aligned_vaddr = section_data.second- (section_data.second % align);
        if (elf.is_x86()) {
            if (!set_prot_flags_gen_code_x86(aligned_vaddr, section_data.second - aligned_vaddr + section_data.first.size(),
                                             prot_flags | PF_W, mprotect_code))
                return false;
        }
        else if (!set_prot_flags_gen_code_x64(aligned_vaddr, section_data.second - aligned_vaddr + section_data.first.size(),
                                              prot_flags | PF_W, mprotect_code))
                return false;

        if (!compile(mprotect_code, mprotect_compiled))
            return false;

        compiled_code.append(mprotect_compiled);

        compiled_code.append(CodeDefines<Registers_x86>::saveRegister(Registers_x86::ECX));
        compiled_code.append(CodeDefines<Registers_x86>::movValueToReg<Elf32_Addr>(section_data.first.size(), Registers_x86::ECX));
        // rep movsb
        compiled_code.append("\xf3\xa4", 2);

        // TODO: restore memory protect flags

        compiled_code.append(CodeDefines<Registers_x86>::restoreRegister(Registers_x86::ECX));
        compiled_code.append(CodeDefines<Registers_x86>::restoreRegister(Registers_x86::EDI));
        compiled_code.append(CodeDefines<Registers_x86>::restoreRegister(Registers_x86::ESI));

        // jmp to init
        if (elf.is_x86()) {
            compiled_code.append(CodeDefines<Registers_x86>::movValueToReg<Elf64_Addr>(section_data.second, Registers_x86::EAX));
            compiled_code.append(CodeDefines<Registers_x86>::jmpReg(Registers_x86::EAX));
        }
        else {
            compiled_code.append(CodeDefines<Registers_x64>::movValueToReg<Elf64_Addr>(section_data.second, Registers_x64::RAX));
            compiled_code.append(CodeDefines<Registers_x64>::jmpReg(Registers_x64::RAX));
        }

        nf = elf.extend_segment(compiled_code, inject_desc.change_x_only, nva);
        if (!nf.length())
            return false;

        // change section content
        QByteArray init_section_code;
        if (elf.is_x86()) {
            init_section_code.append(QString("%1\n").arg(arch_type[ArchitectureType::BITS32]));
            init_section_code.append(AsmCodeGenerator::mov_reg_const<Registers_x86>(Registers_x86::EAX, nva));
            init_section_code.append(AsmCodeGenerator::jmp_reg<Registers_x86>(Registers_x86::EAX));
        }
        else {
            init_section_code.append(QString("%1\n").arg(arch_type[ArchitectureType::BITS64]));
            init_section_code.append(AsmCodeGenerator::mov_reg_const<Registers_x64>(Registers_x64::RAX, nva));
            init_section_code.append(AsmCodeGenerator::jmp_reg<Registers_x64>(Registers_x64::RAX));
        }

        QByteArray compiled_jmp;
        if (!compile(init_section_code, compiled_jmp))
            return false;

        if (!elf.set_section_content(nf, ELF::SectionType::INIT, compiled_jmp, '\x90'))
            return false;

        qDebug() << "data added at: " << QString("0x%1").arg(nva, 0, 16);

        break;
    }
    case CallingMethod::INIT_ARRAY: {
        QPair<QByteArray, Elf64_Addr> section_data;
        if (!elf.get_section_content(elf.get_elf_content(), ELF::SectionType::INIT_ARRAY, section_data))
            return false;

        QList<Elf64_Addr> addresses;
        uint8_t addr_size = elf.is_x86() ? sizeof(Elf32_Addr) : sizeof(Elf64_Addr);
        if (!get_addresses(section_data.first, addr_size, addresses, { 0 }))
            return false;

        // no place to store our pointer
        if (!addresses.size())
            return false;

        // TODO: address should be randomized, not always 0
        int idx = 0;

        // compiled_code += jmp to old address

        if (elf.is_x86()) {
            compiled_code.append(CodeDefines<Registers_x86>::movValueToReg<Elf32_Addr>(addresses.at(idx), Registers_x86::EAX));
            compiled_code.append(CodeDefines<Registers_x86>::jmpReg(Registers_x86::EAX));
        }
        else {
            compiled_code.append(CodeDefines<Registers_x64>::movValueToReg<Elf64_Addr>(addresses.at(idx), Registers_x64::RAX));
            compiled_code.append(CodeDefines<Registers_x64>::jmpReg(Registers_x64::RAX));
        }

        nf = elf.extend_segment(compiled_code, inject_desc.change_x_only, nva);
        if (!nf.length())
            return false;

        // set section content, set filler for elf function
        section_data.first.replace(idx * addr_size, addr_size, QByteArray(reinterpret_cast<const char *>(&nva), addr_size));

        if (!elf.set_section_content(nf, ELF::SectionType::INIT_ARRAY, section_data.first))
            return false;

        qDebug() << "data added at: " << QString("0x%1").arg(nva, 0, 16);

        break;
    }
    default:
        return false;
    }

    qDebug() << "saving to file: " << inject_desc.saved_fname;
    elf.write_to_file(inject_desc.saved_fname, nf);

    return true;
}

#endif // ELFADDINGMETHODS_H
