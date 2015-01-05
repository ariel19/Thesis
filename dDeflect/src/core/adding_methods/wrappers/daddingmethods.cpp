#include <QProcess>
#include <QString>
#include <QDebug>
#include <QDataStream>

#include <core/adding_methods/wrappers/daddingmethods.h>

#define mnemonic_stringify(mnemonic) \
    QString((std::string(#mnemonic).substr(std::string(#mnemonic).find_last_of(':') != std::string::npos ? \
                                  std::string(#mnemonic).find_last_of(':') + 1 : 0)).c_str()).toLower()

#define enum_stringify(enum_val) \
    QString((std::string(#enum_val).substr(std::string(#enum_val).find_last_of(':') != std::string::npos ? \
                                  std::string(#enum_val).find_last_of(':') + 1 : 0)).c_str()).toLower()

#define instruction_stringify(instruction) \
    QString((std::string(#instruction).substr(std::string(#instruction).find_last_of(':') != std::string::npos ? \
                                  std::string(#instruction).find_last_of(':') + 1 : 0)).c_str()).toLower()

const QMap<Registers_x86, QString> AsmCodeGenerator::regs_x86 = {
    { Registers_x86::EAX, enum_stringify(Registers_x86::EAX) },
    { Registers_x86::EBX, enum_stringify(Registers_x86::EBX) },
    { Registers_x86::ECX, enum_stringify(Registers_x86::ECX) },
    { Registers_x86::EDX, enum_stringify(Registers_x86::EDX) },
    { Registers_x86::EDI, enum_stringify(Registers_x86::EDI) },
    { Registers_x86::EBP, enum_stringify(Registers_x86::EBP) },
    { Registers_x86::ESI, enum_stringify(Registers_x86::ESI) },
    { Registers_x86::ESP, enum_stringify(Registers_x86::ESP) }
};

const QMap<Registers_x64, QString> AsmCodeGenerator::regs_x64 = {
    { Registers_x64::RAX, enum_stringify(Registers_x64::RAX) },
    { Registers_x64::RBX, enum_stringify(Registers_x64::RBX) },
    { Registers_x64::RCX, enum_stringify(Registers_x64::RCX) },
    { Registers_x64::RDX, enum_stringify(Registers_x64::RDX) },
    { Registers_x64::RDI, enum_stringify(Registers_x64::RDI) },
    { Registers_x64::RSI, enum_stringify(Registers_x64::RSI) },
    { Registers_x64::RBP, enum_stringify(Registers_x64::RBP) },
    { Registers_x64::RSP, enum_stringify(Registers_x64::RSP) },
    { Registers_x64::R8 , enum_stringify(Registers_x64::R8)  },
    { Registers_x64::R9 , enum_stringify(Registers_x64::R9)  },
    { Registers_x64::R10, enum_stringify(Registers_x64::R10) },
    { Registers_x64::R11, enum_stringify(Registers_x64::R11) },
    { Registers_x64::R12, enum_stringify(Registers_x64::R12) },
    { Registers_x64::R13, enum_stringify(Registers_x64::R13) },
    { Registers_x64::R14, enum_stringify(Registers_x64::R14) },
    { Registers_x64::R15, enum_stringify(Registers_x64::R15) }
};

const QMap<AsmCodeGenerator::Instructions, QString> AsmCodeGenerator::instructions = {
    { AsmCodeGenerator::Instructions::POP , instruction_stringify(AsmCodeGenerator::Instructions::POP)  },
    { AsmCodeGenerator::Instructions::PUSH, instruction_stringify(AsmCodeGenerator::Instructions::PUSH) },
    { AsmCodeGenerator::Instructions::MOV,  instruction_stringify(AsmCodeGenerator::Instructions::MOV)  },
    { AsmCodeGenerator::Instructions::JMP,  instruction_stringify(AsmCodeGenerator::Instructions::JMP)  },
    { AsmCodeGenerator::Instructions::CALL, instruction_stringify(AsmCodeGenerator::Instructions::CALL) }
};

DAddingMethods::DAddingMethods(BinaryFile *f) :
    file(f)
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

    arch_type = {
        { ArchitectureType::BITS32, "[bits 32]" },
        { ArchitectureType::BITS64, "[bits 64]" }
    };
}

uint64_t DAddingMethods::fill_params(QString &code, const QMap<QString, QString> &params) {
    uint64_t cnt = 0;
    foreach (QString param, params.keys()) {
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

bool DAddingMethods::get_addresses(const QByteArray &addr_data, uint8_t addr_size, QList<Elf64_Addr> &addr_list) {
    int data_size = addr_data.size();
    if (data_size % addr_size)
        return false;

    quint64 addr;

    for (int i = 0; i < data_size; i += addr_size) {
        addr = 0;
        for (int j = 0; j < addr_size; ++j)
            addr |= (static_cast<const char>((addr_data.data())[i + j]) & 0xff) << (j * 8);
        if (addr)
            addr_list.push_back(addr);
    }

    return true;
}

