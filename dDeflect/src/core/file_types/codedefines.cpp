#include "codedefines.h"

template <typename Register>
uint64_t CodeDefines<Register>::seed = 0;

template <typename Register>
const QRegExp CodeDefines<Register>::newLineRegExp = QRegExp("[\r\n]");
template const QRegExp CodeDefines<Registers_x86>::newLineRegExp;
template const QRegExp CodeDefines<Registers_x64>::newLineRegExp;

template <typename Register>
const QRegExp CodeDefines<Register>::callRegExp = QRegExp("^[0-9a-f]{8}  E8[0-9a-f]{6}(00|ff) ", Qt::CaseInsensitive);
template const QRegExp CodeDefines<Registers_x86>::callRegExp;
template const QRegExp CodeDefines<Registers_x64>::callRegExp;

template <typename Register>
const QRegExp CodeDefines<Register>::jmpRegExp = QRegExp("^^[0-9a-f]{8}  E9[0-9a-f]{6}(00|ff) ", Qt::CaseInsensitive);
template const QRegExp CodeDefines<Registers_x86>::jmpRegExp;
template const QRegExp CodeDefines<Registers_x64>::jmpRegExp;

template <>
const uint8_t CodeDefines<Registers_x64>::shadowSize = 4;

template <>
const uint8_t CodeDefines<Registers_x64>::align16Size = 1;

template <>
const uint8_t CodeDefines<Registers_x86>::align16Size = 2;

template <>
const uint8_t CodeDefines<Registers_x86>::stackCellSize = 4;

template <>
const uint8_t CodeDefines<Registers_x64>::stackCellSize = 8;

template <>
const QByteArray CodeDefines<Registers_x86>::startFunc = QByteArray("\x55\x89\xE5");

template <>
const QByteArray CodeDefines<Registers_x64>::startFunc = QByteArray("\x55\x48\x89\xE5");

template <>
const QByteArray CodeDefines<Registers_x86>::_pushad = QByteArray("\x60");

template <>
const QByteArray CodeDefines<Registers_x86>::_popad = QByteArray("\x61");

template <typename Register>
const QByteArray CodeDefines<Register>::endFunc = QByteArray("\x5D");
template const QByteArray CodeDefines<Registers_x86>::endFunc;
template const QByteArray CodeDefines<Registers_x64>::endFunc;

template <typename Register>
const QByteArray CodeDefines<Register>::_jz_rel = QByteArray("\x74");
template const QByteArray CodeDefines<Registers_x86>::_jz_rel;
template const QByteArray CodeDefines<Registers_x64>::_jz_rel;

template <typename Register>
const QByteArray CodeDefines<Register>::_jmp_rel = QByteArray("\xEB");
template const QByteArray CodeDefines<Registers_x86>::_jmp_rel;
template const QByteArray CodeDefines<Registers_x64>::_jmp_rel;

template <>
const QByteArray CodeDefines<Registers_x86>::_reserve_stack = QByteArray("\x83\xEC");

template <>
const QByteArray CodeDefines<Registers_x64>::_reserve_stack = QByteArray("\x48\x83\xEC");

template <>
const QByteArray CodeDefines<Registers_x86>::_clear_stack = QByteArray("\x83\xC4");

template <>
const QByteArray CodeDefines<Registers_x64>::_clear_stack = QByteArray("\x48\x83\xC4");

template <typename Register>
const QByteArray CodeDefines<Register>::_store_value = QByteArray("\x68");

template <>
const QByteArray CodeDefines<Registers_x64>::_store_high_bytes = QByteArray("\xC7\x44\x24\x04");

template <typename Register>
const QByteArray CodeDefines<Register>::_ret_n = QByteArray("\xC2");
template const QByteArray CodeDefines<Registers_x86>::_ret_n;
template const QByteArray CodeDefines<Registers_x64>::_ret_n;

template <typename Register>
const QByteArray CodeDefines<Register>::ret = QByteArray("\xC3");
template const QByteArray CodeDefines<Registers_x86>::ret;
template const QByteArray CodeDefines<Registers_x64>::ret;

