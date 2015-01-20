#include "elfaddingmethods.h"

#include <QProcess>
#include <QFileInfo>
#include <QTemporaryFile>
#include <QDebug>
#include <QMap>

#include <ApplicationManager/DJsonParser/djsonparser.h>
#include <ApplicationManager/dsettings.h>
#include <ApplicationManager/dlogger.h>

template <typename RegistersType>
const QMap<typename ELFAddingMethods<RegistersType>::ErrorCode, QString> ELFAddingMethods<RegistersType>::error_desc = {
    { ELFAddingMethods<RegistersType>::ErrorCode::BinaryFileNoElf,
      QString("Invalid Elf file structure!") },
    { ELFAddingMethods<RegistersType>::ErrorCode::CompiledFileOpenFailed,
      QString("Failed to open file with compiled code.") },
    { ELFAddingMethods<RegistersType>::ErrorCode::FileToCompileOpenFailed,
      QString("Failed to open file with assembly code.") },
    { ELFAddingMethods<RegistersType>::ErrorCode::FileToCompileWriteFailed,
      QString("Failed to write to file with assembly code.") },
    { ELFAddingMethods<RegistersType>::ErrorCode::GetEntryPointFailed,
      QString("Failed to get entry point from specified ELF file.") },
    { ELFAddingMethods<RegistersType>::ErrorCode::GetRelativeAddressFailed,
      QString("Failed to get relative address from specified ELF file.") },
    { ELFAddingMethods<RegistersType>::ErrorCode::GetSectionContentFailed,
      QString("Failed to get section content from specified ELF file.") },
    { ELFAddingMethods<RegistersType>::ErrorCode::GetSectionFileOffsetFailed,
      QString("Failed to get section file offset from specified ELF file.") },
    { ELFAddingMethods<RegistersType>::ErrorCode::GetSegmentAlignFailed,
      QString("Failed to get segment align from specified ELF file.") },
    { ELFAddingMethods<RegistersType>::ErrorCode::GetSegmentProtectFlagsFailed,
      QString("Failed to get segment protect flags from specified ELF file.") },
    { ELFAddingMethods<RegistersType>::ErrorCode::InvalidAddingMethodType,
      QString("Invalid code adding method type was specified.") },
    { ELFAddingMethods<RegistersType>::ErrorCode::InvalidAddressSizeAlign,
      QString("Invalid address size align for ELF file architecture.") },
    { ELFAddingMethods<RegistersType>::ErrorCode::InvalidElfFile,
      QString("Given file is not a valid ELF file!") },
    { ELFAddingMethods<RegistersType>::ErrorCode::NasmExecutionFailed,
      QString("Failed to start nasm compiler") },
    { ELFAddingMethods<RegistersType>::ErrorCode::NdisasmExecutionFailed,
      QString("Failed to start ndisasm decompiler") },
    { ELFAddingMethods<RegistersType>::ErrorCode::NullInjectDescription,
      QString("Loading inject description failed.") },
    { ELFAddingMethods<RegistersType>::ErrorCode::NullWrapper,
      QString("Loading wrapper failed.") },
    { ELFAddingMethods<RegistersType>::ErrorCode::SegmentExtensionFailed,
      QString("Failed to extend segment in specified ELF file.") },
    { ELFAddingMethods<RegistersType>::ErrorCode::SetEntryPointFailed,
      QString("Failed to set new entry point in specified ELF file.") },
    { ELFAddingMethods<RegistersType>::ErrorCode::SetRelativeAddressFailed,
      QString("Failed to set relative address in specified ELF file.") },
    { ELFAddingMethods<RegistersType>::ErrorCode::SetSectionContentFailed,
      QString("Failed to set section content in specified ELF file.") },
    { ELFAddingMethods<RegistersType>::ErrorCode::Success,
      QString("Success") },
    { ELFAddingMethods<RegistersType>::ErrorCode::TempFileOpenFailed,
      QString("Failed to open temporary file.") },
    { ELFAddingMethods<RegistersType>::ErrorCode::TemplateErrorWTF,
      QString("Invalid template type WTF??????.") },
    { ELFAddingMethods<RegistersType>::ErrorCode::TrampolineAddressAbsence,
      QString("Absence of addresses to inject trampoline code.") },
    { ELFAddingMethods<RegistersType>::ErrorCode::WrapperGenCodeFailed,
      QString("Failed to generate a specified wrapper code.") }
};

template <typename RegistersType>
ELFAddingMethods<RegistersType>::ELFAddingMethods(ELF *f) :
    DAddingMethods<RegistersType>(f),
    tramp_code_cover(5)
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
typename ELFAddingMethods<RegistersType>::ErrorCode
ELFAddingMethods<RegistersType>::fill_magic_params(QMap<QString, QString> &params, const ELF *elf) {
    static QString magic_sec_size("magic!sec_size"),
                   magic_sec_checksum("magic!sec_checksum");

    QPair<QByteArray, Elf64_Addr> section_data;
    if (!elf->get_section_content(ELF::SectionType::TEXT, section_data))
        return ErrorCode::GetSectionContentFailed;

    if (params.contains(magic_sec_size))
        params[magic_sec_size] = QString::number(section_data.first.size());
    if (params.contains(magic_sec_checksum)) {
        /*
        // TODO: calculate dummy checksum
        uint32_t checksum = 0;
        foreach (unsigned char b, section_data.first)
            checksum += b;
        */
        // fill checksum after
        params[magic_sec_checksum] = QString::number(0x1ee74b1d);
    }

    return ErrorCode::Success;
}

