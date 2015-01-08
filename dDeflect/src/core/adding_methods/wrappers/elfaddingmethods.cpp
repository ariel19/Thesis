#include "elfaddingmethods.h"

template <typename RegistersType>
ELFAddingMethods<RegistersType>::ELFAddingMethods(ELF *f) :
    DAddingMethods<RegistersType>(f)
{
    placeholder_id = {
        { PlaceholderTypes::PARAM_PRE,          QString("(?^_^")     },
        { PlaceholderTypes::PARAM_POST,         QString("^_^?)")     },
        { PlaceholderTypes::PLACEHOLDER_PRE,    QString("(rsj?^_^")  },
        { PlaceholderTypes::PLACEHOLDER_POST,   QString("^_^?rsj)")  }
    };

    placeholder_mnm = {
        { PlaceholderMnemonics::DDETECTIONHANDLER,  mnemonic_stringify(PlaceholderMnemonics::DDETECTIONHANDLER) },
        { PlaceholderMnemonics::DDETECTIONMETHOD,   mnemonic_stringify(PlaceholderMnemonics::DDETECTIONMETHOD)  },
        { PlaceholderMnemonics::DDRET,              mnemonic_stringify(PlaceholderMnemonics::DDRET)             }
    };
}
template ELFAddingMethods<Registers_x86>::ELFAddingMethods(ELF *f);
template ELFAddingMethods<Registers_x64>::ELFAddingMethods(ELF *f);

template <typename RegistersType>
ELFAddingMethods<RegistersType>::~ELFAddingMethods() {}
template ELFAddingMethods<Registers_x86>::~ELFAddingMethods();
template ELFAddingMethods<Registers_x64>::~ELFAddingMethods();

template <typename RegistersType>
uint64_t ELFAddingMethods<RegistersType>::fill_params(QString &code, const QMap<QString, QString> &params) {
    uint64_t cnt = 0;
    foreach (QString param, params.keys()) {
        QString plc_param(QString("%1%2%3").arg(placeholder_id[PlaceholderTypes::PARAM_PRE], param,
                          placeholder_id[PlaceholderTypes::PARAM_POST]));

        // look in string for plc_param cahnge it to value from dictionary
        code.replace(plc_param, params[param]);
    }
    return cnt;
}

template <typename RegistersType>
uint64_t ELFAddingMethods<RegistersType>::fill_placeholders(QString &code, const QString &gen_code, PlaceholderMnemonics plc_mnm) {
    uint64_t cnt = 0;

    code.replace(QString("%1%2%3").arg(placeholder_id[PlaceholderTypes::PLACEHOLDER_PRE], placeholder_mnm[plc_mnm],
                 placeholder_id[PlaceholderTypes::PLACEHOLDER_POST]), gen_code);

    return cnt;
}

template <typename RegistersType>
bool ELFAddingMethods<RegistersType>::compile(const QString &code2compile, QByteArray &compiled_code) {
    // TODO: change
    QFile file("tocompile.asm");
    if (!file.open(QIODevice::WriteOnly))
        return false;

    if (file.write(code2compile.toStdString().c_str(), code2compile.length()) != code2compile.length())
        return false;

    file.close();

    // TODO: change file names and etc.
    if (QProcess::execute("nasm", { "tocompile.asm" }))
        return false;

    file.setFileName("tocompile");

    if (!file.open(QIODevice::ReadOnly))
        return false;

    compiled_code = file.readAll();

    file.close();
    return true;
}

template <typename RegistersType>
bool ELFAddingMethods<RegistersType>::get_addresses(const QByteArray &addr_data, uint8_t addr_size, QList<Elf64_Addr> &addr_list,
                                                    const QList<Elf64_Addr> &except_list) {
    int data_size = addr_data.size();
    if (data_size % addr_size)
        return false;

    quint64 addr;

    for (int i = 0; i < data_size; i += addr_size) {
        addr = 0;
        for (int j = 0; j < addr_size; ++j)
            addr |= (static_cast<const char>((addr_data.data())[i + j]) & 0xff) << (j * 8);
        if (!except_list.contains(addr))
            addr_list.push_back(addr);
    }

    return true;
}