template <>
const QList<Registers_x86> CodeDefines<Registers_x86>::internalRegs =
{
    Registers_x86::EAX,
    Registers_x86::ECX,
    Registers_x86::EDX
};

template <>
const QList<Registers_x64> CodeDefines<Registers_x64>::internalRegs =
{
    Registers_x64::RAX,
    Registers_x64::RCX,
    Registers_x64::RDX,
    Registers_x64::R8,
    Registers_x64::R9,
    Registers_x64::R10,
    Registers_x64::R11
};

template <>
const QList<Registers_x86> CodeDefines<Registers_x86>::externalRegs =
{
    Registers_x86::EBX,
    Registers_x86::ESP,
    Registers_x86::EBP,
    Registers_x86::ESI,
    Registers_x86::EDI
};

template <>
const QList<Registers_x64> CodeDefines<Registers_x64>::externalRegs =
{
    Registers_x64::RBX,
    Registers_x64::RSP,
    Registers_x64::RBP,
    Registers_x64::RSI,
    Registers_x64::RDI,
    Registers_x64::R12,
    Registers_x64::R13,
    Registers_x64::R14,
    Registers_x64::R15
};

template <>
const QMap<Registers_x86, QByteArray> CodeDefines<Registers_x86>::_save_reg =
{
    { Registers_x86::EAX, QByteArray("\x50") },
    { Registers_x86::EBX, QByteArray("\x53") },
    { Registers_x86::ECX, QByteArray("\x51") },
    { Registers_x86::EDX, QByteArray("\x52") },
    { Registers_x86::ESP, QByteArray("\x54") },
    { Registers_x86::EBP, QByteArray("\x55") },
    { Registers_x86::ESI, QByteArray("\x56") },
    { Registers_x86::EDI, QByteArray("\x57") }
};

template <>
const QMap<Registers_x64, QByteArray> CodeDefines<Registers_x64>::_save_reg =
{
    { Registers_x64::RAX, QByteArray("\x50") },
    { Registers_x64::RBX, QByteArray("\x53") },
    { Registers_x64::RCX, QByteArray("\x51") },
    { Registers_x64::RDX, QByteArray("\x52") },
    { Registers_x64::RSP, QByteArray("\x54") },
    { Registers_x64::RBP, QByteArray("\x55") },
    { Registers_x64::RSI, QByteArray("\x56") },
    { Registers_x64::RDI, QByteArray("\x57") },
    { Registers_x64::R8, QByteArray("\x41\x50") },
    { Registers_x64::R9, QByteArray("\x41\x51") },
    { Registers_x64::R10, QByteArray("\x41\x52") },
    { Registers_x64::R11, QByteArray("\x41\x53") },
    { Registers_x64::R12, QByteArray("\x41\x54") },
    { Registers_x64::R13, QByteArray("\x41\x55") },
    { Registers_x64::R14, QByteArray("\x41\x56") },
    { Registers_x64::R15, QByteArray("\x41\x57") }
};

template <>
const QMap<Registers_x86, QByteArray> CodeDefines<Registers_x86>::_restore_reg =
{
    { Registers_x86::EAX, QByteArray("\x58") },
    { Registers_x86::EBX, QByteArray("\x5B") },
    { Registers_x86::ECX, QByteArray("\x59") },
    { Registers_x86::EDX, QByteArray("\x5A") },
    { Registers_x86::ESP, QByteArray("\x5C") },
    { Registers_x86::EBP, QByteArray("\x5D") },
    { Registers_x86::ESI, QByteArray("\x5E") },
    { Registers_x86::EDI, QByteArray("\x5F") }
};

template <>
const QMap<Registers_x64, QByteArray> CodeDefines<Registers_x64>::_restore_reg =
{
    { Registers_x64::RAX, QByteArray("\x58") },
    { Registers_x64::RBX, QByteArray("\x5B") },
    { Registers_x64::RCX, QByteArray("\x59") },
    { Registers_x64::RDX, QByteArray("\x5A") },
    { Registers_x64::RSP, QByteArray("\x5C") },
    { Registers_x64::RBP, QByteArray("\x5D") },
    { Registers_x64::RSI, QByteArray("\x5E") },
    { Registers_x64::RDI, QByteArray("\x5F") },
    { Registers_x64::R8, QByteArray("\x41\x58") },
    { Registers_x64::R9, QByteArray("\x41\x59") },
    { Registers_x64::R10, QByteArray("\x41\x5A") },
    { Registers_x64::R11, QByteArray("\x41\x5B") },
    { Registers_x64::R12, QByteArray("\x41\x5C") },
    { Registers_x64::R13, QByteArray("\x41\x5D") },
    { Registers_x64::R14, QByteArray("\x41\x5E") },
    { Registers_x64::R15, QByteArray("\x41\x5F") }
};

