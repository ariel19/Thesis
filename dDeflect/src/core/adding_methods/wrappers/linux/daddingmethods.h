#ifndef DADDINGMETHODS_H
#define DADDINGMETHODS_H

#include <QList>
#include <QMap>
#include <QString>

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
     * @brief Rejestry dla architektury x86
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
     * @brief Rejestry dla architektury x64
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

    template <typename RegistersType>
    class Wrapper {
        QList<RegistersType> used_regs;
        QMap<QString, RegistersType> params;
        RegistersType ret;
        QString code;
    };

    template <typename RegistersType>
    class ThreadWrapper : public Wrapper<RegistersType> {
        Wrapper<RegistersType> *thread_action;
    };

    template <typename RegistersType>
    class OEPWrapper : public Wrapper<RegistersType> {
        Wrapper<RegistersType> *oep_action;
    };

    template <typename RegistersType>
    class TrampolineWrapper : public Wrapper<RegistersType> {

    };


    DAddingMethods() {}
};

#endif // DADDINGMETHODS_H
