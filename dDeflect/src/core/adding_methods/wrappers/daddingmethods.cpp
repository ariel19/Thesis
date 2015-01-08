#include <QProcess>
#include <QString>
#include <QDebug>
#include <QDataStream>

#include <core/adding_methods/wrappers/daddingmethods.h>

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
    arch_type = {
        { ArchitectureType::BITS32, "[bits 32]" },
        { ArchitectureType::BITS64, "[bits 64]" }
    };
}

template <typename Reg>
const QMap<QString, typename DAddingMethods::Wrapper<Reg>::WrapperType> DAddingMethods::Wrapper<Reg>::wrapperTypes =
{
    { "Handler", DAddingMethods::Wrapper<Reg>::WrapperType::Handler },
    { "Method", DAddingMethods::Wrapper<Reg>::WrapperType::Method },
    { "Helper", DAddingMethods::Wrapper<Reg>::WrapperType::Helper },
    { "ThreadWrapper", DAddingMethods::Wrapper<Reg>::WrapperType::ThreadWrapper },
    { "OEPWrapper", DAddingMethods::Wrapper<Reg>::WrapperType::OepWrapper },
    { "TrampolineWrapper", DAddingMethods::Wrapper<Reg>::WrapperType::TrampolineWrapper }
};
template const QMap<QString, DAddingMethods::Wrapper<Registers_x86>::WrapperType> DAddingMethods::Wrapper<Registers_x86>::wrapperTypes;
template const QMap<QString, DAddingMethods::Wrapper<Registers_x64>::WrapperType> DAddingMethods::Wrapper<Registers_x64>::wrapperTypes;

const QMap<QString, DAddingMethods::CallingMethod> DAddingMethods::callingMethods =
{
    { "EntryPoint", DAddingMethods::CallingMethod::OEP },
    { "Thread", DAddingMethods::CallingMethod::Thread },
    { "Trampoline", DAddingMethods::CallingMethod::Trampoline },
    { "INIT", DAddingMethods::CallingMethod::INIT },
    { "INIT_ARRAY", DAddingMethods::CallingMethod::INIT_ARRAY },
    { "CTORS", DAddingMethods::CallingMethod::CTORS },
    { "TLS", DAddingMethods::CallingMethod::TLS }
};

template <>
const QMap<QString, Registers_x86> DAddingMethods::Wrapper<Registers_x86>::registerTypes =
{
    { "EAX", Registers_x86::EAX },
    { "EBX", Registers_x86::EBX },
    { "ECX", Registers_x86::ECX },
    { "EDX", Registers_x86::EDX },
    { "ESI", Registers_x86::ESI },
    { "EDI", Registers_x86::EDI },
    { "ESP", Registers_x86::ESP },
    { "EBP", Registers_x86::EBP },
    { "None", Registers_x86::None }
};

template <>
const QMap<QString, Registers_x64> DAddingMethods::Wrapper<Registers_x64>::registerTypes =
{
    { "RAX", Registers_x64::RAX },
    { "RBX", Registers_x64::RAX },
    { "RCX", Registers_x64::RAX },
    { "RDX", Registers_x64::RAX },
    { "RSI", Registers_x64::RAX },
    { "RDI", Registers_x64::RAX },
    { "RSP", Registers_x64::RAX },
    { "RBP", Registers_x64::RAX },
    { "R8", Registers_x64::RAX },
    { "R9", Registers_x64::RAX },
    { "R10", Registers_x64::RAX },
    { "R11", Registers_x64::RAX },
    { "R12", Registers_x64::RAX },
    { "R13", Registers_x64::RAX },
    { "R14", Registers_x64::RAX },
    { "R15", Registers_x64::RAX },
    { "None", Registers_x64::None }
};