template <>
const QMap<Registers_x86, QByteArray> CodeDefines<Registers_x86>::_mov_to_reg =
{
    { Registers_x86::EAX, QByteArray("\xB8") },
    { Registers_x86::EBX, QByteArray("\xBB") },
    { Registers_x86::ECX, QByteArray("\xB9") },
    { Registers_x86::EDX, QByteArray("\xBA") },
    { Registers_x86::ESP, QByteArray("\xBC") },
    { Registers_x86::EBP, QByteArray("\xBD") },
    { Registers_x86::ESI, QByteArray("\xBE") },
    { Registers_x86::EDI, QByteArray("\xBF") }
};

template <>
const QMap<Registers_x64, QByteArray> CodeDefines<Registers_x64>::_mov_to_reg =
{
    { Registers_x64::RAX, QByteArray("\x48\xB8") },
    { Registers_x64::RBX, QByteArray("\x48\xBB") },
    { Registers_x64::RCX, QByteArray("\x48\xB9") },
    { Registers_x64::RDX, QByteArray("\x48\xBA") },
    { Registers_x64::RSP, QByteArray("\x48\xBC") },
    { Registers_x64::RBP, QByteArray("\x48\xBD") },
    { Registers_x64::RSI, QByteArray("\x48\xBE") },
    { Registers_x64::RDI, QByteArray("\x48\xBF") },
    { Registers_x64::R8, QByteArray("\x49\xB8") },
    { Registers_x64::R9, QByteArray("\x49\xB9") },
    { Registers_x64::R10, QByteArray("\x49\xBA") },
    { Registers_x64::R11, QByteArray("\x49\xBB") },
    { Registers_x64::R12, QByteArray("\x49\xBC") },
    { Registers_x64::R13, QByteArray("\x49\xBD") },
    { Registers_x64::R14, QByteArray("\x49\xBE") },
    { Registers_x64::R15, QByteArray("\x49\xBF") }
};

template <>
const QMap<Registers_x86, QByteArray> CodeDefines<Registers_x86>::_call_reg =
{
    { Registers_x86::EAX, QByteArray("\xFF\xD0") },
    { Registers_x86::EBX, QByteArray("\xFF\xD3") },
    { Registers_x86::ECX, QByteArray("\xFF\xD1") },
    { Registers_x86::EDX, QByteArray("\xFF\xD2") },
    { Registers_x86::ESP, QByteArray("\xFF\xD4") },
    { Registers_x86::EBP, QByteArray("\xFF\xD5") },
    { Registers_x86::ESI, QByteArray("\xFF\xD6") },
    { Registers_x86::EDI, QByteArray("\xFF\xD7") }
};

template <>
const QMap<Registers_x64, QByteArray> CodeDefines<Registers_x64>::_call_reg =
{
    { Registers_x64::RAX, QByteArray("\xFF\xD0") },
    { Registers_x64::RBX, QByteArray("\xFF\xD3") },
    { Registers_x64::RCX, QByteArray("\xFF\xD1") },
    { Registers_x64::RDX, QByteArray("\xFF\xD2") },
    { Registers_x64::RSP, QByteArray("\xFF\xD4") },
    { Registers_x64::RBP, QByteArray("\xFF\xD5") },
    { Registers_x64::RSI, QByteArray("\xFF\xD6") },
    { Registers_x64::RDI, QByteArray("\xFF\xD7") },
    { Registers_x64::R8, QByteArray("\x41\xFF\xD0") },
    { Registers_x64::R9, QByteArray("\x41\xFF\xD1") },
    { Registers_x64::R10, QByteArray("\x41\xFF\xD2") },
    { Registers_x64::R11, QByteArray("\x41\xFF\xD3") },
    { Registers_x64::R12, QByteArray("\x41\xFF\xD4") },
    { Registers_x64::R13, QByteArray("\x41\xFF\xD5") },
    { Registers_x64::R14, QByteArray("\x41\xFF\xD6") },
    { Registers_x64::R15, QByteArray("\x41\xFF\xD7") }
};

