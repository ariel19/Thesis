#ifndef DADDINGMETHODS_H
#define DADDINGMETHODS_H

#include <QList>
#include <QMap>
#include <QString>
#include <QDebug>
#include <QProcess>

#include <core/file_types/elffile.h>
#include <core/file_types/codedefines.h>

#define mnemonic_stringify(mnemonic) \
    QString((std::string(#mnemonic).substr(std::string(#mnemonic).find_last_of(':') != std::string::npos ? \
                                  std::string(#mnemonic).find_last_of(':') + 1 : 0)).c_str()).toLower()

#define enum_stringify(enum_val) \
    QString((std::string(#enum_val).substr(std::string(#enum_val).find_last_of(':') != std::string::npos ? \
                                  std::string(#enum_val).find_last_of(':') + 1 : 0)).c_str()).toLower()

#define instruction_stringify(instruction) \
    QString((std::string(#instruction).substr(std::string(#instruction).find_last_of(':') != std::string::npos ? \
                                  std::string(#instruction).find_last_of(':') + 1 : 0)).c_str()).toLower()

class DAddingMethods {
public:

    /**
     * @brief Typ kompilowanego pliku assembly.
     */
    enum class ArchitectureType {
        BITS32,
        BITS64
    };

    /**
     * @brief Typy możliwości wstrzyknięcia kodu.
     */
    enum class CallingMethod {
        OEP,
        Thread,
        Trampoline,
        INIT,
        INIT_ARRAY,
        CTORS,
        TLS
    };


    /**
     * @brief Klasa bazowa reprezentująca opakowanie dla kawałków kodu.
     */
    template <typename RegistersType>
    class Wrapper {
    public:
        QList<RegistersType> used_regs;
        QMap<RegistersType, QString> dynamic_params;
        QMap<QString, QString> static_params;
        RegistersType ret;
        QByteArray code;
        Wrapper<RegistersType> *detect_handler;

        virtual ~Wrapper() {}
    };

    /**
     * @brief Klasa reprezentująca opakowanie dla tworzenia nowego wątku.
     */
    template <typename RegistersType>
    class ThreadWrapper : public Wrapper<RegistersType> {
    public:
        QList<Wrapper<RegistersType>*> thread_actions;
        uint16_t sleep_time;
    };

    /**
     * @brief Klasa reprezentująca opakowanie dla tworzenia nowego punktu wejściowego.
     */
    template <typename RegistersType>
    class OEPWrapper : public Wrapper<RegistersType> {
    public:
        Wrapper<RegistersType> *oep_action;
    };

    /**
     * @brief Klasa reprezentująca opakowanie dla tworzenia tramplin w funkcjach bibliotecznych.
     */
    template <typename RegistersType>
    class TrampolineWrapper : public Wrapper<RegistersType> {
    public:
        Wrapper<RegistersType> *tramp_action;
    };

    /**
     * @brief Klasa opisująca metodę wstrzykiwania kodu.
     */
    template <typename RegistersType>
    class InjectDescription {
    public:
        CallingMethod cm;
        Wrapper<RegistersType> *adding_method;
        QString saved_fname;
        bool change_x_only;
    };

    /**
     * @brief Konstruktor.
     */
    DAddingMethods(BinaryFile *f);

protected:
    BinaryFile *file;
    QMap<ArchitectureType, QString> arch_type;
};

class AsmCodeGenerator {
    static const QMap<Registers_x86, QString> regs_x86;
    static const QMap<Registers_x64, QString> regs_x64;

    enum class Instructions {
        POP,
        PUSH,
        MOV,
        JMP,
        CALL
    };

    static const QMap<Instructions, QString> instructions;

public:
    AsmCodeGenerator() {}

    template <typename RegistersType>
    static QString push_regs(const RegistersType reg);

    template <typename RegistersType>
    static QString push_regs(const QList<RegistersType> &regs);

    template <typename RegistersType>
    static QString pop_regs(const RegistersType reg);

    template <typename RegistersType>
    static QString pop_regs(const QList<RegistersType> &regs);

    template <typename RegistersType>
    static QString mov_reg_const(const RegistersType reg, Elf64_Addr value);

    template <typename RegistersType>
    static QString jmp_reg(const RegistersType reg);

    template <typename RegistersType>
    static QString get_reg(const RegistersType reg);

    template <typename RegistersType>
    static QString call_reg(const RegistersType);

    static QString call_const(Elf64_Addr value) {
        return QString("%1 %2\n").arg(instructions[Instructions::CALL], QString::number(value));
    }
};

template <typename RegistersType>
QString AsmCodeGenerator::push_regs(const RegistersType reg) {
    return std::is_same<RegistersType, Registers_x86>::value ?
               QString("%1 %2\n").arg(instructions[Instructions::PUSH], regs_x86[reg]) :
               std::is_same<RegistersType, Registers_x64>::value ?
                   QString("%1 %2\n").arg(instructions[Instructions::PUSH], regs_x64[reg]) :
                   QString();
}

template <typename RegistersType>
QString AsmCodeGenerator::push_regs(const QList<RegistersType> &regs) {
    QString gen_code;
    if (std::is_same<RegistersType, Registers_x86>::value)
        foreach (RegistersType reg, regs)
            gen_code.append(QString("%1 %2\n").arg(instructions[Instructions::PUSH],
                            regs_x86[static_cast<Registers_x86>(reg)]));
    else if (std::is_same<RegistersType, Registers_x64>::value)
        foreach (RegistersType reg, regs)
            gen_code.append(QString("%1 %2\n").arg(instructions[Instructions::PUSH],
                            regs_x64[static_cast<Registers_x64>(reg)]));
    return gen_code;
}

template <typename RegistersType>
QString AsmCodeGenerator::pop_regs(const RegistersType reg) {
    return std::is_same<RegistersType, Registers_x86>::value ?
               QString("%1 %2\n").arg(instructions[Instructions::POP], regs_x86[reg]) :
               std::is_same<RegistersType, Registers_x64>::value ?
                   QString("%1 %2\n").arg(instructions[Instructions::POP], regs_x64[reg]) :
                   QString();
}

template <typename RegistersType>
QString AsmCodeGenerator::pop_regs(const QList<RegistersType> &regs) {
    QString gen_code;
    if (std::is_same<RegistersType, Registers_x86>::value)
        foreach (RegistersType reg, regs)
            gen_code.append(QString("%1 %2\n").arg(instructions[Instructions::POP],
                            regs_x86[static_cast<Registers_x86>(reg)]));
    else if (std::is_same<RegistersType, Registers_x64>::value)
        foreach (RegistersType reg, regs)
            gen_code.append(QString("%1 %2\n").arg(instructions[Instructions::POP],
                            regs_x64[static_cast<Registers_x64>(reg)]));
    return gen_code;
}

template <typename RegistersType>
QString AsmCodeGenerator::mov_reg_const(const RegistersType reg, Elf64_Addr value) {
    QString qreg = std::is_same<RegistersType, Registers_x86>::value ?
                        regs_x86[static_cast<Registers_x86>(reg)] :
                            std::is_same<RegistersType, Registers_x64>::value ?
                                regs_x64[static_cast<Registers_x64>(reg)] : "xxx";
    return QString("%1 %2, %3\n").arg(instructions[Instructions::MOV], qreg, QString::number(value));
}

template <typename RegistersType>
QString AsmCodeGenerator::jmp_reg(const RegistersType reg) {
    QString qreg = std::is_same<RegistersType, Registers_x86>::value ?
                        regs_x86[static_cast<Registers_x86>(reg)] :
                            std::is_same<RegistersType, Registers_x64>::value ?
                                regs_x64[static_cast<Registers_x64>(reg)] : "xxx";
    return QString("%1 %2\n").arg(instructions[Instructions::JMP], qreg);
}

template <typename RegistersType>
QString AsmCodeGenerator::get_reg(const RegistersType reg) {
    return std::is_same<RegistersType, Registers_x86>::value ?
                regs_x86[static_cast<Registers_x86>(reg)] :
                    std::is_same<RegistersType, Registers_x64>::value ?
                        regs_x64[static_cast<Registers_x64>(reg)] : "xxx";
}

template <typename RegistersType>
QString AsmCodeGenerator::call_reg(const RegistersType reg) {
    QString qreg = std::is_same<RegistersType, Registers_x86>::value ?
                        regs_x86[static_cast<Registers_x86>(reg)] :
                            std::is_same<RegistersType, Registers_x64>::value ?
                                regs_x64[static_cast<Registers_x64>(reg)] : "xxx";
    return QString("%1 %2\n").arg(instructions[Instructions::CALL], qreg);
}

#endif // DADDINGMETHODS_H
