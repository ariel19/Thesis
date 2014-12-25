#include "pecodedefines.h"

template <>
const uint8_t PECodeDefines<Register_x86>::_stack_cell_size = 4;

template <>
const uint8_t PECodeDefines<Register_x64>::_stack_cell_size = 8;

template <>
const QByteArray PECodeDefines<Register_x86>::startFunc = QByteArray("\x55\x89\xE5");

template <>
const QByteArray PECodeDefines<Register_x86>::endFunc = QByteArray("\x5D");

template <>
const QByteArray PECodeDefines<Register_x86>::_jz_rel = QByteArray("\x74");

template <>
const QByteArray PECodeDefines<Register_x86>::_jmp_rel = QByteArray("\xEB");

template <>
const QByteArray PECodeDefines<Register_x86>::_reserve_stack = QByteArray("\x83\xEC");

template <>
const QByteArray PECodeDefines<Register_x86>::_clear_stack = QByteArray("\x83\xC4");

template <>
const QByteArray PECodeDefines<Register_x86>::_store_dword = QByteArray("\x68");

template <>
const QByteArray PECodeDefines<Register_x86>::_ret_n = QByteArray("\xC2");

template <>
const QByteArray PECodeDefines<Register_x86>::ret = QByteArray("\xC3");

template <>
const QList<Register_x86> PECodeDefines<Register_x86>::internalRegs =
{
    Register_x86::EAX,
    Register_x86::ECX,
    Register_x86::EDX
};

template <>
const QList<Register_x86> PECodeDefines<Register_x86>::externalRegs =
{
    Register_x86::EBX,
    Register_x86::ESP,
    Register_x86::EBP,
    Register_x86::ESI,
    Register_x86::EDI
};

template <>
const QMap<Register_x86, QByteArray> PECodeDefines<Register_x86>::_save_reg =
{
    { Register_x86::EAX, QByteArray("\x50") },
    { Register_x86::EBX, QByteArray("\x53") },
    { Register_x86::ECX, QByteArray("\x51") },
    { Register_x86::EDX, QByteArray("\x52") },
    { Register_x86::ESP, QByteArray("\x54") },
    { Register_x86::EBP, QByteArray("\x55") },
    { Register_x86::ESI, QByteArray("\x56") },
    { Register_x86::EDI, QByteArray("\x57") }
};

template <>
const QMap<Register_x86, QByteArray> PECodeDefines<Register_x86>::_restore_reg =
{
    { Register_x86::EAX, QByteArray("\x58") },
    { Register_x86::EBX, QByteArray("\x5B") },
    { Register_x86::ECX, QByteArray("\x59") },
    { Register_x86::EDX, QByteArray("\x5A") },
    { Register_x86::ESP, QByteArray("\x5C") },
    { Register_x86::EBP, QByteArray("\x5D") },
    { Register_x86::ESI, QByteArray("\x5E") },
    { Register_x86::EDI, QByteArray("\x5F") }
};

template <>
const QMap<Register_x86, QByteArray> PECodeDefines<Register_x86>::_mov_to_reg =
{
    { Register_x86::EAX, QByteArray("\xB8") },
    { Register_x86::EBX, QByteArray("\xBB") },
    { Register_x86::ECX, QByteArray("\xB9") },
    { Register_x86::EDX, QByteArray("\xBA") },
    { Register_x86::ESP, QByteArray("\xBC") },
    { Register_x86::EBP, QByteArray("\xBD") },
    { Register_x86::ESI, QByteArray("\xBE") },
    { Register_x86::EDI, QByteArray("\xBF") }
};

template <>
const QMap<Register_x86, QByteArray> PECodeDefines<Register_x86>::_call_reg =
{
    { Register_x86::EAX, QByteArray("\xFF\xD0") },
    { Register_x86::EBX, QByteArray("\xFF\xD3") },
    { Register_x86::ECX, QByteArray("\xFF\xD1") },
    { Register_x86::EDX, QByteArray("\xFF\xD2") },
    { Register_x86::ESP, QByteArray("\xFF\xD4") },
    { Register_x86::EBP, QByteArray("\xFF\xD5") },
    { Register_x86::ESI, QByteArray("\xFF\xD6") },
    { Register_x86::EDI, QByteArray("\xFF\xD7") }
};

template <>
const QMap<Register_x86, QByteArray> PECodeDefines<Register_x86>::_jmp_reg =
{
    { Register_x86::EAX, QByteArray("\xFF\xE0") },
    { Register_x86::EBX, QByteArray("\xFF\xE3") },
    { Register_x86::ECX, QByteArray("\xFF\xE1") },
    { Register_x86::EDX, QByteArray("\xFF\xE2") },
    { Register_x86::ESP, QByteArray("\xFF\xE4") },
    { Register_x86::EBP, QByteArray("\xFF\xE5") },
    { Register_x86::ESI, QByteArray("\xFF\xE6") },
    { Register_x86::EDI, QByteArray("\xFF\xE7") }
};

template <>
const QMap<Register_x86, QByteArray> PECodeDefines<Register_x86>::_test_reg =
{
    { Register_x86::EAX, QByteArray("\x85\xC0") },
    { Register_x86::EBX, QByteArray("\x85\xDB") },
    { Register_x86::ECX, QByteArray("\x85\xC9") },
    { Register_x86::EDX, QByteArray("\x85\xD2") },
    { Register_x86::ESP, QByteArray("\x85\xE4") },
    { Register_x86::EBP, QByteArray("\x85\xED") },
    { Register_x86::ESI, QByteArray("\x85\xF6") },
    { Register_x86::EDI, QByteArray("\x85\xFF") }
};