template <typename RegistersType>
bool ELFAddingMethods<RegistersType>::wrapper_gen_code(typename DAddingMethods<RegistersType>::Wrapper *wrap, QString &code) {
    if (!wrap)
        return false;

    // TODO: add check if contains all registers
    /*
    if ((std::is_same<RegistersType, Registers_x86>::value && !wrap->used_regs.contains(Registers_x86::All)) ||
        (std::is_same<RegistersType, Registers_x64>::value && !wrap->used_regs.contains(Registers_x64::All)))
    */
    // add internal registers to one's we have to save
    foreach (RegistersType reg, CodeDefines<RegistersType>::internalRegs)
        if (!wrap->used_regs.contains(reg))
            wrap->used_regs.push_back(reg);

    // TODO: add check if emthod use all registers not to save ret one
    // check if ret is in used registers (if it is remove)
    if (wrap->used_regs.indexOf(wrap->ret) != -1)
        wrap->used_regs.removeAll(wrap->ret);

    // save flag register
    code.append(AsmCodeGenerator::save_flags<RegistersType>());

    // generate push registers
    code.append(AsmCodeGenerator::push_regs<RegistersType>(wrap->used_regs));

    code.append(wrap->code);
    // fill params
    uint64_t filled_params = fill_params(code, wrap->static_params);

    // generate pop registers
    QList<RegistersType> rused_args;
    rused_args.reserve(wrap->used_regs.size());
    std::reverse_copy(wrap->used_regs.begin(), wrap->used_regs.end(), std::back_inserter(rused_args));

    // restore registers
    code.append(AsmCodeGenerator::pop_regs<RegistersType>(rused_args));

    // restore flag register
    code.append(AsmCodeGenerator::restore_flags<RegistersType>());

    return true;
}
template bool ELFAddingMethods<Registers_x86>::wrapper_gen_code(DAddingMethods<Registers_x86>::Wrapper *wrap, QString &code);
template bool ELFAddingMethods<Registers_x64>::wrapper_gen_code(DAddingMethods<Registers_x64>::Wrapper *wrap, QString &code);

template <>
template <>
bool ELFAddingMethods<Registers_x86>::set_prot_flags_gen_code(Elf32_Addr vaddr, Elf32_Word mem_size,
                                                              Elf32_Word flags, QString &code) {
    // TODO: generated using JSON parser, here is stupid dummy solution
    DAddingMethods<Registers_x86>::Wrapper mprotect;

    // TODO: should be changed
    mprotect.detect_handler = nullptr;
    mprotect.used_regs = { Registers_x86::EAX, Registers_x86::EBX, Registers_x86::ECX, Registers_x86::EDX };
    mprotect.static_params = { { "vaddr", QString::number(vaddr) },
                               { "vsize", QString::number(mem_size) },
                               { "flags", QString::number(flags) } };
    mprotect.ret = Registers_x86::None;

    QFile protect("mprotect.asm");
    if (!protect.open(QIODevice::ReadOnly))
        return false;

    mprotect.code = protect.readAll();
    code = QString("%1\n").arg(arch_type[ArchitectureType::BITS32]);

    return wrapper_gen_code(&mprotect, code);
}

template <>
template <>
bool ELFAddingMethods<Registers_x64>::set_prot_flags_gen_code(Elf64_Addr vaddr, Elf64_Xword mem_size,
                                                              Elf64_Word flags, QString &code) {
    // TODO: generated using JSON parser, here is stupid dummy solution
    DAddingMethods<Registers_x64>::Wrapper mprotect;

    // TODO: should be changed
    mprotect.detect_handler = nullptr;
    mprotect.used_regs = { Registers_x64::RAX, Registers_x64::RDX, Registers_x64::RSI, Registers_x64::RDI };
    mprotect.static_params = { { "vaddr", QString::number(vaddr) },
                               { "vsize", QString::number(mem_size) },
                               { "flags", QString::number(flags) } };
    mprotect.ret = Registers_x64::None;

    QFile protect("mprotect64.asm");
    if (!protect.open(QIODevice::ReadOnly))
        return false;

    mprotect.code = protect.readAll();
    code = QString("%1\n").arg(arch_type[ArchitectureType::BITS64]);

    return wrapper_gen_code(&mprotect, code);
}

template <>
template <>
bool ELFAddingMethods<Registers_x86>::set_prot_flags_gen_code(Elf64_Addr vaddr, Elf64_Xword mem_size,
                                                              Elf64_Word flags, QString &code) {
    return set_prot_flags_gen_code(static_cast<Elf32_Addr>(vaddr), static_cast<Elf32_Word>(mem_size),
                                   static_cast<Elf32_Word>(flags), code);
}