template <>
const QMap<Registers_x86, QByteArray> CodeDefines<Registers_x86>::_jmp_reg =
{
    { Registers_x86::EAX, QByteArray("\xFF\xE0") },
    { Registers_x86::EBX, QByteArray("\xFF\xE3") },
    { Registers_x86::ECX, QByteArray("\xFF\xE1") },
    { Registers_x86::EDX, QByteArray("\xFF\xE2") },
    { Registers_x86::ESP, QByteArray("\xFF\xE4") },
    { Registers_x86::EBP, QByteArray("\xFF\xE5") },
    { Registers_x86::ESI, QByteArray("\xFF\xE6") },
    { Registers_x86::EDI, QByteArray("\xFF\xE7") }
};

template <>
const QMap<Registers_x64, QByteArray> CodeDefines<Registers_x64>::_jmp_reg =
{
    { Registers_x64::RAX, QByteArray("\xFF\xE0") },
    { Registers_x64::RBX, QByteArray("\xFF\xE3") },
    { Registers_x64::RCX, QByteArray("\xFF\xE1") },
    { Registers_x64::RDX, QByteArray("\xFF\xE2") },
    { Registers_x64::RSP, QByteArray("\xFF\xE4") },
    { Registers_x64::RBP, QByteArray("\xFF\xE5") },
    { Registers_x64::RSI, QByteArray("\xFF\xE6") },
    { Registers_x64::RDI, QByteArray("\xFF\xE7") },
    { Registers_x64::R8, QByteArray("\x41\xFF\xE0") },
    { Registers_x64::R9, QByteArray("\x41\xFF\xE1") },
    { Registers_x64::R10, QByteArray("\x41\xFF\xE2") },
    { Registers_x64::R11, QByteArray("\x41\xFF\xE3") },
    { Registers_x64::R12, QByteArray("\x41\xFF\xE4") },
    { Registers_x64::R13, QByteArray("\x41\xFF\xE5") },
    { Registers_x64::R14, QByteArray("\x41\xFF\xE6") },
    { Registers_x64::R15, QByteArray("\x41\xFF\xE7") }
};

template <>
const QMap<Registers_x86, QByteArray> CodeDefines<Registers_x86>::_test_reg =
{
    { Registers_x86::EAX, QByteArray("\x85\xC0") },
    { Registers_x86::EBX, QByteArray("\x85\xDB") },
    { Registers_x86::ECX, QByteArray("\x85\xC9") },
    { Registers_x86::EDX, QByteArray("\x85\xD2") },
    { Registers_x86::ESP, QByteArray("\x85\xE4") },
    { Registers_x86::EBP, QByteArray("\x85\xED") },
    { Registers_x86::ESI, QByteArray("\x85\xF6") },
    { Registers_x86::EDI, QByteArray("\x85\xFF") }
};

template <>
const QMap<Registers_x64, QByteArray> CodeDefines<Registers_x64>::_test_reg =
{
    { Registers_x64::RAX, QByteArray("\x48\x85\xC0") },
    { Registers_x64::RBX, QByteArray("\x48\x85\xDB") },
    { Registers_x64::RCX, QByteArray("\x48\x85\xC9") },
    { Registers_x64::RDX, QByteArray("\x48\x85\xD2") },
    { Registers_x64::RSP, QByteArray("\x48\x85\xE4") },
    { Registers_x64::RBP, QByteArray("\x48\x85\xED") },
    { Registers_x64::RSI, QByteArray("\x48\x85\xF6") },
    { Registers_x64::RDI, QByteArray("\x48\x85\xFF") },
    { Registers_x64::R8, QByteArray("\x4D\x85\xC0") },
    { Registers_x64::R9, QByteArray("\x4D\x85\xC9") },
    { Registers_x64::R10, QByteArray("\x4D\x85\xD2") },
    { Registers_x64::R11, QByteArray("\x4D\x85\xDB") },
    { Registers_x64::R12, QByteArray("\x4D\x85\xE4") },
    { Registers_x64::R13, QByteArray("\x4D\x85\xED") },
    { Registers_x64::R14, QByteArray("\x4D\x85\xF6") },
    { Registers_x64::R15, QByteArray("\x4D\x85\xFF") }
};

