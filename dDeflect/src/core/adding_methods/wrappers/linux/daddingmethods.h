#ifndef DADDINGMETHODS_H
#define DADDINGMETHODS_H

#include <QList>
#include <QMap>
#include <QString>
#include <QDebug>

#include <core/file_types/elffile.h>

class DAddingMethods {
public:
    /**
     * @brief Typy możliwości wstrzyknięcia kodu.
     */
    enum class CallingMethod {
        OEP,
        Thread,
        Trampoline
    };

    /**
     * @brief Rejestry dla architektury x86.
     */
    enum class Registers_x86 {
        EAX,
        EBX,
        ECX,
        EDX,
        ESI,
        EDI,
        EBP,
        ESP
    };

    /**
     * @brief Rejestry dla architektury x64.
     */
    enum class Registers_x64 {
        RAX,
        RBX,
        RCX,
        RDX,
        RSI,
        RDI,
        RBP,
        RSP,
        R8,
        R9,
        R10,
        R11,
        R12,
        R13,
        R14,
        R15
    };

    /**
     * @brief Klasa bazowa reprezentująca opakowanie dla kawałków kodu.
     */
    template <typename RegistersType>
    class Wrapper {
    public:
        QList<RegistersType> used_regs;
        QMap<QString, QString> params;
        RegistersType ret;
        QString code;
        Wrapper<RegistersType> *detect_handler;

        virtual ~Wrapper() {}
    };

    /**
     * @brief Klasa reprezentująca opakowanie dla tworzenia nowego wątku.
     */
    template <typename RegistersType>
    class ThreadWrapper : public Wrapper<RegistersType> {
    public:
        Wrapper<RegistersType> *thread_action;
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

    };

    /**
     * @brief Klasa opisująca metodę wstrzykiwania kodu.
     */
    template <typename RegistersType>
    class InjectDescription {
    public:
        CallingMethod cm;
        Wrapper<RegistersType> *adding_method;
    };

    /**
     * @brief Konstruktor.
     */
    DAddingMethods();

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
        DDETECTIONMETHOD
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
};

class AsmCodeGenerator {
    static const QMap<DAddingMethods::Registers_x86, QString> regs_x86;
    static const QMap<DAddingMethods::Registers_x64, QString> regs_x64;

    enum class Instructions {
        POP,
        PUSH,
        MOV,
        JMP
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
            gen_code.append(QString("%1 %2\n").arg(instructions[Instructions::PUSH],
                            regs_x64[static_cast<DAddingMethods::Registers_x64>(reg)]));
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
            gen_code.append(QString("%1 %2\n").arg(instructions[Instructions::POP],
                            regs_x64[static_cast<DAddingMethods::Registers_x64>(reg)]));
    return gen_code;
}

template <typename RegistersType>
QString AsmCodeGenerator::mov_reg_const(const RegistersType reg, Elf64_Addr value) {
    QString qreg = std::is_same<RegistersType, DAddingMethods::Registers_x86>::value ?
                        regs_x86[static_cast<DAddingMethods::Registers_x86>(reg)] :
                            std::is_same<RegistersType, DAddingMethods::Registers_x64>::value ?
                                regs_x64[static_cast<DAddingMethods::Registers_x64>(reg)] : "xxx";
    return QString("%1 %2, %3\n").arg(instructions[Instructions::MOV], qreg, QString::number(value));
}

template <typename RegistersType>
QString AsmCodeGenerator::jmp_reg(const RegistersType reg) {
    QString qreg = std::is_same<RegistersType, DAddingMethods::Registers_x86>::value ?
                        regs_x86[static_cast<DAddingMethods::Registers_x86>(reg)] :
                            std::is_same<RegistersType, DAddingMethods::Registers_x64>::value ?
                                regs_x64[static_cast<DAddingMethods::Registers_x64>(reg)] : "xxx";
    return QString("%1 %2\n").arg(instructions[Instructions::JMP], qreg);
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

    code.append(wrap->code);
    // fill params
    // TODO: use return value
    uint64_t filled_params = fill_params(code, wrap->params);

    // generate pop registers
    code.append(AsmCodeGenerator::pop_regs<RegistersType>(wrap->used_regs));

    return true;
}

template <typename RegistersType>
bool DAddingMethods::secure_elf(ELF &elf, const InjectDescription<RegistersType> &inject_desc) {
    QString code2compile,
            code_ddetect_handler,
            code_ddetect;
    QByteArray compiled_code;

    if (!inject_desc.adding_method)
        return false;

    // check platform version
    if (std::is_same<RegistersType, Registers_x86>::value)
        code2compile.append(QString("[bits 32]\n"));
    else if(std::is_same<RegistersType, Registers_x64>::value)
        code2compile.append(QString("[bits 64]\n"));
    else return false;

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
        if (!wrapper_gen_code<RegistersType>(twrapper->thread_action, code_ddetect))
            return false;
        break;
    }
    case CallingMethod::Trampoline: {

        break;
    }
    default:
        return false;
    }

    // qDebug() << code2compile;

    // 3. merge code
    fill_placeholders(code2compile, code_ddetect_handler, PlaceholderMnemonics::DDETECTIONHANDLER);

    // qDebug() << code2compile;

    fill_placeholders(code2compile, code_ddetect, PlaceholderMnemonics::DDETECTIONMETHOD);

    // qDebug() << code2compile;

    Elf64_Addr oldep;
    if (!elf.get_entry_point(oldep))
        return false;

    // add jump to old original entry point
    // TODO: change
    if (elf.is_x86()) {
        code2compile.append(AsmCodeGenerator::mov_reg_const<Registers_x86>(Registers_x86::EAX, oldep));
        code2compile.append(AsmCodeGenerator::jmp_reg<Registers_x86>(Registers_x86::EAX));
    }
    else {
        code2compile.append(AsmCodeGenerator::mov_reg_const<Registers_x64>(Registers_x64::RAX, oldep));
        code2compile.append(AsmCodeGenerator::jmp_reg<Registers_x64>(Registers_x64::RAX));
    }

    // 4. compile code
    if (!compile(code2compile, compiled_code))
        return false;

    // 5. secure elf file
    Elf64_Addr nva;

    // TODO: parameter for x segment
    QByteArray nf = elf.extend_segment(compiled_code, false, nva);
    if (!nf.length())
        return false;

    if (!elf.set_entry_point(nva, nf))
        return false;

    qDebug() << "new entry point: " << QString("0x%1").arg(nva, 0, 16);

    // TODO: name of file should be changed
    elf.write_to_file("template", nf);

    return true;
}

#endif // DADDINGMETHODS_H
