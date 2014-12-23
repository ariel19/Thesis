#include <QString>
#include <core/adding_methods/wrappers/linux/asmcodegenerator.h>

#define enum_stringify(enum_val) \
    QString((std::string(#enum_val).substr(std::string(#enum_val).find_last_of(':') != std::string::npos ? \
                                  std::string(#enum_val).find_last_of(':') + 1 : 0)).c_str()).toLower()

#define instruction_stringify(instruction) \
    QString((std::string(#instruction).substr(std::string(#instruction).find_last_of(':') != std::string::npos ? \
                                  std::string(#instruction).find_last_of(':') + 1 : 0)).c_str()).toLower()

const QMap<DAddingMethods::Registers_x86, QString> AsmCodeGenerator::regs_x86 = {
    { DAddingMethods::Registers_x86::EAX, enum_stringify(DAddingMethods::Registers_x86::EAX) },
    { DAddingMethods::Registers_x86::EBX, enum_stringify(DAddingMethods::Registers_x86::EBX) },
    { DAddingMethods::Registers_x86::ECX, enum_stringify(DAddingMethods::Registers_x86::ECX) },
    { DAddingMethods::Registers_x86::EDX, enum_stringify(DAddingMethods::Registers_x86::EDX) },
    { DAddingMethods::Registers_x86::EDI, enum_stringify(DAddingMethods::Registers_x86::EDI) },
    { DAddingMethods::Registers_x86::EBP, enum_stringify(DAddingMethods::Registers_x86::EBP) },
    { DAddingMethods::Registers_x86::ESI, enum_stringify(DAddingMethods::Registers_x86::ESI) },
    { DAddingMethods::Registers_x86::ESP, enum_stringify(DAddingMethods::Registers_x86::ESP) }
};

const QMap<DAddingMethods::Registers_x64, QString> AsmCodeGenerator::regs_x64 = {
    { DAddingMethods::Registers_x64::RAX, enum_stringify(DAddingMethods::Registers_x64::RAX) },
    { DAddingMethods::Registers_x64::RBX, enum_stringify(DAddingMethods::Registers_x64::RBX) },
    { DAddingMethods::Registers_x64::RCX, enum_stringify(DAddingMethods::Registers_x64::RCX) },
    { DAddingMethods::Registers_x64::RDX, enum_stringify(DAddingMethods::Registers_x64::RDX) },
    { DAddingMethods::Registers_x64::RDI, enum_stringify(DAddingMethods::Registers_x64::RDI) },
    { DAddingMethods::Registers_x64::RSI, enum_stringify(DAddingMethods::Registers_x64::RSI) },
    { DAddingMethods::Registers_x64::RBP, enum_stringify(DAddingMethods::Registers_x64::RBP) },
    { DAddingMethods::Registers_x64::RSP, enum_stringify(DAddingMethods::Registers_x64::RSP) },
    { DAddingMethods::Registers_x64::R8 , enum_stringify(DAddingMethods::Registers_x64::R8)  },
    { DAddingMethods::Registers_x64::R9 , enum_stringify(DAddingMethods::Registers_x64::R9)  },
    { DAddingMethods::Registers_x64::R10, enum_stringify(DAddingMethods::Registers_x64::R10) },
    { DAddingMethods::Registers_x64::R11, enum_stringify(DAddingMethods::Registers_x64::R11) },
    { DAddingMethods::Registers_x64::R12, enum_stringify(DAddingMethods::Registers_x64::R12) },
    { DAddingMethods::Registers_x64::R13, enum_stringify(DAddingMethods::Registers_x64::R13) },
    { DAddingMethods::Registers_x64::R14, enum_stringify(DAddingMethods::Registers_x64::R14) },
    { DAddingMethods::Registers_x64::R15, enum_stringify(DAddingMethods::Registers_x64::R15) }
};

const QMap<AsmCodeGenerator::Instructions, QString> AsmCodeGenerator::instructions = {
    { AsmCodeGenerator::Instructions::POP , instruction_stringify(AsmCodeGenerator::Instructions::POP)  },
    { AsmCodeGenerator::Instructions::PUSH, instruction_stringify(AsmCodeGenerator::Instructions::PUSH) }
};

template <typename RegistersType>
QString AsmCodeGenerator::push_regs(const RegistersType reg) {
    return std::is_same<RegistersType, DAddingMethods::Registers_x86>::value ?
               QString("%1 %2\n").arg(instructions[Instructions::PUSH], regs_x86[reg]) :
               std::is_same<RegistersType, DAddingMethods::Registers_x64>::value ?
                   QString("%1 %2\n").arg(instructions[Instructions::PUSH], regs_x64[reg]) :
                   QString();
}

template <typename RegistersType>
QString AsmCodeGenerator::push_regs(const QList<RegistersType> &regs) {
    QString gen_code;
    if (std::is_same<RegistersType, DAddingMethods::Registers_x86>::value)
        foreach (RegistersType reg, regs)
            gen_code.append(QString("%1 %2\n").arg(instructions[Instructions::PUSH], regs_x86[reg]));
    else if (std::is_same<RegistersType, DAddingMethods::Registers_x64>::value)
        foreach (RegistersType reg, regs)
            gen_code.append(QString("%1 %2\n").arg(instructions[Instructions::PUSH], regs_x64[reg]));
    return gen_code;
}

template <typename RegistersType>
QString AsmCodeGenerator::pop_regs(const RegistersType reg) {
    return std::is_same<RegistersType, DAddingMethods::Registers_x86>::value ?
               QString("%1 %2\n").arg(instructions[Instructions::POP], regs_x86[reg]) :
               std::is_same<RegistersType, DAddingMethods::Registers_x64>::value ?
                   QString("%1 %2\n").arg(instructions[Instructions::POP], regs_x64[reg]) :
                   QString();
}

template <typename RegistersType>
QString AsmCodeGenerator::pop_regs(const QList<RegistersType> &regs) {
    QString gen_code;
    if (std::is_same<RegistersType, DAddingMethods::Registers_x86>::value)
        foreach (RegistersType reg, regs)
            gen_code.append(QString("%1 %2\n").arg(instructions[Instructions::POP], regs_x86[reg]));
    else if (std::is_same<RegistersType, DAddingMethods::Registers_x64>::value)
        foreach (RegistersType reg, regs)
            gen_code.append(QString("%1 %2\n").arg(instructions[Instructions::POP], regs_x64[reg]));
    return gen_code;
}