// bool ELFAddingMethods::fill_magic_checksum(const ELF64_Addr vaddr, ELF *elf, uint32_t &checksum) {
    /*
    static uint32_t code_cheksum_magic = 0x1ee74b1d;
    int j = 0;
    while ((j = compiled_code.indexOf(static_cast<const char *>(&code_cheksum_magic), j + 1)) != -1)
        if (!elf->set_relative_address(file_off + j, text_data.second - (nva + j - (elf->is_x86() ? 3 : 4))))
            return false;
    */
// }

template <typename RegistersType>
uint64_t ELFAddingMethods<RegistersType>::fill_placeholders(QString &code, const QString &gen_code, PlaceholderMnemonics plc_mnm) {
    uint64_t cnt = 0;

    code.replace(QString("%1%2%3").arg(placeholder_id[PlaceholderTypes::PLACEHOLDER_PRE], placeholder_mnm[plc_mnm],
                 placeholder_id[PlaceholderTypes::PLACEHOLDER_POST]), gen_code);

    return cnt;
}

template <typename RegistersType>
typename ELFAddingMethods<RegistersType>::ErrorCode
ELFAddingMethods<RegistersType>::compile(const QString &code2compile, QByteArray &compiled_code) {
    // TODO: change
    QFile file("tocompile.asm");
    if (!file.open(QIODevice::WriteOnly))
        return ErrorCode::FileToCompileOpenFailed;

    if (file.write(code2compile.toStdString().c_str(), code2compile.length()) != code2compile.length())
        return ErrorCode::FileToCompileWriteFailed;

    file.close();

    // TODO: change file names and etc.
    if (QProcess::execute(DSettings::getSettings().getNasmPath(), { "tocompile.asm" }))
        return ErrorCode::NasmExecutionFailed;

    file.setFileName("tocompile");

    if (!file.open(QIODevice::ReadOnly))
        return ErrorCode::CompiledFileOpenFailed;

    compiled_code = file.readAll();

    file.close();
    return ErrorCode::Success;
}

template <typename RegistersType>
typename ELFAddingMethods<RegistersType>::ErrorCode
ELFAddingMethods<RegistersType>::get_addresses(const QByteArray &addr_data, uint8_t addr_size, QList<Elf64_Addr> &addr_list,
                                               const QList<Elf64_Addr> &except_list) {
    int data_size = addr_data.size();
    if (data_size % addr_size)
        return ErrorCode::InvalidAddressSizeAlign;

    quint64 addr;

    for (int i = 0; i < data_size; i += addr_size) {
        addr = 0;
        for (int j = 0; j < addr_size; ++j)
            addr |= (static_cast<const char>((addr_data.data())[i + j]) & 0xff) << (j * 8);
        if (!except_list.contains(addr))
            addr_list.push_back(addr);
    }

    return ErrorCode::Success;
}

template <typename RegistersType>
typename ELFAddingMethods<RegistersType>::ErrorCode
ELFAddingMethods<RegistersType>::wrapper_gen_code(Wrapper<RegistersType> *wrap, QString &code) {
    if (!wrap)
        return ErrorCode::NullWrapper;

    // TODO: add check if contains all registers
    // add internal registers to one's we have to save
    foreach (RegistersType reg, CodeDefines<RegistersType>::internalRegs)
        if (!wrap->used_regs.contains(reg))
            wrap->used_regs.push_back(reg);

    // TODO: add check if method use all registers not to save ret one
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

    return ErrorCode::Success;
}
template typename ELFAddingMethods<Registers_x86>::ErrorCode ELFAddingMethods<Registers_x86>::wrapper_gen_code(Wrapper<Registers_x86> *wrap, QString &code);
template typename ELFAddingMethods<Registers_x64>::ErrorCode ELFAddingMethods<Registers_x64>::wrapper_gen_code(Wrapper<Registers_x64> *wrap, QString &code);

template <typename RegistersType>
void ELFAddingMethods<RegistersType>::get_file_offsets_from_opcodes(QStringList &opcodes, QList<Elf64_Addr> &file_off, Elf64_Addr base_off) {
    foreach(QString op, opcodes)
        file_off.append(op.mid(0, 8).toUInt(NULL, 16) + base_off + 1);
}

