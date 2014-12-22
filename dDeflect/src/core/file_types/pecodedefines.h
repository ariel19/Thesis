#ifndef PECODEDEFINES_H
#define PECODEDEFINES_H

#include <QByteArray>
#include <QSet>
#include <core/file_types/pehelpers.h>

class PECodeDefines
{
private:
    static const QMap<Register, QByteArray> _save_reg;
    static const QMap<Register, QByteArray> _restore_reg;
    static const QMap<Register, QByteArray> _mov_to_reg;
    static const QMap<Register, QByteArray> _call_reg;
    static const QMap<Register, QByteArray> _test_reg;

    static const QByteArray _jz_rel;

public:
    static const QByteArray startFunc;
    static const QByteArray endFunc;
    static const QList<Register> internalRegs;
    static const QList<Register> externalRegs;

    static QByteArray saveRegister(Register reg);
    static QByteArray restoreRegister(Register reg);
    static QByteArray movDWordToReg(uint32_t dword, Register reg);
    static QByteArray callReg(Register reg);
    static QByteArray testReg(Register reg);
    static QByteArray jzRelative(int8_t pos);
};

#endif // PECODEDEFINES_H