template <>
const QMap<Registers_x86, QByteArray> CodeDefines<Registers_x86>::_esp_mem_to_reg =
{
    { Registers_x86::EAX, QByteArray("\x8B\x44\x24") },
    { Registers_x86::EBX, QByteArray("\x8B\x5C\x24") },
    { Registers_x86::ECX, QByteArray("\x8B\x4C\x24") },
    { Registers_x86::EDX, QByteArray("\x8B\x54\x24") },
    { Registers_x86::ESP, QByteArray("\x8B\x64\x24") },
    { Registers_x86::EBP, QByteArray("\x8B\x6C\x24") },
    { Registers_x86::ESI, QByteArray("\x8B\x74\x24") },
    { Registers_x86::EDI, QByteArray("\x8B\x7C\x24") }
};

template <>
const QMap<Registers_x64, QByteArray> CodeDefines<Registers_x64>::_esp_mem_to_reg =
{
    { Registers_x64::RAX, QByteArray("\x48\x8B\x44\x24") },
    { Registers_x64::RBX, QByteArray("\x48\x8B\x5C\x24") },
    { Registers_x64::RCX, QByteArray("\x48\x8B\x4C\x24") },
    { Registers_x64::RDX, QByteArray("\x48\x8B\x54\x24") },
    { Registers_x64::RSP, QByteArray("\x48\x8B\x64\x24") },
    { Registers_x64::RBP, QByteArray("\x48\x8B\x6C\x24") },
    { Registers_x64::RSI, QByteArray("\x48\x8B\x74\x24") },
    { Registers_x64::RDI, QByteArray("\x48\x8B\x7C\x24") },
    { Registers_x64::R8, QByteArray("\x4C\x8B\x44\x24") },
    { Registers_x64::R9, QByteArray("\x4C\x8B\x4C\x24") },
    { Registers_x64::R10, QByteArray("\x4C\x8B\x54\x24") },
    { Registers_x64::R11, QByteArray("\x4C\x8B\x5C\x24") },
    { Registers_x64::R12, QByteArray("\x4C\x8B\x64\x24") },
    { Registers_x64::R13, QByteArray("\x4C\x8B\x6C\x24") },
    { Registers_x64::R14, QByteArray("\x4C\x8B\x74\x24") },
    { Registers_x64::R15, QByteArray("\x4C\x8B\x7C\x24") }
};

template <>
const QMap<Registers_x86, QByteArray> CodeDefines<Registers_x86>::_reg_to_esp_mem =
{
    { Registers_x86::EAX, QByteArray("\x89\x44\x24") },
    { Registers_x86::EBX, QByteArray("\x89\x5C\x24") },
    { Registers_x86::ECX, QByteArray("\x89\x4C\x24") },
    { Registers_x86::EDX, QByteArray("\x89\x54\x24") },
    { Registers_x86::ESP, QByteArray("\x89\x64\x24") },
    { Registers_x86::EBP, QByteArray("\x89\x6C\x24") },
    { Registers_x86::ESI, QByteArray("\x89\x74\x24") },
    { Registers_x86::EDI, QByteArray("\x89\x7C\x24") }
};

