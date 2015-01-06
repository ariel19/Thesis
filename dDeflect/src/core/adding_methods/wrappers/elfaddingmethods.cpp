#include "elfaddingmethods.h"

ELFAddingMethods::ELFAddingMethods(ELF *f) :
    DAddingMethods(f)
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

ELFAddingMethods::~ELFAddingMethods() {}

uint64_t ELFAddingMethods::fill_params(QString &code, const QMap<QString, QString> &params) {
    uint64_t cnt = 0;
    foreach (QString param, params.keys()) {
        QString plc_param(QString("%1%2%3").arg(placeholder_id[PlaceholderTypes::PARAM_PRE], param,
                                                placeholder_id[PlaceholderTypes::PARAM_POST]));

        // look in string for plc_param cahnge it to value from dictionary
        code.replace(plc_param, params[param]);
    }
    return cnt;
}

uint64_t ELFAddingMethods::fill_placeholders(QString &code, const QString &gen_code, PlaceholderMnemonics plc_mnm) {
    uint64_t cnt = 0;

    code.replace(QString("%1%2%3").arg(placeholder_id[PlaceholderTypes::PLACEHOLDER_PRE], placeholder_mnm[plc_mnm],
                                       placeholder_id[PlaceholderTypes::PLACEHOLDER_POST]), gen_code);

    return cnt;
}

bool ELFAddingMethods::compile(const QString &code2compile, QByteArray &compiled_code) {
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

bool ELFAddingMethods::get_addresses(const QByteArray &addr_data, uint8_t addr_size, QList<Elf64_Addr> &addr_list,
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

bool ELFAddingMethods::set_prot_flags_gen_code_x86(Elf32_Addr vaddr, Elf32_Word mem_size,
                                                 Elf32_Word flags, QString &code) {
    // TODO: generated using JSON parser, here is stupid dummy solution
    Wrapper<Registers_x86> mprotect;

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

    return wrapper_gen_code<Registers_x86>(&mprotect, code);
}

bool ELFAddingMethods::set_prot_flags_gen_code_x64(Elf64_Addr vaddr, Elf64_Xword mem_size,
                                                 Elf64_Word flags, QString &code) {
    // TODO: generated using JSON parser, here is stupid dummy solution
    Wrapper<Registers_x64> mprotect;

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

    return wrapper_gen_code<Registers_x64>(&mprotect, code);
}