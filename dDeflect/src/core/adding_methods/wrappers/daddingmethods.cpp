#include <core/adding_methods/wrappers/daddingmethods.h>

#include <chrono>
#include <QProcess>

#include <ApplicationManager/dsettings.h>
#include <ApplicationManager/dlogger.h>

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

template <typename Reg>
DAddingMethods<Reg>::DAddingMethods(BinaryFile *f) :
    file(f),
    r_gen(std::chrono::system_clock::now().time_since_epoch().count())
{
    arch_type = {
        { ArchitectureType::BITS32, "[bits 32]" },
        { ArchitectureType::BITS64, "[bits 64]" }
    };
}
template DAddingMethods<Registers_x86>::DAddingMethods(BinaryFile *f);
template DAddingMethods<Registers_x64>::DAddingMethods(BinaryFile *f);

template <typename Register>
bool DAddingMethods<Register>::pack(QString file_path, DAddingMethods::CompressionLevel level, DAddingMethods::CompressionOptions opt)
{
    QProcess upx;
    QString upx_path = DSettings::getSettings().getUpxPath();
    QList<QString> params = { "-f" };

    if(level == CompressionLevel::BEST)
        params.append("--best");
    else
        params.append(QString("-%1").arg(static_cast<int>(level)));

    if(opt == CompressionOptions::Brute)
        params.append("--brute");
    if(opt == CompressionOptions::Ultra)
        params.append("--ultra-brute");

    params.append(file_path);

    LOG_MSG("Starting packing file with UPX. This may take a while...");

    upx.start(upx_path, params);

    if(!upx.waitForFinished(5 * 60 * 1000))
    {
        LOG_ERROR("UPX failed.");
        return false;
    }

    LOG_MSG("Done.");

    return true;
}
template bool DAddingMethods<Registers_x86>::pack(QString file_path, DAddingMethods<Registers_x86>::CompressionLevel level, DAddingMethods<Registers_x86>::CompressionOptions opt);
template bool DAddingMethods<Registers_x64>::pack(QString file_path, DAddingMethods<Registers_x64>::CompressionLevel level, DAddingMethods<Registers_x64>::CompressionOptions opt);

template <typename Reg>
const QMap<QString, typename DAddingMethods<Reg>::Wrapper::WrapperType> DAddingMethods<Reg>::Wrapper::wrapperTypes =
{
    { "Method", DAddingMethods<Reg>::Wrapper::WrapperType::Method },
    { "Handler", DAddingMethods<Reg>::Wrapper::WrapperType::Handler },
    { "Helper", DAddingMethods<Reg>::Wrapper::WrapperType::Helper },
    { "ThreadWrapper", DAddingMethods<Reg>::Wrapper::WrapperType::ThreadWrapper },
    { "OEPWrapper", DAddingMethods<Reg>::Wrapper::WrapperType::OepWrapper },
    { "TrampolineWrapper", DAddingMethods<Reg>::Wrapper::WrapperType::TrampolineWrapper }
};
template const QMap<QString, DAddingMethods<Registers_x86>::Wrapper::WrapperType> DAddingMethods<Registers_x86>::Wrapper::wrapperTypes;
template const QMap<QString, DAddingMethods<Registers_x64>::Wrapper::WrapperType> DAddingMethods<Registers_x64>::Wrapper::wrapperTypes;

template <typename Reg>
const QMap<QString, typename DAddingMethods<Reg>::CallingMethod> DAddingMethods<Reg>::callingMethods =
{
    { "EntryPoint", DAddingMethods<Reg>::CallingMethod::OEP },
    { "Thread", DAddingMethods<Reg>::CallingMethod::Thread },
    { "Trampoline", DAddingMethods<Reg>::CallingMethod::Trampoline },
    { "INIT", DAddingMethods<Reg>::CallingMethod::INIT },
    { "INIT_ARRAY", DAddingMethods<Reg>::CallingMethod::INIT_ARRAY },
    { "CTORS", DAddingMethods<Reg>::CallingMethod::CTORS },
    { "TLS", DAddingMethods<Reg>::CallingMethod::TLS }
};
template const QMap<QString, DAddingMethods<Registers_x86>::CallingMethod> DAddingMethods<Registers_x86>::callingMethods;
template const QMap<QString, DAddingMethods<Registers_x64>::CallingMethod> DAddingMethods<Registers_x64>::callingMethods;

template <>
const QMap<QString, Registers_x86> DAddingMethods<Registers_x86>::Wrapper::registerTypes =
{
    { "EAX", Registers_x86::EAX },
    { "EBX", Registers_x86::EBX },
    { "ECX", Registers_x86::ECX },
    { "EDX", Registers_x86::EDX },
    { "ESI", Registers_x86::ESI },
    { "EDI", Registers_x86::EDI },
    { "ESP", Registers_x86::ESP },
    { "EBP", Registers_x86::EBP },
    { "None", Registers_x86::None },
    { "All", Registers_x86::All }
};

template <>
const QMap<QString, Registers_x64> DAddingMethods<Registers_x64>::Wrapper::registerTypes =
{
    { "RAX", Registers_x64::RAX },
    { "RBX", Registers_x64::RBX },
    { "RCX", Registers_x64::RCX },
    { "RDX", Registers_x64::RDX },
    { "RSI", Registers_x64::RSI },
    { "RDI", Registers_x64::RDI },
    { "RSP", Registers_x64::RSP },
    { "RBP", Registers_x64::RBP },
    { "R8",  Registers_x64::R8 },
    { "R9",  Registers_x64::R9 },
    { "R10", Registers_x64::R10 },
    { "R11", Registers_x64::R11 },
    { "R12", Registers_x64::R12 },
    { "R13", Registers_x64::R13 },
    { "R14", Registers_x64::R14 },
    { "R15", Registers_x64::R15 },
    { "None",Registers_x64::None },
    { "All", Registers_x64::All }
};