template <>
const QMap<Registers_x64, QByteArray> CodeDefines<Registers_x64>::_reg_to_esp_mem =
{
    { Registers_x64::RAX, QByteArray("\x48\x89\x44\x24") },
    { Registers_x64::RBX, QByteArray("\x48\x89\x5C\x24") },
    { Registers_x64::RCX, QByteArray("\x48\x89\x4C\x24") },
    { Registers_x64::RDX, QByteArray("\x48\x89\x54\x24") },
    { Registers_x64::RSP, QByteArray("\x48\x89\x64\x24") },
    { Registers_x64::RBP, QByteArray("\x48\x89\x6C\x24") },
    { Registers_x64::RSI, QByteArray("\x48\x89\x74\x24") },
    { Registers_x64::RDI, QByteArray("\x48\x89\x7C\x24") },
    { Registers_x64::R8, QByteArray("\x4C\x89\x44\x24") },
    { Registers_x64::R9, QByteArray("\x4C\x89\x4C\x24") },
    { Registers_x64::R10, QByteArray("\x4C\x89\x54\x24") },
    { Registers_x64::R11, QByteArray("\x4C\x89\x5C\x24") },
    { Registers_x64::R12, QByteArray("\x4C\x89\x64\x24") },
    { Registers_x64::R13, QByteArray("\x4C\x89\x6C\x24") },
    { Registers_x64::R14, QByteArray("\x4C\x89\x74\x24") },
    { Registers_x64::R15, QByteArray("\x4C\x89\x7C\x24") }
};


template <typename Register>
QByteArray CodeDefines<Register>::saveRegister(Register reg)
{
    return _save_reg.contains(reg) ? _save_reg[reg] : QByteArray("");
}
template QByteArray CodeDefines<Registers_x86>::saveRegister(Registers_x86 reg);
template QByteArray CodeDefines<Registers_x64>::saveRegister(Registers_x64 reg);


template <typename Register>
QByteArray CodeDefines<Register>::restoreRegister(Register reg)
{
    return _restore_reg.contains(reg) ? _restore_reg[reg] : QByteArray("");
}
template QByteArray CodeDefines<Registers_x86>::restoreRegister(Registers_x86 reg);
template QByteArray CodeDefines<Registers_x64>::restoreRegister(Registers_x64 reg);


template <>
template <>
QByteArray CodeDefines<Registers_x86>::movValueToReg<uint32_t>(uint32_t value, Registers_x86 reg)
{
    QByteArray code;

    if(_mov_to_reg.contains(reg))
    {
        code.append(_mov_to_reg[reg]);
        code.append(reinterpret_cast<const char*>(&value), sizeof(uint32_t));
    }

    return code;
}


template <>
template <>
QByteArray CodeDefines<Registers_x64>::movValueToReg<uint64_t>(uint64_t value, Registers_x64 reg)
{
    QByteArray code;

    if(_mov_to_reg.contains(reg))
    {
        code.append(_mov_to_reg[reg]);
        code.append(reinterpret_cast<const char*>(&value), sizeof(uint64_t));
    }

    return code;
}


template <>
template <>
QByteArray CodeDefines<Registers_x86>::movValueToReg(uint64_t value, Registers_x86 reg)
{
    return CodeDefines<Registers_x86>::movValueToReg<uint32_t>(value, reg);
}


template <typename Register>
QByteArray CodeDefines<Register>::callReg(Register reg)
{
    return _call_reg.contains(reg) ? _call_reg[reg] : QByteArray("");
}
template QByteArray CodeDefines<Registers_x86>::callReg(Registers_x86 reg);
template QByteArray CodeDefines<Registers_x64>::callReg(Registers_x64 reg);

template <typename Register>
QByteArray CodeDefines<Register>::callRelative(uint32_t pos) {
    // TODO: some const value in class
    return QByteArray("\xe8", 1).append(reinterpret_cast<const char*>(&pos), sizeof(uint32_t));
}
template QByteArray CodeDefines<Registers_x86>::callRelative(uint32_t pos);
template QByteArray CodeDefines<Registers_x64>::callRelative(uint32_t pos);


template <typename Register>
QByteArray CodeDefines<Register>::jmpReg(Register reg)
{
    return _jmp_reg.contains(reg) ? _jmp_reg[reg] : QByteArray("");
}
template QByteArray CodeDefines<Registers_x86>::jmpReg(Registers_x86 reg);
template QByteArray CodeDefines<Registers_x64>::jmpReg(Registers_x64 reg);