template <typename RegistersType>
typename ELFAddingMethods<RegistersType>::ErrorCode
ELFAddingMethods<RegistersType>::get_address_offsets_from_text_section(QList<Elf64_Addr> &__file_off, Elf64_Addr &base_off,
                                                                            QPair<QByteArray, Elf64_Addr> &text_data) {

    ELF *elf = dynamic_cast<ELF*>(DAddingMethods<RegistersType>::file);
    if(!elf)
        return ErrorCode::BinaryFileNoElf;

    if (!elf->is_valid())
        return ErrorCode::InvalidElfFile;

    QTemporaryFile temp_file;
    if(!temp_file.open())
        return ErrorCode::TempFileOpenFailed;


    if (!elf->get_section_content(ELF::SectionType::TEXT, text_data))
        return ErrorCode::GetSectionContentFailed;

    temp_file.write(text_data.first);
    temp_file.flush();

    QProcess ndisasm;

    ndisasm.setProcessChannelMode(QProcess::MergedChannels);

    ndisasm.start(DSettings::getSettings().getNdisasmPath(), {"-a", "-b", elf->is_x64() ? "64" : "32", QFileInfo(temp_file).absoluteFilePath()});

    if(!ndisasm.waitForStarted())
        return ErrorCode::NdisasmExecutionFailed;

    QByteArray assembly;

    while(ndisasm.waitForReadyRead(-1))
        assembly.append(ndisasm.readAll());

    QStringList asm_inst = QString(assembly).split(CodeDefines<RegistersType>::newLineRegExp, QString::SkipEmptyParts);
    QStringList call_inst = asm_inst.filter(CodeDefines<RegistersType>::callRegExp);
    QStringList jmp_inst = asm_inst.filter(CodeDefines<RegistersType>::jmpRegExp);

    if (!elf->get_section_file_off(ELF::SectionType::TEXT, base_off))
        return ErrorCode::GetSectionFileOffsetFailed;

    get_file_offsets_from_opcodes(call_inst, __file_off, base_off);
    get_file_offsets_from_opcodes(jmp_inst, __file_off, base_off);

    return ErrorCode::Success;
}

template <typename RegistersType>
bool ELFAddingMethods<RegistersType>::obfuscate(uint8_t code_cover, uint8_t min_len, uint8_t max_len) {
    ErrorCode ec = safe_obfuscate(code_cover, min_len, max_len);
    if (ec != ErrorCode::Success)
        LOG_ERROR(error_desc[ec]);
    return ec == ErrorCode::Success;
}
template bool ELFAddingMethods<Registers_x86>::obfuscate(uint8_t code_cover, uint8_t min_len, uint8_t max_len);
template bool ELFAddingMethods<Registers_x64>::obfuscate(uint8_t code_cover, uint8_t min_len, uint8_t max_len);

template <typename RegistersType>
typename ELFAddingMethods<RegistersType>::ErrorCode
ELFAddingMethods<RegistersType>::safe_obfuscate(uint8_t code_cover, uint8_t min_len, uint8_t max_len) {

    ErrorCode ec;
    ELF *elf = dynamic_cast<ELF*>(DAddingMethods<RegistersType>::file);
    if(!elf)
        return ErrorCode::BinaryFileNoElf;

    if (!elf->is_valid())
        return ErrorCode::InvalidElfFile;


    QList<Elf64_Addr> __file_off;
    Elf64_Addr base_off;
    QPair<QByteArray, Elf64_Addr> text_data;

    ec = get_address_offsets_from_text_section(__file_off, base_off, text_data);
    if (ec != ErrorCode::Success)
        return ec;

    QByteArray full_compiled_code;
    int32_t rva;
    Elf64_Addr inst_addr;

    QList<rel_jmp_info> tramp_file_off; // < <offset in added data, offset in file>,  virtual address>

    // TODO: should be changed
    std::uniform_int_distribution<int> prob(0, 99);
    uint8_t coverage = code_cover > 100 ? 100 : code_cover;
    static QByteArray fake_jmp("\xe9\xde\xad\xbe\xef", 5);

    QByteArray trash_code;
    foreach (Elf64_Addr off, __file_off) {
        if(prob(DAddingMethods<RegistersType>::r_gen) >= coverage)
            continue;

        if (!elf->get_relative_address(off, rva))
            return ErrorCode::GetRelativeAddressFailed;

        inst_addr = text_data.second + off - base_off;
        trash_code = CodeDefines<RegistersType>::obfuscate(DAddingMethods<RegistersType>::r_gen, min_len, max_len);

        tramp_file_off.push_back(rel_jmp_info(full_compiled_code.size(), trash_code.size() + fake_jmp.size(),
                                              off, inst_addr + rva + 4));

        full_compiled_code.append(trash_code);
        full_compiled_code.append(fake_jmp);
    }

    Elf64_Addr nva;
    Elf64_Off file_off;

    // TODO: change only_x value
    if (!elf->extend_segment(full_compiled_code, false, nva, file_off))
        return ErrorCode::SegmentExtensionFailed;

    foreach (auto fo_addr, tramp_file_off) {
        // 5 - size of call instruction (minus 1 byte for call byte)
        if (!elf->set_relative_address(fo_addr.fdata_off,
                                       nva + fo_addr.ndata_off - (text_data.second + fo_addr.fdata_off - base_off) - 4))
            return ErrorCode::SetRelativeAddressFailed;
        qDebug() << "jumping on : " << QString("0x%1").arg(text_data.second + fo_addr.fdata_off - base_off - 1, 0, 16)
                 << "to: " << QString("0x%1").arg(nva + fo_addr.ndata_off, 0, 16);

        // set new relative address for jmp
        if (!elf->set_relative_address(file_off + (fo_addr.ndata_off + fo_addr.ndata_size) - 4,
                                       fo_addr.data_vaddr - (nva + ((fo_addr.ndata_off + fo_addr.ndata_size) - fake_jmp.size())) - 5))
            return ErrorCode::SetRelativeAddressFailed;
    }

    return ErrorCode::Success;
}

