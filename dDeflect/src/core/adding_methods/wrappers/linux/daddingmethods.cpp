#include <QProcess>
#include <QString>
#include <QDebug>

#include <core/adding_methods/wrappers/linux/daddingmethods.h>
#include <core/adding_methods/wrappers/linux/asmcodegenerator.h>

#define mnemonic_stringify(mnemonic) \
    QString((std::string(#mnemonic).substr(std::string(#mnemonic).find_last_of(':') != std::string::npos ? \
                                  std::string(#mnemonic).find_last_of(':') + 1 : 0)).c_str()).toLower()

DAddingMethods::DAddingMethods() {
    placeholder_id = {
        { PlaceholderTypes::PARAM_PRE,          QString("(?^_^")     },
        { PlaceholderTypes::PARAM_POST,         QString("^_^?)")     },
        { PlaceholderTypes::PLACEHOLDER_PRE,    QString("(rsj?^_^")  },
        { PlaceholderTypes::PLACEHOLDER_POST,   QString("^_^?rsj)")  }
    };

    placeholder_mnm = {
        { PlaceholderMnemonics::DDETECTIONHANDLER,  mnemonic_stringify(PlaceholderMnemonics::DDETECTIONHANDLER) },
        { PlaceholderMnemonics::DDETECTIONMETHOD,   mnemonic_stringify(PlaceholderMnemonics::DDETECTIONMETHOD)  }
    };
}

template <typename RegistersType>
bool DAddingMethods::secure_elf(ELF &elf, const InjectDescription<RegistersType> &inject_desc) {
    QString code2compile,
            code_ddetect_handler,
            code_ddetect;
    QByteArray compiled_code;

    if (!inject_desc.adding_method)
        return false;

    // 0. take code from input
    code2compile = inject_desc.adding_method->code;

    // 1. generate code for handler
    if (!wrapper_gen_code(inject_desc.adding_method->detect_handler, code_ddetect_handler))
        return false;

    // 2. generate code for debugger detection method
    switch (inject_desc.cm) {
    case CallingMethod::OEP: {
        OEPWrapper<RegistersType> *oepwrapper =
                dynamic_cast<OEPWrapper<RegistersType>*>(inject_desc.adding_method);
        if (!oepwrapper)
            return false;
        if (!wrapper_gen_code(oepwrapper->oep_action, code_ddetect))
            return false;
        break;
    }
    case CallingMethod::Thread: {
        ThreadWrapper<RegistersType> *twrapper =
                dynamic_cast<ThreadWrapper<RegistersType>*>(inject_desc.adding_method);
        if (!twrapper)
            return false;
        if (!wrapper_gen_code(twrapper->thread_action, code_ddetect))
            return false;
        break;
    }
    case CallingMethod::Trampoline: {

        break;
    }
    default:
        return false;
    }

    // 3. merge code
    fill_placeholders(code2compile, code_ddetect_handler, PlaceholderMnemonics::DDETECTIONHANDLER);
    fill_placeholders(code2compile, code_ddetect, PlaceholderMnemonics::DDETECTIONMETHOD);

    // 4. compile code
    if (!compile(code2compile, compiled_code))
        return false;

    // 5. secure elf file
    Elf64_Addr oldep;
    if (!elf.get_entry_point(oldep))
        return false;

    // 6. add jmp in binary
    // TODO: should be changed
    QByteArray jmp;
    if (elf.is_x86() || oldep < 0x100000000) {
        jmp.append(0xb8); // mov eax, addr
        // oldep -= 0x5;
        for(uint i = 0; i < sizeof(Elf32_Addr); ++i) {
            int a = (oldep >> (i * 8) & 0xff);
            jmp.append(static_cast<char>(a));
        }
        jmp.append("\xff\xe0", 2); // jmp eax
    }
    else if (elf.is_x64()) {
        jmp.append("\x48\xb8", 2); // mov rax, addr
        for(uint i = 0; i < sizeof(Elf64_Addr); ++i) {
            int a = (oldep >> (i * 8) & 0xff);
            jmp.append(static_cast<char>(a));
        }
        jmp.append("\xff\xe0", 2);
    }
    else return false;

    // 5. secure elf file
    Elf64_Addr nva;

    compiled_code.append(jmp);

    // TODO: parameter for x segment
    QByteArray nf = elf.extend_segment(compiled_code, true, nva);
    if (!nf.length())
        return false;

    if (!elf.set_entry_point(nva, nf))
        return false;

    qDebug() << "new entry point: " << QString("0x%1").arg(nva, 0, 16);

    // TODO: name of file should be changed
    elf.write_to_file("template", nf);

    return true;
}

template <typename RegistersType>
bool DAddingMethods::wrapper_gen_code(Wrapper<RegistersType> *wrap, QString &code) {
    if (!wrap)
        return false;

    // check if ret is in used registers (if it is remove)
    if (wrap->used_regs.indexOf(wrap->ret) != -1)
        wrap->used_regs.removeAll(wrap->ret);

    // generate push registers
    code.append(AsmCodeGenerator::push_regs<RegistersType>(wrap->used_regs));

    // fill params
    uint64_t filled_params = fill_params(code, wrap->params);

    // generate pop registers
    code.append(AsmCodeGenerator::pop_regs<RegistersType>(wrap->used_regs));

    return true;
}

uint64_t DAddingMethods::fill_params(QString &code, const QMap<QString, QString> &params) {
    uint64_t cnt = 0;
    foreach (QString param, params) {
        QString plc_param(QString("%1%2%3").arg(placeholder_id[PlaceholderTypes::PARAM_PRE], param,
                                                placeholder_id[PlaceholderTypes::PARAM_POST]));

        // look in string for plc_param cahnge it to value from dictionary
        code.replace(plc_param, params[param]);
    }
    return cnt;
}

uint64_t DAddingMethods::fill_placeholders(QString &code, const QString &gen_code, PlaceholderMnemonics plc_mnm) {
    uint64_t cnt = 0;

    code.replace(QString("%1%2%3").arg(placeholder_id[PlaceholderTypes::PLACEHOLDER_PRE], placeholder_mnm[plc_mnm],
                                       placeholder_id[PlaceholderTypes::PLACEHOLDER_POST]), gen_code);

    return cnt;
}

bool DAddingMethods::compile(const QString &code2compile, QByteArray &compiled_code) {
    // TODO: change
    QFile file("tocompile.asm");
    if (!file.open(QIODevice::WriteOnly))
        return false;

    if (file.write(code2compile.toStdString().c_str(), code2compile.length() != code2compile.length()))
        return false;

    file.close();

    if (QProcess::execute("nasm", { "-o compiled.bin", "tocompile.asm" }))
        return false;

    file.setFileName("compiled.bin");

    if (!file.open(QIODevice::ReadOnly))
        return false;

    compiled_code = file.readAll();

    file.close();
    return true;
}