template <typename Register>
QByteArray CodeDefines<Register>::testReg(Register reg)
{
    return _test_reg.contains(reg) ? _test_reg[reg] : QByteArray("");
}
template QByteArray CodeDefines<Registers_x86>::testReg(Registers_x86 reg);
template QByteArray CodeDefines<Registers_x64>::testReg(Registers_x64 reg);


template <typename Register>
QByteArray CodeDefines<Register>::jzRelative(int8_t pos)
{
    return QByteArray(_jz_rel).append(static_cast<char>(pos));
}
template QByteArray CodeDefines<Registers_x86>::jzRelative(int8_t pos);
template QByteArray CodeDefines<Registers_x64>::jzRelative(int8_t pos);


template <typename Register>
QByteArray CodeDefines<Register>::jmpRelative(int8_t pos)
{
    return QByteArray(_jmp_rel).append(static_cast<char>(pos));
}
template QByteArray CodeDefines<Registers_x86>::jmpRelative(int8_t pos);
template QByteArray CodeDefines<Registers_x64>::jmpRelative(int8_t pos);


template <typename Register>
QByteArray CodeDefines<Register>::saveAllInternal()
{
    QByteArray code;

    foreach (Register r, internalRegs)
        code.append(saveRegister(r));

    return code;
}
template QByteArray CodeDefines<Registers_x86>::saveAllInternal();
template QByteArray CodeDefines<Registers_x64>::saveAllInternal();


template <typename Register>
QByteArray CodeDefines<Register>::restoreAllInternal()
{
    QByteArray code;

    std::list<Register> revList = internalRegs.toStdList();
    revList.reverse();

    foreach (Register r, revList)
        code.append(restoreRegister(r));

    return code;
}
template QByteArray CodeDefines<Registers_x86>::restoreAllInternal();
template QByteArray CodeDefines<Registers_x64>::restoreAllInternal();


template <typename Register>
QByteArray CodeDefines<Register>::reserveStackSpace(uint16_t noParams)
{
    QByteArray code;
    int bytes = noParams * stackCellSize;

    while(bytes > 0)
    {
        uint8_t tmp_bytes = bytes > 64 ? 64 : bytes;
        bytes -= tmp_bytes;
        code.append(_reserve_stack);
        code.append(static_cast<char>(tmp_bytes));
    }

    return code;
}
template QByteArray CodeDefines<Registers_x86>::reserveStackSpace(uint16_t noParams);
template QByteArray CodeDefines<Registers_x64>::reserveStackSpace(uint16_t noParams);


template <typename Register>
QByteArray CodeDefines<Register>::clearStackSpace(uint16_t noParams)
{
    QByteArray code;
    int bytes = noParams * stackCellSize;

    while(bytes > 0)
    {
        uint8_t tmp_bytes = bytes > 64 ? 64 : bytes;
        bytes -= tmp_bytes;
        code.append(_clear_stack);
        code.append(static_cast<char>(tmp_bytes));
    }

    return code;
}
template QByteArray CodeDefines<Registers_x86>::clearStackSpace(uint16_t noParams);
template QByteArray CodeDefines<Registers_x64>::clearStackSpace(uint16_t noParams);


template <>
template <>
QByteArray CodeDefines<Registers_x86>::storeValue(uint32_t value)
{
    return QByteArray(_store_value).append(reinterpret_cast<const char*>(&value), sizeof(uint32_t));
}


template <>
template <>
QByteArray CodeDefines<Registers_x64>::storeValue(uint64_t value)
{
    return QByteArray(_store_value)
            .append(reinterpret_cast<const char*>(&value), sizeof(uint32_t))
            .append(_store_high_bytes)
            .append(reinterpret_cast<const char*>(&value) + 4, sizeof(uint32_t));
}


template <typename Register>
QByteArray CodeDefines<Register>::readFromEspMemToReg(Register reg, int8_t base)
{
    QByteArray code;

    if(_esp_mem_to_reg.contains(reg))
    {
        code.append(_esp_mem_to_reg[reg]);
        code.append(static_cast<char>(base));
    }

    return code;
}
template QByteArray CodeDefines<Registers_x86>::readFromEspMemToReg(Registers_x86 reg, int8_t base);
template QByteArray CodeDefines<Registers_x64>::readFromEspMemToReg(Registers_x64 reg, int8_t base);


