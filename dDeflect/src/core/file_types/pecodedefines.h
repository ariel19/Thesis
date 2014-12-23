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
    static const QMap<Register, QByteArray> _esp_mem_to_reg;
    static const QMap<Register, QByteArray> _reg_to_esp_mem;
    static const QMap<Register, QByteArray> _jmp_reg;

    static const QByteArray _jz_rel;
    static const QByteArray _jmp_rel;
    static const QByteArray _reserve_stack;
    static const QByteArray _clear_stack;
    static const QByteArray _store_dword;
    static const QByteArray _ret_n;

public:
    static const QByteArray startFunc;
    static const QByteArray endFunc;
    static const QByteArray ret;
    static const QList<Register> internalRegs;
    static const QList<Register> externalRegs;

    static QByteArray saveRegister(Register reg);
    static QByteArray restoreRegister(Register reg);
    static QByteArray movDWordToReg(uint32_t dword, Register reg);
    static QByteArray callReg(Register reg);
    static QByteArray jmpReg(Register reg);
    static QByteArray testReg(Register reg);
    static QByteArray jzRelative(int8_t pos);
    static QByteArray jmpRelative(int8_t pos);
    static QByteArray saveAllInternal();
    static QByteArray restoreAllInternal();
    static QByteArray reserveStackSpace(uint16_t noParams);
    static QByteArray clearStackSpace(uint16_t noParams);
    static QByteArray storeDWord(uint32_t dword);
    static QByteArray readFromEspMemToReg(Register reg, int8_t base);
    static QByteArray readFromRegToEspMem(Register reg, int8_t base);
    static QByteArray retN(uint16_t n);
};

#endif // PECODEDEFINES_H
