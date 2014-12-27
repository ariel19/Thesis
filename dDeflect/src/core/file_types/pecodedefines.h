#ifndef PECODEDEFINES_H
#define PECODEDEFINES_H

#include <QByteArray>
#include <QSet>
#include <core/file_types/pehelpers.h>

template <typename Register>
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
    static const uint8_t _stack_cell_size;

public:
    static const QByteArray startFunc;
    static const QByteArray endFunc;
    static const QByteArray ret;
    static const QList<Register> internalRegs;
    static const QList<Register> externalRegs;
    static const uint8_t shadowSize;
    static const uint8_t align16Size;

    static QByteArray saveRegister(Register reg);
    static QByteArray restoreRegister(Register reg);
    template <typename T>
    static QByteArray movValueToReg(T value, Register reg);
    static QByteArray callReg(Register reg);
    static QByteArray jmpReg(Register reg);
    static QByteArray testReg(Register reg);
    static QByteArray jzRelative(int8_t pos);
    static QByteArray jmpRelative(int8_t pos);
    static QByteArray saveAllInternal();
    static QByteArray restoreAllInternal();
    static QByteArray reserveStackSpace(uint16_t noParams);
    static QByteArray clearStackSpace(uint16_t noParams);
    template <typename T>
    static QByteArray storeValue(T dword);
    static QByteArray readFromEspMemToReg(Register reg, int8_t base);
    static QByteArray readFromRegToEspMem(Register reg, int8_t base);
    static QByteArray retN(uint16_t n);
};

#endif // PECODEDEFINES_H