template <typename Register>
QByteArray CodeDefines<Register>::readFromRegToEspMem(Register reg, int8_t base)
{
    QByteArray code;

    if(_reg_to_esp_mem.contains(reg))
    {
        code.append(_reg_to_esp_mem[reg]);
        code.append(static_cast<char>(base));
    }

    return code;
}
template QByteArray CodeDefines<Registers_x86>::readFromRegToEspMem(Registers_x86 reg, int8_t base);
template QByteArray CodeDefines<Registers_x64>::readFromRegToEspMem(Registers_x64 reg, int8_t base);


template <typename Register>
QByteArray CodeDefines<Register>::retN(uint16_t n)
{
    return QByteArray(_ret_n).append(reinterpret_cast<const char*>(&n), sizeof(uint16_t));
}
template QByteArray CodeDefines<Registers_x86>::retN(uint16_t n);
template QByteArray CodeDefines<Registers_x64>::retN(uint16_t n);


template <>
QByteArray CodeDefines<Registers_x86>::saveAll()
{
    return _pushad;
}

template <>
QByteArray CodeDefines<Registers_x64>::saveAll()
{
    typedef Registers_x64 Reg;

    QList<Reg> regs = {Reg::RAX, Reg::RCX, Reg::RDX, Reg::RBX, Reg::RSI, Reg::RDI,
                       Reg::R8, Reg::R9, Reg::R10, Reg::R11, Reg::R12, Reg::R13, Reg::R14, Reg::R15};

    seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::default_random_engine gen(seed);
    std::uniform_int_distribution<int> idx(0, 99);

    QByteArray code;

    while(!regs.empty())
    {
        int i = idx(gen) % regs.length();
        code.append(CodeDefines::saveRegister(regs[i]));
        regs.removeAt(i);
    }

    return code;
}

template <>
QByteArray CodeDefines<Registers_x86>::restoreAll()
{
    return _popad;
}

template <>
QByteArray CodeDefines<Registers_x64>::restoreAll()
{
    typedef Registers_x64 Reg;

    QList<Reg> regs = {Reg::RAX, Reg::RCX, Reg::RDX, Reg::RBX, Reg::RSI, Reg::RDI,
                       Reg::R8, Reg::R9, Reg::R10, Reg::R11, Reg::R12, Reg::R13, Reg::R14, Reg::R15};

    std::default_random_engine gen(seed);
    std::uniform_int_distribution<int> idx(0, 99);

    QByteArray code;

    while(!regs.empty())
    {
        int i = idx(gen) % regs.length();
        code.prepend(CodeDefines::restoreRegister(regs[i]));
        regs.removeAt(i);
    }

    return code;
}

template <typename Register>
QByteArray BinaryCode<Register>::getBytes()
{
    return code;
}
template QByteArray BinaryCode<Registers_x86>::getBytes();
template QByteArray BinaryCode<Registers_x64>::getBytes();


template <typename Register>
void BinaryCode<Register>::append(QByteArray _code, bool relocation)
{
    code.append(_code);

    if(relocation)
        relocations.append(code.length() - addrSize);
}
template void BinaryCode<Registers_x86>::append(QByteArray _code, bool relocation);
template void BinaryCode<Registers_x64>::append(QByteArray _code, bool relocation);


template <typename Register>
QList<uint64_t> BinaryCode<Register>::getRelocations(uint64_t codeBase)
{
    QList<uint64_t> rel;

    foreach(uint64_t val, relocations)
        rel.append(val + codeBase);

    return rel;
}
template QList<uint64_t> BinaryCode<Registers_x86>::getRelocations(uint64_t codeBase);
template QList<uint64_t> BinaryCode<Registers_x64>::getRelocations(uint64_t codeBase);


template <typename Register>
int BinaryCode<Register>::length()
{
    return code.length();
}
template int BinaryCode<Registers_x86>::length();
template int BinaryCode<Registers_x64>::length();


template <>
const uint8_t BinaryCode<Registers_x86>::addrSize = 4;


template <>
const uint8_t BinaryCode<Registers_x64>::addrSize = 8;