template <>
const QMap<Register_x86, QByteArray> PECodeDefines<Register_x86>::_esp_mem_to_reg =
{
    { Register_x86::EAX, QByteArray("\x8B\x44\x24") },
    { Register_x86::EBX, QByteArray("\x8B\x5C\x24") },
    { Register_x86::ECX, QByteArray("\x8B\x4C\x24") },
    { Register_x86::EDX, QByteArray("\x8B\x54\x24") },
    { Register_x86::ESP, QByteArray("\x8B\x64\x24") },
    { Register_x86::EBP, QByteArray("\x8B\x6C\x24") },
    { Register_x86::ESI, QByteArray("\x8B\x74\x24") },
    { Register_x86::EDI, QByteArray("\x8B\x7C\x24") }
};

template <>
const QMap<Register_x86, QByteArray> PECodeDefines<Register_x86>::_reg_to_esp_mem =
{
    { Register_x86::EAX, QByteArray("\x89\x44\x24") },
    { Register_x86::EBX, QByteArray("\x89\x5C\x24") },
    { Register_x86::ECX, QByteArray("\x89\x4C\x24") },
    { Register_x86::EDX, QByteArray("\x89\x54\x24") },
    { Register_x86::ESP, QByteArray("\x89\x64\x24") },
    { Register_x86::EBP, QByteArray("\x89\x6C\x24") },
    { Register_x86::ESI, QByteArray("\x89\x74\x24") },
    { Register_x86::EDI, QByteArray("\x89\x7C\x24") }
};

template <typename Register>
QByteArray PECodeDefines<Register>::saveRegister(Register reg)
{
    return _save_reg.contains(reg) ? _save_reg[reg] : QByteArray("");
}

template <typename Register>
QByteArray PECodeDefines<Register>::restoreRegister(Register reg)
{
    return _restore_reg.contains(reg) ? _restore_reg[reg] : QByteArray("");
}

template <>
template <>
QByteArray PECodeDefines<Register_x86>::movValueToReg(uint32_t value, Register_x86 reg)
{
    QByteArray code;

    if(_mov_to_reg.contains(reg))
    {
        code.append(_mov_to_reg[reg]);
        code.append(reinterpret_cast<const char*>(&value), sizeof(uint32_t));
    }

    return code;
}

template <typename Register>
QByteArray PECodeDefines<Register>::callReg(Register reg)
{
    return _call_reg.contains(reg) ? _call_reg[reg] : QByteArray("");
}

template <typename Register>
QByteArray PECodeDefines<Register>::jmpReg(Register reg)
{
    return _jmp_reg.contains(reg) ? _jmp_reg[reg] : QByteArray("");
}

template <typename Register>
QByteArray PECodeDefines<Register>::testReg(Register reg)
{
    return _test_reg.contains(reg) ? _test_reg[reg] : QByteArray("");
}

template <typename Register>
QByteArray PECodeDefines<Register>::jzRelative(int8_t pos)
{
    return QByteArray(_jz_rel).append(static_cast<char>(pos));
}

template <typename Register>
QByteArray PECodeDefines<Register>::jmpRelative(int8_t pos)
{
    return QByteArray(_jmp_rel).append(static_cast<char>(pos));
}

template <typename Register>
QByteArray PECodeDefines<Register>::saveAllInternal()
{
    QByteArray code;

    foreach (Register r, internalRegs)
        code.append(saveRegister(r));

    return code;
}

template <typename Register>
QByteArray PECodeDefines<Register>::restoreAllInternal()
{
    QByteArray code;

    foreach (Register r, internalRegs)
        code.append(restoreRegister(r));

    return code;
}

template <typename Register>
QByteArray PECodeDefines<Register>::reserveStackSpace(uint16_t noParams)
{
    QByteArray code;
    int bytes = noParams * _stack_cell_size;

    while(bytes > 0)
    {
        uint8_t tmp_bytes = bytes > 64 ? 64 : bytes;
        bytes -= tmp_bytes;
        code.append(_reserve_stack);
        code.append(static_cast<char>(tmp_bytes));
    }

    return code;
}

template <typename Register>
QByteArray PECodeDefines<Register>::clearStackSpace(uint16_t noParams)
{
    QByteArray code;
    int bytes = noParams * _stack_cell_size;

    while(bytes > 0)
    {
        uint8_t tmp_bytes = bytes > 64 ? 64 : bytes;
        bytes -= tmp_bytes;
        code.append(_clear_stack);
        code.append(static_cast<char>(tmp_bytes));
    }

    return code;
}

template <>
template <>
QByteArray PECodeDefines<Register_x86>::storeValue(uint32_t dword)
{
    return QByteArray(_store_dword).append(reinterpret_cast<const char*>(&dword), sizeof(uint32_t));
}

template <typename Register>
QByteArray PECodeDefines<Register>::readFromEspMemToReg(Register reg, int8_t base)
{
    QByteArray code;

    if(_esp_mem_to_reg.contains(reg))
    {
        code.append(_esp_mem_to_reg[reg]);
        code.append(static_cast<char>(base));
    }

    return code;
}

template <typename Register>
QByteArray PECodeDefines<Register>::readFromRegToEspMem(Register reg, int8_t base)
{
    QByteArray code;

    if(_reg_to_esp_mem.contains(reg))
    {
        code.append(_reg_to_esp_mem[reg]);
        code.append(static_cast<char>(base));
    }

    return code;
}

template <typename Register>
QByteArray PECodeDefines<Register>::retN(uint16_t n)
{
    return QByteArray(_ret_n).append(reinterpret_cast<const char*>(&n), sizeof(uint16_t));
}
