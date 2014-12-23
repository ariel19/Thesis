#ifndef ASMCODEGENERATOR_H
#define ASMCODEGENERATOR_H

#include <QMap>
#include <QString>
#include <core/adding_methods/wrappers/linux/daddingmethods.h>

class AsmCodeGenerator {
    static const QMap<DAddingMethods::Registers_x86, QString> regs_x86;
    static const QMap<DAddingMethods::Registers_x64, QString> regs_x64;

    enum class Instructions {
        POP,
        PUSH
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
};

#endif // ASMCODEGENERATOR_H