// ===============================================================================
// TODO: add detect method return value to wrapper used_regs, and push it on stack
// ===============================================================================
template <typename RegistersType>
bool ELFAddingMethods<RegistersType>::secure(const QList<typename DAddingMethods<RegistersType>::InjectDescription*> &inject_desc) {
    ErrorCode ec;
    foreach(typename DAddingMethods<RegistersType>::InjectDescription* id, inject_desc) {
        ec = secure_one(id);
        if(ec != ErrorCode::Success) {
            LOG_ERROR(error_desc[ec]);
            return false;
        }
    }
    return true;
}

template <typename RegistersType>
typename ELFAddingMethods<RegistersType>::ErrorCode
ELFAddingMethods<RegistersType>::secure_one(typename DAddingMethods<RegistersType>::InjectDescription* i_desc) {
    QString code2compile,
            code_ddetect_handler,
            code_ddetect;
    QByteArray compiled_code;
    ErrorCode ec;

    ELF *elf = dynamic_cast<ELF*>(DAddingMethods<RegistersType>::file);
    if(!elf)
        return ErrorCode::BinaryFileNoElf;

    if (!elf->is_valid())
        return ErrorCode::InvalidElfFile;

    // check platform version
    if (std::is_same<RegistersType, Registers_x86>::value)
        code2compile.append(QString("%1\n").arg(
                                DAddingMethods<RegistersType>::arch_type[DAddingMethods<RegistersType>::ArchitectureType::BITS32]));
    else if(std::is_same<RegistersType, Registers_x64>::value)
        code2compile.append(QString("%1\n").arg(
                                DAddingMethods<RegistersType>::arch_type[DAddingMethods<RegistersType>::ArchitectureType::BITS64]));
    else return ErrorCode::TemplateErrorWTF;

    if (!i_desc)
        return ErrorCode::NullInjectDescription;

    // add to params
    if (!i_desc->adding_method || !i_desc->adding_method->detect_handler)
        return ErrorCode::NullWrapper;

    // adding a param value for (?^_^ddret^_^?)
    i_desc->adding_method->static_params[placeholder_mnm[PlaceholderMnemonics::DDRET]] = elf->is_x86() ?
                AsmCodeGenerator::get_reg<Registers_x86>(static_cast<Registers_x86>(i_desc->adding_method->ret)) :
                AsmCodeGenerator::get_reg<Registers_x64>(static_cast<Registers_x64>(i_desc->adding_method->ret));

    // make a wrapper save register that is used for debugger detection function to return value
    if (!i_desc->adding_method->used_regs.contains(i_desc->adding_method->ret))
        i_desc->adding_method->used_regs.push_back(i_desc->adding_method->ret);

    i_desc->adding_method->ret = RegistersType::None;

    // 0. take code from input
    ec = wrapper_gen_code(i_desc->adding_method, code2compile);
    if (ec != ErrorCode::Success)
        return ec;

    // 1. generate code for handler
    ec = wrapper_gen_code(i_desc->adding_method->detect_handler, code_ddetect_handler);
    if (ec != ErrorCode::Success)
        return ec;

    bool dyn_magic = false;
    static QString dynmagic_offset("dyn_magic!offset");

    // 2. generate code for debugger detection method
    switch (i_desc->cm) {
    case DAddingMethods<RegistersType>::CallingMethod::OEP: {
         OEPWrapper<RegistersType> *oepwrapper =
                dynamic_cast<OEPWrapper<RegistersType>*>(i_desc->adding_method);
        if (!oepwrapper)
            return ErrorCode::NullWrapper;
        ec = fill_magic_params(oepwrapper->oep_action->static_params, elf);
        if (ec != ErrorCode::Success)
            return ec;
        ec = wrapper_gen_code(oepwrapper->oep_action, code_ddetect);
        if (ec != ErrorCode::Success)
            return ec;
        if (oepwrapper->oep_action->static_params.contains(dynmagic_offset))
            dyn_magic = true;
        break;
    }
    case DAddingMethods<RegistersType>::CallingMethod::Thread: {
        ThreadWrapper<RegistersType> *twrapper =
                dynamic_cast<ThreadWrapper<RegistersType>*>(i_desc->adding_method);
        if (!twrapper)
            return ErrorCode::NullWrapper;
        ec = fill_magic_params(twrapper->thread_actions[0]->static_params, elf);
        if (ec != ErrorCode::Success)
            return ec;
        ec = wrapper_gen_code(twrapper->thread_actions[0], code_ddetect);
        if (ec != ErrorCode::Success)
            return ec;
        if (twrapper->thread_actions[0]->static_params.contains(dynmagic_offset))
            dyn_magic = true;
        break;
    }
    case DAddingMethods<RegistersType>::CallingMethod::Trampoline:
    case DAddingMethods<RegistersType>::CallingMethod::INIT_ARRAY:
    case DAddingMethods<RegistersType>::CallingMethod::CTORS :
    case DAddingMethods<RegistersType>::CallingMethod::INIT: {
        TrampolineWrapper<RegistersType> *trmwrapper =
                dynamic_cast<TrampolineWrapper<RegistersType>*>(i_desc->adding_method);
        if (!trmwrapper)
            return ErrorCode::NullWrapper;
        ec = fill_magic_params(trmwrapper->tramp_action->static_params, elf);
        if (ec != ErrorCode::Success)
            return ec;\
        ec = wrapper_gen_code(trmwrapper->tramp_action, code_ddetect);
        if (ec != ErrorCode::Success)
            return ec;
        if (trmwrapper->tramp_action->static_params.contains(dynmagic_offset))
            dyn_magic = true;
        break;
    }

    default:
        return ErrorCode::InvalidAddingMethodType;
    }

    // 3. merge code
    fill_placeholders(code2compile, code_ddetect_handler, PlaceholderMnemonics::DDETECTIONHANDLER);
    fill_placeholders(code2compile, code_ddetect, PlaceholderMnemonics::DDETECTIONMETHOD);

    Elf64_Addr oldep;
    if (!elf->get_entry_point(oldep))
        return ErrorCode::GetEntryPointFailed;

    // 4. compile code
    ec = compile(code2compile, compiled_code);
    if (ec != ErrorCode::Success)
        return ec;

    // 5. secure elf file

    Elf64_Addr nva;
    Elf64_Off file_off;
    // TODO: change
    static QByteArray fake_jmp("\xe9\xde\xad\xbe\xef", 5);

    switch(i_desc->cm) {
    case DAddingMethods<RegistersType>::CallingMethod::Thread:
    case DAddingMethods<RegistersType>::CallingMethod::OEP: {
        Elf64_Addr oep;
        // add fake relative jump to the code and repair it after
        compiled_code.append(fake_jmp);

        if (!elf->extend_segment(compiled_code, i_desc->change_x_only, nva, file_off))
            return ErrorCode::SegmentExtensionFailed;

        if (!elf->set_entry_point(nva, &oep))
            return ErrorCode::SetEntryPointFailed;

        // set new relative address for jmp
        if (!elf->set_relative_address(file_off + compiled_code.size() - 4, oep - (nva + (compiled_code.size() - fake_jmp.size())) - 5))
            return ErrorCode::SetRelativeAddressFailed;

        if (dyn_magic) {
            QPair<QByteArray, Elf64_Addr> text_data;
            if (!elf->get_section_content(ELF::SectionType::TEXT, text_data))
                return ErrorCode::GetSectionContentFailed;

            int j = 0;
            while ((j = compiled_code.indexOf("\xba\xda\xda\xba", j + 1)) != -1)
                if (!elf->set_relative_address(file_off + j, text_data.second - (nva + j - (elf->is_x86() ? 3 : 4))))
                    return ErrorCode::SetRelativeAddressFailed;

            static uint32_t code_cheksum_magic = 0x1ee74b1d;
            j = 0;
            uint32_t checksum = 0;
            foreach (unsigned char b, text_data.first)
                checksum += b;
            while ((j = compiled_code.indexOf("\x1d\x4b\xe7\x1e", j + 1)) != -1)
                if (!elf->set_relative_address(file_off + j, checksum))
                    return ErrorCode::SetRelativeAddressFailed;
        }

        qDebug() << "new entry point: " << QString("0x%1").arg(nva, 0, 16);
        break;
    }
    case DAddingMethods<RegistersType>::CallingMethod::CTORS: {
        QPair<QByteArray, Elf64_Addr> section_data;
        if (!elf->get_section_content(ELF::SectionType::CTORS, section_data))
            return ErrorCode::GetSectionContentFailed;

        QList<Elf64_Addr> addresses;
        uint8_t addr_size = elf->is_x86() ? sizeof(Elf32_Addr) : sizeof(Elf64_Addr);
        ec = get_addresses(section_data.first, addr_size, addresses, { 0 });
        if (ec != ErrorCode::Success)
            return ec;

        // no place to store our pointer
        if (!addresses.size())
            return ErrorCode::TrampolineAddressAbsence;

        // TODO: address should be randomized, not always 0
        int idx = 0;

        compiled_code.append(fake_jmp);

        if (!elf->extend_segment(compiled_code, i_desc->change_x_only, nva, file_off))
            return ErrorCode::SegmentExtensionFailed;

        // set new relative address for jmp
        if (!elf->set_relative_address(file_off + compiled_code.size() - 4,
                                       addresses[idx] - (nva + (compiled_code.size() - fake_jmp.size())) - 5))
            return ErrorCode::SetRelativeAddressFailed;

        // set section content, set filler for elf function
        section_data.first.replace(idx * addr_size, addr_size, QByteArray(reinterpret_cast<const char *>(&nva), addr_size));

        if (!elf->set_section_content(ELF::SectionType::CTORS, section_data.first))
            return ErrorCode::SetSectionContentFailed;

        if (dyn_magic) {
            QPair<QByteArray, Elf64_Addr> text_data;
            if (!elf->get_section_content(ELF::SectionType::TEXT, text_data))
                return ErrorCode::GetSectionContentFailed;

            int j = 0;
            while ((j = compiled_code.indexOf("\xba\xda\xda\xba", j + 1)) != -1)
                if (!elf->set_relative_address(file_off + j, text_data.second - (nva + j - (elf->is_x86() ? 3 : 4))))
                    return ErrorCode::SetRelativeAddressFailed;

            static uint32_t code_cheksum_magic = 0x1ee74b1d;
            j = 0;
            uint32_t checksum = 0;
            foreach (unsigned char b, text_data.first)
                checksum += b;
            while ((j = compiled_code.indexOf("\x1d\x4b\xe7\x1e", j + 1)) != -1)
                if (!elf->set_relative_address(file_off + j, checksum))
                    return ErrorCode::SetRelativeAddressFailed;
        }

        qDebug() << "data added at: " << QString("0x%1").arg(nva, 0, 16);

        break;
    }
    case DAddingMethods<RegistersType>::CallingMethod::INIT: {
        QPair<QByteArray, Elf64_Addr> section_data;
        if (!elf->get_section_content(ELF::SectionType::INIT, section_data))
            return ErrorCode::GetSectionContentFailed;

        unsigned int prot_flags;
        Elf64_Addr align;

        // get relevant info
        if (!elf->get_segment_prot_flags(section_data.second, prot_flags))
            return ErrorCode::GetSegmentProtectFlagsFailed;
        if (!elf->get_segment_align(section_data.second, align))
            return ErrorCode::GetSegmentAlignFailed;

        // whole code will look like:
        // detection code
        // jmp to copy to init code
        // init code
        // change memory protection +w
        // copy to init code
        // restore old memory protection
        // jmp to init code

        // compiled_code = debugger detection + jmp to copy routine
        // TODO: save flags here, shitiest solution ever
        compiled_code.append('\x9c'); // pushf | pushfq
        compiled_code.append(CodeDefines<RegistersType>::saveAll());

        // TODO: should be redone
        // store init data address on stack
        compiled_code.append(CodeDefines<Registers_x86>::callRelative(section_data.first.size()));

        // compiled_code = debugger detection + jmp to copy routine + previous init
        compiled_code.append(section_data.first);

        compiled_code.append(CodeDefines<Registers_x86>::restoreRegister(Registers_x86::ESI));
        // TODO: change
        compiled_code.append("\xe8\x00\x00\x00\x00", 5); // call $+5
        compiled_code.append(CodeDefines<Registers_x86>::restoreRegister(Registers_x86::EDI)); // pop (e|r)di

        Elf64_Off offset_to_get_init_section_code_addr_copy = compiled_code.size();

        static QByteArray fake_add_edi("\x81\xc7\x00\x00\x00\x00", 6);
        static QByteArray fake_add_rdi("\x48\x81\xc7\x00\x00\x00\x00", 7);

        compiled_code.append(elf->is_x86() ? fake_add_edi : fake_add_rdi); // sub (e|r)di, 0

        compiled_code.append(CodeDefines<RegistersType>::saveAll());

        // ========
        // mprotect
        // ========

        int init_size = section_data.first.size();
        unsigned int prot_flags_w = prot_flags | PF_W;

        compiled_code.append(QByteArray(1, '\xba') + QByteArray(reinterpret_cast<const char*>(&prot_flags_w),
                                                                sizeof(unsigned int))); // mov (e|r)dx, flags
        compiled_code.append("\x31\xc0"); // xor eax, eax

        if (elf->is_x86())
            compiled_code.append(QByteArray(1, '\xb8') + QByteArray(reinterpret_cast<const char*>(&align),
                                                                    sizeof(uint32_t))); // mov eax, page_align
        else
            compiled_code.append(QByteArray("\x48\xb8", 2) + QByteArray(reinterpret_cast<const char*>(&align),
                                                                        sizeof(uint64_t))); // mov rax, page_align

        compiled_code.append(elf->is_x86() ? QByteArray("\xff\xc8", 2) : QByteArray("\x48\xff\xc8", 3)); // dec (e|r)ax
        compiled_code.append(elf->is_x86() ? QByteArray("\xF7\xD0", 2) : QByteArray("\x48\xF7\xD0", 3)); // not (e|r)ax

        compiled_code.append("\xe8\x00\x00\x00\x00", 5); // call $+5
        compiled_code.append(CodeDefines<Registers_x86>::restoreRegister(Registers_x86::EDI)); // pop (e|r)di <--- address of current instruction

        Elf64_Off offset_to_get_init_section_code_addr_mprotect = compiled_code.size();

        compiled_code.append(elf->is_x86() ? fake_add_edi : fake_add_rdi); // sub (e|r)di, 0

        // round page
        compiled_code.append(elf->is_x86() ? QByteArray("\x89\xFB", 2) :
                                             QByteArray("\x48\x89\xFB", 3)); // mov (e|r)bx, (e|r)di

        compiled_code.append(elf->is_x86() ? QByteArray("\x21\xc7", 2) :
                                             QByteArray("\x48\x21\xc7", 3)); // and (e|r)di, (r|e)ax

        compiled_code.append(elf->is_x86() ? QByteArray("\x29\xFB", 2) :
                                             QByteArray("\x48\x29\xFB", 3)); // sub bx, di <----- bx size of page

        compiled_code.append(QByteArray("\x81\xc3", 2) +
                             QByteArray(reinterpret_cast<const char*>(&init_size), sizeof(int))); // mov ebx, init_size

        // dx <--- flags
        // bx <--- memory size
        // di <--- page_vaddr

        compiled_code.append(elf->is_x86() ? QByteArray("\xB8\x7D\x00\x00\x00", 5) :
                                             QByteArray("\xB8\x0A\x00\x00\x00", 5)); // mov eax, syscall_num

        if (elf->is_x86()) {
            compiled_code.append(QByteArray("\x89\xD9", 2)); // mov ecx, ebx
            compiled_code.append(QByteArray("\x89\xFB", 2)); // mov ebx, edi
        }
        else {
            compiled_code.append(QByteArray("\x48\x89\xDE", 3)); // mov rsi, rdx
        }

        compiled_code.append(elf->is_x86() ? QByteArray("\xCD\x80") : QByteArray("\x0F\x05", 2)); // syscall

        compiled_code.append(CodeDefines<RegistersType>::restoreAll());


        // save all registers now before using an mprotect

        // mov ecx, init_section_len , don't use rcx cause
        // TODO: probably should xor rax, rax
        compiled_code.append("\x31\xc9"); // xor eax, eax
        compiled_code.append(QByteArray(1, '\xb9') +
                             QByteArray(reinterpret_cast<const char*>(&init_size), sizeof(int)));

        // make a copy to init section
        // rep movsb
        compiled_code.append("\xf3\xa4", 2);

        // ===========================================
        // TODO: call mprotect here for READ ^ EXECUTE
        // TODO: restore flags here
        // ===========================================

        // restore regs
        // TODO: save flags here, shitiest solution ever
        compiled_code.append(CodeDefines<RegistersType>::restoreAll());
        compiled_code.append('\x9d'); // popf | popfq

        // jmp to init
        static QByteArray fake_jmp("\xe9\xde\xad\xbe\xef", 5);
        compiled_code.append(fake_jmp);

        if (!elf->extend_segment(compiled_code, i_desc->change_x_only, nva, file_off))
            return ErrorCode::SegmentExtensionFailed;

        // set up dummy values

        // 4 jmp to init
        if (!elf->set_relative_address(file_off + compiled_code.size() - 4,
                                       section_data.second - (nva + compiled_code.size() - fake_jmp.size()) - 5))
            return ErrorCode::SetRelativeAddressFailed;

        // 4 string copy destinationaddress
        if (!elf->set_relative_address(file_off + offset_to_get_init_section_code_addr_copy + (elf->is_x86() ? 2 : 3),
                                       section_data.second - (nva + offset_to_get_init_section_code_addr_copy - 1)))
            return ErrorCode::SetRelativeAddressFailed;

        // 4 mprotect page vaddr
        if (!elf->set_relative_address(file_off + offset_to_get_init_section_code_addr_mprotect + (elf->is_x86() ? 2 : 3),
                                       section_data.second - (nva + offset_to_get_init_section_code_addr_mprotect - 1)))
            return ErrorCode::SetRelativeAddressFailed;

        // set redirection from init sectin to our code
        Elf32_Addr redirect_off = nva - section_data.second - 5;
        QByteArray compiled_jmp((QByteArray(1, '\xe9') + QByteArray(reinterpret_cast<const char*>(&redirect_off), sizeof(Elf32_Addr))));


        if (!elf->set_section_content(ELF::SectionType::INIT, compiled_jmp, '\x90'))
            return ErrorCode::SetSectionContentFailed;

        if (dyn_magic) {
            QPair<QByteArray, Elf64_Addr> text_data;
            if (!elf->get_section_content(ELF::SectionType::TEXT, text_data))
                return ErrorCode::GetSectionContentFailed;

            int j = 0;
            while ((j = compiled_code.indexOf("\xba\xda\xda\xba", j + 1)) != -1)
                if (!elf->set_relative_address(file_off + j, text_data.second - (nva + j - (elf->is_x86() ? 3 : 4))))
                    return ErrorCode::SetRelativeAddressFailed;

            static uint32_t code_cheksum_magic = 0x1ee74b1d;
            j = 0;
            uint32_t checksum = 0;
            foreach (unsigned char b, text_data.first)
                checksum += b;
            while ((j = compiled_code.indexOf("\x1d\x4b\xe7\x1e", j + 1)) != -1)
                if (!elf->set_relative_address(file_off + j, checksum))
                    return ErrorCode::SetRelativeAddressFailed;
        }


        qDebug() << "data added at: " << QString("0x%1").arg(nva, 0, 16);

        break;
    }
    case DAddingMethods<RegistersType>::CallingMethod::INIT_ARRAY: {
        QPair<QByteArray, Elf64_Addr> section_data;
        if (!elf->get_section_content(ELF::SectionType::INIT_ARRAY, section_data))
            return ErrorCode::GetSectionContentFailed;

        QList<Elf64_Addr> addresses;
        uint8_t addr_size = elf->is_x86() ? sizeof(Elf32_Addr) : sizeof(Elf64_Addr);
        ec = get_addresses(section_data.first, addr_size, addresses, { 0 });
        if (ec != ErrorCode::Success)
            return ec;

        // no place to store our pointer
        if (!addresses.size())
            return ErrorCode::TrampolineAddressAbsence;

        // TODO: address should be randomized, not always 0
        int idx = 0;

        compiled_code.append(fake_jmp);

        if (!elf->extend_segment(compiled_code, i_desc->change_x_only, nva, file_off))
            return ErrorCode::SegmentExtensionFailed;

        // set new relative address for jmp
        if (!elf->set_relative_address(file_off + compiled_code.size() - 4,
                                       addresses[idx] - (nva + (compiled_code.size() - fake_jmp.size())) - 5))
            return ErrorCode::SetRelativeAddressFailed;

        // set section content, set filler for elf function
        section_data.first.replace(idx * addr_size, addr_size, QByteArray(reinterpret_cast<const char *>(&nva), addr_size));

        if (!elf->set_section_content(ELF::SectionType::INIT_ARRAY, section_data.first))
            return ErrorCode::SetSectionContentFailed;

        if (dyn_magic) {
            QPair<QByteArray, Elf64_Addr> text_data;
            if (!elf->get_section_content(ELF::SectionType::TEXT, text_data))
                return ErrorCode::GetSectionContentFailed;

            int j = 0;
            while ((j = compiled_code.indexOf("\xba\xda\xda\xba", j + 1)) != -1)
                if (!elf->set_relative_address(file_off + j, text_data.second - (nva + j - (elf->is_x86() ? 3 : 4))))
                    return ErrorCode::SetRelativeAddressFailed;

            static uint32_t code_cheksum_magic = 0x1ee74b1d;
            j = 0;
            uint32_t checksum = 0;
            foreach (unsigned char b, text_data.first)
                checksum += b;
            while ((j = compiled_code.indexOf("\x1d\x4b\xe7\x1e", j + 1)) != -1)
                if (!elf->set_relative_address(file_off + j, checksum))
                    return ErrorCode::SetRelativeAddressFailed;
        }

        qDebug() << "data added at: " << QString("0x%1").arg(nva, 0, 16);

        break;
    }
    case DAddingMethods<RegistersType>::CallingMethod::Trampoline: {
        // look for jmp's and call's in code
        // get virtual address in code of such instruction and calculate address
        // case 'call': add code that performs debug check + call to previous code using push : ret
        // case 'jmp' : add code that performs debug check + call to previous code

        QList<Elf64_Addr> __file_off;
        Elf64_Addr base_off;
        QPair<QByteArray, Elf64_Addr> text_data;
        QList<QPair<Elf64_Addr, Elf64_Addr> > tramp_file_off;

        ec = get_address_offsets_from_text_section(__file_off, base_off, text_data);
        if (ec != ErrorCode::Success)
            return ec;

        QByteArray full_compiled_code;
        // TODO: choose randomy few addresses
        int32_t rva;
        Elf64_Addr inst_addr;

        // TODO: should be changed
        std::uniform_int_distribution<int> prob(0, 99);

        foreach (Elf64_Addr off, __file_off) {
            if(prob(DAddingMethods<RegistersType>::r_gen) >= tramp_code_cover)
                continue;

            if (!elf->get_relative_address(off, rva))
                return ErrorCode::GetRelativeAddressFailed;

            inst_addr = text_data.second + off - base_off;
            tramp_file_off.push_back(QPair<Elf64_Addr, Elf64_Addr>(off, inst_addr + rva + 4));
            full_compiled_code.append(compiled_code);
            full_compiled_code.append(fake_jmp);
        }

        Elf64_Addr nva;
        if (!elf->extend_segment(full_compiled_code, i_desc->change_x_only, nva, file_off))
            return ErrorCode::SegmentExtensionFailed;

        if (!tramp_file_off.size())
            return ErrorCode::TrampolineAddressAbsence;

        Elf32_Addr tramp_size = full_compiled_code.size() / tramp_file_off.size();
        int i = 0;

        foreach (auto fo_addr, tramp_file_off) {
            // 5 - size of call instruction (minus 1 byte for call byte)
            if (!elf->set_relative_address(fo_addr.first, nva + (tramp_size * i) - (text_data.second + fo_addr.first - base_off) - 4))
                return ErrorCode::SetRelativeAddressFailed;
            qDebug() << "jumping on : " << QString("0x%1 ").arg(text_data.second + fo_addr.first - base_off - 1, 0, 16)
                     << "to: " << QString("0x%1 ").arg(nva + (tramp_size * i), 0, 16);

            // set new relative address for jmp
            if (!elf->set_relative_address(file_off + (tramp_size * (i + 1)) - 4,
                                           fo_addr.second - (nva + ((tramp_size * (i + 1)) - fake_jmp.size())) - 5))
                return ErrorCode::SetRelativeAddressFailed;

            ++i;
        }

        if (dyn_magic) {
            QPair<QByteArray, Elf64_Addr> text_data;
            if (!elf->get_section_content(ELF::SectionType::TEXT, text_data))
                return ErrorCode::GetSectionContentFailed;

            int j = 0;
            while ((j = full_compiled_code.indexOf("\xba\xda\xda\xba", j + 1)) != -1)
                if (!elf->set_relative_address(file_off + j, text_data.second - (nva + j - (elf->is_x86() ? 3 : 4))))
                    return ErrorCode::SetRelativeAddressFailed;

            // static uint32_t code_cheksum_magic = 0x1ee74b1d;
            j = 0;
            uint32_t checksum = 0;
            foreach (unsigned char b, text_data.first)
                checksum += b;
            while ((j = full_compiled_code.indexOf("\x1d\x4b\xe7\x1e", j + 1)) != -1)
                if (!elf->set_relative_address(file_off + j, checksum))
                    return ErrorCode::SetRelativeAddressFailed;
        }

        break;
    }
    default:
        return ErrorCode::InvalidAddingMethodType;
    }
    return ErrorCode::Success;
}
template bool ELFAddingMethods<Registers_x86>::secure(const QList<DAddingMethods<Registers_x86>::InjectDescription *> &inject_desc);
template bool ELFAddingMethods<Registers_x64>::secure(const QList<DAddingMethods<Registers_x64>::InjectDescription *> &inject_desc);