template <typename RegistersType>
void ELFAddingMethods<RegistersType>::get_file_offsets_from_opcodes(QStringList &opcodes, QList<Elf64_Addr> &file_off, Elf64_Addr base_off) {
    foreach(QString op, opcodes)
        file_off.append(op.mid(0, 8).toUInt(NULL, 16) + base_off + 1);
}

// ===============================================================================
// TODO: add detect method return value to wrapper used_regs, and push it on stack
// ===============================================================================
template <typename RegistersType>
bool ELFAddingMethods<RegistersType>::secure(const QList<typename DAddingMethods<RegistersType>::InjectDescription*> &inject_desc) {
    foreach(typename DAddingMethods<RegistersType>::InjectDescription* id, inject_desc) {
        if(secure_one(id))
            return false;
    }
    return true;
}

template <typename RegistersType>
bool ELFAddingMethods<RegistersType>::secure_one(typename DAddingMethods<RegistersType>::InjectDescription* i_desc) {
    QString code2compile,
            code_ddetect_handler,
            code_ddetect;
    QByteArray compiled_code;

    ELF *elf = dynamic_cast<ELF*>(DAddingMethods<RegistersType>::file);
    if(!elf)
        return false;

    if (!elf->is_valid())
        return false;

    // check platform version
    if (std::is_same<RegistersType, Registers_x86>::value)
        code2compile.append(QString("%1\n").arg(
                                DAddingMethods<RegistersType>::arch_type[DAddingMethods<RegistersType>::ArchitectureType::BITS32]));
    else if(std::is_same<RegistersType, Registers_x64>::value)
        code2compile.append(QString("%1\n").arg(
                                DAddingMethods<RegistersType>::arch_type[DAddingMethods<RegistersType>::ArchitectureType::BITS64]));
    else return false;

    // add to params
    if (!i_desc->adding_method || !i_desc->adding_method->detect_handler)
        return false;

    // adding a param value for (?^_^ddret^_^?)
    i_desc->adding_method->static_params[placeholder_mnm[PlaceholderMnemonics::DDRET]] = elf->is_x86() ?
                AsmCodeGenerator::get_reg<Registers_x86>(static_cast<Registers_x86>(i_desc->adding_method->ret)) :
                AsmCodeGenerator::get_reg<Registers_x64>(static_cast<Registers_x64>(i_desc->adding_method->ret));

    // make a wrapper save register that is used for debugger detection function to return value
    if (!i_desc->adding_method->used_regs.contains(i_desc->adding_method->ret))
        i_desc->adding_method->used_regs.push_back(i_desc->adding_method->ret);

    if (elf->is_x86())
        i_desc->adding_method->ret = static_cast<RegistersType>(Registers_x86::None);
    else
        i_desc->adding_method->ret = static_cast<RegistersType>(Registers_x64::None);

    // 0. take code from input
    if (!wrapper_gen_code(i_desc->adding_method, code2compile))
        return false;

    // 1. generate code for handler
    if (!wrapper_gen_code(i_desc->adding_method->detect_handler, code_ddetect_handler))
        return false;

    // 2. generate code for debugger detection method
    switch (i_desc->cm) {
    case DAddingMethods<RegistersType>::CallingMethod::OEP: {
        typename DAddingMethods<RegistersType>::OEPWrapper *oepwrapper =
                dynamic_cast<typename DAddingMethods<RegistersType>::OEPWrapper*>(i_desc->adding_method);
        if (!oepwrapper)
            return false;
        if (!wrapper_gen_code(oepwrapper->oep_action, code_ddetect))
            return false;
        break;
    }
    case DAddingMethods<RegistersType>::CallingMethod::Thread: {
        typename DAddingMethods<RegistersType>::ThreadWrapper *twrapper =
                dynamic_cast<typename DAddingMethods<RegistersType>::ThreadWrapper*>(i_desc->adding_method);
        if (!twrapper)
            return false;
        if (!wrapper_gen_code(twrapper->thread_actions[0], code_ddetect))
            return false;
        break;
    }
    case DAddingMethods<RegistersType>::CallingMethod::Trampoline:
    case DAddingMethods<RegistersType>::CallingMethod::INIT_ARRAY:
    case DAddingMethods<RegistersType>::CallingMethod::CTORS :
    case DAddingMethods<RegistersType>::CallingMethod::INIT: {
        typename DAddingMethods<RegistersType>::TrampolineWrapper *trmwrapper =
                dynamic_cast<typename DAddingMethods<RegistersType>::TrampolineWrapper*>(i_desc->adding_method);
        if (!trmwrapper)
            return false;
        if (!wrapper_gen_code(trmwrapper->tramp_action, code_ddetect))
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
    if (!elf->get_entry_point(oldep))
        return false;

    // add jump to old original entry point
    switch(i_desc->cm) {
    case DAddingMethods<RegistersType>::CallingMethod::Thread:
    case DAddingMethods<RegistersType>::CallingMethod::OEP:
        if (elf->is_x86()) {
            code2compile.append(AsmCodeGenerator::mov_reg_const<Registers_x86>(Registers_x86::EAX, oldep));
            code2compile.append(AsmCodeGenerator::jmp_reg<Registers_x86>(Registers_x86::EAX));
        }
        else {
            code2compile.append(AsmCodeGenerator::mov_reg_const<Registers_x64>(Registers_x64::RAX, oldep));
            code2compile.append(AsmCodeGenerator::jmp_reg<Registers_x64>(Registers_x64::RAX));
        }
        break;
    case DAddingMethods<RegistersType>::CallingMethod::CTORS:
        break;
    case DAddingMethods<RegistersType>::CallingMethod::INIT:
        break;
    case DAddingMethods<RegistersType>::CallingMethod::INIT_ARRAY:
        break;
    case DAddingMethods<RegistersType>::CallingMethod::Trampoline:
        break;
    default:
        return false;
    }

    // 4. compile code
    if (!compile(code2compile, compiled_code))
        return false;

    // 5. secure elf file

    Elf64_Addr nva;

    switch(i_desc->cm) {
    case DAddingMethods<RegistersType>::CallingMethod::Thread:
    case DAddingMethods<RegistersType>::CallingMethod::OEP: {
        if (!elf->extend_segment(compiled_code, i_desc->change_x_only, nva))
            return false;

        if (!elf->set_entry_point(nva))
            return false;

        qDebug() << "new entry point: " << QString("0x%1").arg(nva, 0, 16);
        break;
    }
    case DAddingMethods<RegistersType>::CallingMethod::CTORS: {
        QPair<QByteArray, Elf64_Addr> section_data;
        if (!elf->get_section_content(ELF::SectionType::CTORS, section_data))
            return false;

        QList<Elf64_Addr> addresses;
        uint8_t addr_size = elf->is_x86() ? sizeof(Elf32_Addr) : sizeof(Elf64_Addr);
        if (!get_addresses(section_data.first, addr_size, addresses, { 0, 0xffffffff, 0xffffffffffffffff }))
            return false;

        // no place to store our pointer
        if (!addresses.size())
            return false;

        // TODO: address should be randomized, not always 0
        int idx = 0;

        // compiled_code += jmp to old address

        if (elf->is_x86()) {
            compiled_code.append(CodeDefines<Registers_x86>::movValueToReg<Elf32_Addr>(addresses.at(idx), Registers_x86::EAX));
            compiled_code.append(CodeDefines<Registers_x86>::jmpReg(Registers_x86::EAX));
        }
        else {
            compiled_code.append(CodeDefines<Registers_x64>::movValueToReg<Elf64_Addr>(addresses.at(idx), Registers_x64::RAX));
            compiled_code.append(CodeDefines<Registers_x64>::jmpReg(Registers_x64::RAX));
        }

        if (!elf->extend_segment(compiled_code, i_desc->change_x_only, nva))
            return false;

        // set section content, set filler for elf function
        section_data.first.replace(idx * addr_size, addr_size, QByteArray(reinterpret_cast<const char *>(&nva), addr_size));

        if (!elf->set_section_content(ELF::SectionType::CTORS, section_data.first))
            return false;

        qDebug() << "data added at: " << QString("0x%1").arg(nva, 0, 16);

        break;
    }
    case DAddingMethods<RegistersType>::CallingMethod::INIT: {
        QPair<QByteArray, Elf64_Addr> section_data;
        if (!elf->get_section_content(ELF::SectionType::INIT, section_data))
            return false;

        unsigned int prot_flags;
        Elf64_Addr align;

        // get relevant info
        if (!elf->get_segment_prot_flags(section_data.second, prot_flags))
            return false;
        if (!elf->get_segment_align(section_data.second, align))
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
        if (elf->is_x86()) {
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
        if (!set_prot_flags_gen_code(aligned_vaddr, section_data.second - aligned_vaddr + section_data.first.size(),
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
        if (elf->is_x86()) {
            compiled_code.append(CodeDefines<Registers_x86>::movValueToReg<Elf64_Addr>(section_data.second, Registers_x86::EAX));
            compiled_code.append(CodeDefines<Registers_x86>::jmpReg(Registers_x86::EAX));
        }
        else {
            compiled_code.append(CodeDefines<Registers_x64>::movValueToReg<Elf64_Addr>(section_data.second, Registers_x64::RAX));
            compiled_code.append(CodeDefines<Registers_x64>::jmpReg(Registers_x64::RAX));
        }

        if (!elf->extend_segment(compiled_code, i_desc->change_x_only, nva))
            return false;

        // change section content
        QByteArray init_section_code;
        if (elf->is_x86()) {
            init_section_code.append(QString("%1\n").arg(
                                         DAddingMethods<RegistersType>::arch_type[DAddingMethods<RegistersType>::ArchitectureType::BITS32]));
            init_section_code.append(AsmCodeGenerator::mov_reg_const<Registers_x86>(Registers_x86::EAX, nva));
            init_section_code.append(AsmCodeGenerator::jmp_reg<Registers_x86>(Registers_x86::EAX));
        }
        else {
            init_section_code.append(QString("%1\n").arg(
                                         DAddingMethods<RegistersType>::arch_type[DAddingMethods<RegistersType>::ArchitectureType::BITS64]));
            init_section_code.append(AsmCodeGenerator::mov_reg_const<Registers_x64>(Registers_x64::RAX, nva));
            init_section_code.append(AsmCodeGenerator::jmp_reg<Registers_x64>(Registers_x64::RAX));
        }

        QByteArray compiled_jmp;
        if (!compile(init_section_code, compiled_jmp))
            return false;

        if (!elf->set_section_content(ELF::SectionType::INIT, compiled_jmp, '\x90'))
            return false;

        qDebug() << "data added at: " << QString("0x%1").arg(nva, 0, 16);

        break;
    }
    case DAddingMethods<RegistersType>::CallingMethod::INIT_ARRAY: {
        QPair<QByteArray, Elf64_Addr> section_data;
        if (!elf->get_section_content(ELF::SectionType::INIT_ARRAY, section_data))
            return false;

        QList<Elf64_Addr> addresses;
        uint8_t addr_size = elf->is_x86() ? sizeof(Elf32_Addr) : sizeof(Elf64_Addr);
        if (!get_addresses(section_data.first, addr_size, addresses, { 0 }))
            return false;

        // no place to store our pointer
        if (!addresses.size())
            return false;

        // TODO: address should be randomized, not always 0
        int idx = 0;

        // compiled_code += jmp to old address

        if (elf->is_x86()) {
            compiled_code.append(CodeDefines<Registers_x86>::movValueToReg<Elf32_Addr>(addresses.at(idx), Registers_x86::EAX));
            compiled_code.append(CodeDefines<Registers_x86>::jmpReg(Registers_x86::EAX));
        }
        else {
            compiled_code.append(CodeDefines<Registers_x64>::movValueToReg<Elf64_Addr>(addresses.at(idx), Registers_x64::RAX));
            compiled_code.append(CodeDefines<Registers_x64>::jmpReg(Registers_x64::RAX));
        }

        if (!elf->extend_segment(compiled_code, i_desc->change_x_only, nva))
            return false;

        // set section content, set filler for elf function
        section_data.first.replace(idx * addr_size, addr_size, QByteArray(reinterpret_cast<const char *>(&nva), addr_size));

        if (!elf->set_section_content(ELF::SectionType::INIT_ARRAY, section_data.first))
            return false;

        qDebug() << "data added at: " << QString("0x%1").arg(nva, 0, 16);

        break;
    }
    case DAddingMethods<RegistersType>::CallingMethod::Trampoline: {
        // look for jmp's and call's in code
        // get virtual address in code of such instruction and calculate address
        // case 'call': add code that performs debug check + call to previous code using push : ret
        // case 'jmp' : add code that performs debug check + call to previous code

        // get call's and jmp's from text section

        QTemporaryFile temp_file;
        if(!temp_file.open())
            return false;

        QPair<QByteArray, Elf64_Addr> text_data;
        if (!elf->get_section_content(ELF::SectionType::TEXT, text_data))
            return false;

        temp_file.write(text_data.first);
        temp_file.flush();

        QProcess ndisasm;

        ndisasm.setProcessChannelMode(QProcess::MergedChannels);

        // TODO: change
        ndisasm.start("ndisasm", {"-a", "-b", elf->is_x64() ? "64" : "32", QFileInfo(temp_file).absoluteFilePath()});

        if(!ndisasm.waitForStarted())
            return false;

        QByteArray assembly;

        while(ndisasm.waitForReadyRead(-1))
            assembly.append(ndisasm.readAll());

        QStringList asm_inst = QString(assembly).split(CodeDefines<RegistersType>::newLineRegExp, QString::SkipEmptyParts);
        QStringList call_inst = asm_inst.filter(CodeDefines<RegistersType>::callRegExp);
        QStringList jmp_inst = asm_inst.filter(CodeDefines<RegistersType>::jmpRegExp);

        QList<Elf64_Addr> file_off;
        QList<Elf64_Addr> tramp_file_off;

        Elf64_Addr base_off;

        if (!elf->get_section_file_off(ELF::SectionType::TEXT, base_off))
            return false;

        get_file_offsets_from_opcodes(call_inst, file_off, base_off);
        get_file_offsets_from_opcodes(jmp_inst, file_off, base_off);

        QByteArray full_compiled_code;
        // TODO: choose randomy few addresses
        int32_t rva;
        Elf64_Addr inst_addr;

        // TODO: should be changed
        std::uniform_int_distribution<int> prob(0, 99);
        std::default_random_engine gen;
        uint8_t code_cover = 5;

        // FIXME: delete test purposes
        /*
        QList<Elf64_Addr> fo = { file_off.at(file_off.size() - 5), file_off.at(file_off.size() - 6) };
        file_off = fo;
        */
        foreach (Elf64_Addr off, file_off) {
            // FIXME: test purposes
            /*
            if(prob(gen) >= code_cover)
                continue;
            */
            if(prob(gen) >= code_cover)
                continue;

            if (!elf->get_relative_address(off, rva))
                return false;

            inst_addr = text_data.second + off - base_off;
            tramp_file_off.push_back(off);
            full_compiled_code.append(compiled_code);
            // calculate address
            if (elf->is_x86())
                // 5 - size of call instruction (minus 1 byte for call byte)
                full_compiled_code.append(CodeDefines<Registers_x86>::storeValue(static_cast<Elf32_Addr>(inst_addr + rva + 4)));
            else
                // 5 - size of call instruction (minus 1 byte for call byte)
                full_compiled_code.append(CodeDefines<Registers_x64>::storeValue(inst_addr + rva + 4));
            full_compiled_code.append(CodeDefines<RegistersType>::ret);
        }

        Elf64_Addr nva;
        if (!elf->extend_segment(full_compiled_code, i_desc->change_x_only, nva))
            return false;

        // TODO: check if divisible without extras
        Elf32_Addr tramp_size = full_compiled_code.size() / tramp_file_off.size();
        int i = 0;

        foreach (Elf64_Addr fo, tramp_file_off) {
            // 5 - size of call instruction (minus 1 byte for call byte)
            if (!elf->set_relative_address(fo, nva + (tramp_size * i) - (text_data.second + fo - base_off) - 4))
                return false;
            qDebug() << "jumping on : " << QString("0x%1 ").arg(text_data.second + fo - base_off - 1, 0, 16)
                     << "to: " << QString("0x%1 ").arg(nva + (tramp_size * i), 0, 16);
            ++i;
        }

        break;
    }
    default:
        return false;
    }

    qDebug() << "saving to file: " << i_desc->saved_fname;
    elf->write_to_file(i_desc->saved_fname);

    return true;
}
template bool ELFAddingMethods<Registers_x86>::secure(const QList<DAddingMethods<Registers_x86>::InjectDescription *> &inject_desc);
template bool ELFAddingMethods<Registers_x64>::secure(const QList<DAddingMethods<Registers_x64>::InjectDescription *> &inject_desc);
