#include "pecodedefines.h"

const QByteArray PECodeDefines::startFunc = QByteArray("\x55\x89\xE5");
const QByteArray PECodeDefines::endFunc = QByteArray("\x5D");
const QByteArray PECodeDefines::_jz_rel = QByteArray("\x74");
const QByteArray PECodeDefines::_reserve_stack = QByteArray("\x83\xEC");
const QByteArray PECodeDefines::_clear_stack = QByteArray("\x83\xC4");
const QByteArray PECodeDefines::_store_dword = QByteArray("\x68");

const QList<Register> PECodeDefines::internalRegs =
{
    Register::EAX,
    Register::ECX,
    Register::EDX
};

const QList<Register> PECodeDefines::externalRegs =
{
    Register::EBX,
    Register::ESP,
    Register::EBP,
    Register::ESI,
    Register::EDI
};

const QMap<Register, QByteArray> PECodeDefines::_save_reg =
{
    { Register::EAX, QByteArray("\x50") },
    { Register::EBX, QByteArray("\x53") },
    { Register::ECX, QByteArray("\x51") },
    { Register::EDX, QByteArray("\x52") },
    { Register::ESP, QByteArray("\x54") },
    { Register::EBP, QByteArray("\x55") },
    { Register::ESI, QByteArray("\x56") },
    { Register::EDI, QByteArray("\x57") }
};

const QMap<Register, QByteArray> PECodeDefines::_restore_reg =
{
    { Register::EAX, QByteArray("\x58") },
    { Register::EBX, QByteArray("\x5B") },
    { Register::ECX, QByteArray("\x59") },
    { Register::EDX, QByteArray("\x5A") },
    { Register::ESP, QByteArray("\x5C") },
    { Register::EBP, QByteArray("\x5D") },
    { Register::ESI, QByteArray("\x5E") },
    { Register::EDI, QByteArray("\x5F") }
};

const QMap<Register, QByteArray> PECodeDefines::_mov_to_reg =
{
    { Register::EAX, QByteArray("\xB8") },
    { Register::EBX, QByteArray("\xBB") },
    { Register::ECX, QByteArray("\xB9") },
    { Register::EDX, QByteArray("\xBA") },
    { Register::ESP, QByteArray("\xBC") },
    { Register::EBP, QByteArray("\xBD") },
    { Register::ESI, QByteArray("\xBE") },
    { Register::EDI, QByteArray("\xBF") }
};

const QMap<Register, QByteArray> PECodeDefines::_call_reg =
{
    { Register::EAX, QByteArray("\xFF\xD0") },
    { Register::EBX, QByteArray("\xFF\xD3") },
    { Register::ECX, QByteArray("\xFF\xD1") },
    { Register::EDX, QByteArray("\xFF\xD2") },
    { Register::ESP, QByteArray("\xFF\xD4") },
    { Register::EBP, QByteArray("\xFF\xD5") },
    { Register::ESI, QByteArray("\xFF\xD6") },
    { Register::EDI, QByteArray("\xFF\xD7") }
};

const QMap<Register, QByteArray> PECodeDefines::_jmp_reg =
{
    { Register::EAX, QByteArray("\xFF\xE0") },
    { Register::EBX, QByteArray("\xFF\xE3") },
    { Register::ECX, QByteArray("\xFF\xE1") },
    { Register::EDX, QByteArray("\xFF\xE2") },
    { Register::ESP, QByteArray("\xFF\xE4") },
    { Register::EBP, QByteArray("\xFF\xE5") },
    { Register::ESI, QByteArray("\xFF\xE6") },
    { Register::EDI, QByteArray("\xFF\xE7") }
};

const QMap<Register, QByteArray> PECodeDefines::_test_reg =
{
    { Register::EAX, QByteArray("\x85\xC0") },
    { Register::EBX, QByteArray("\x85\xDB") },
    { Register::ECX, QByteArray("\x85\xC9") },
    { Register::EDX, QByteArray("\x85\xD2") },
    { Register::ESP, QByteArray("\x85\xE4") },
    { Register::EBP, QByteArray("\x85\xED") },
    { Register::ESI, QByteArray("\x85\xF6") },
    { Register::EDI, QByteArray("\x85\xFF") }
};

const QMap<Register, QByteArray> PECodeDefines::_esp_mem_to_reg =
{
    { Register::EAX, QByteArray("\x8B\x44\x24") },
    { Register::EBX, QByteArray("\x8B\x5C\x24") },
    { Register::ECX, QByteArray("\x8B\x4C\x24") },
    { Register::EDX, QByteArray("\x8B\x54\x24") },
    { Register::ESP, QByteArray("\x8B\x64\x24") },
    { Register::EBP, QByteArray("\x8B\x6C\x24") },
    { Register::ESI, QByteArray("\x8B\x74\x24") },
    { Register::EDI, QByteArray("\x8B\x7C\x24") }
};

const QMap<Register, QByteArray> PECodeDefines::_reg_to_esp_mem =
{
    { Register::EAX, QByteArray("\x89\x44\x24") },
    { Register::EBX, QByteArray("\x89\x5C\x24") },
    { Register::ECX, QByteArray("\x89\x4C\x24") },
    { Register::EDX, QByteArray("\x89\x54\x24") },
    { Register::ESP, QByteArray("\x89\x64\x24") },
    { Register::EBP, QByteArray("\x89\x6C\x24") },
    { Register::ESI, QByteArray("\x89\x74\x24") },
    { Register::EDI, QByteArray("\x89\x7C\x24") }
};


QByteArray PECodeDefines::saveRegister(Register reg)
{
    return _save_reg.contains(reg) ? _save_reg[reg] : QByteArray("");
}

QByteArray PECodeDefines::restoreRegister(Register reg)
{
    return _restore_reg.contains(reg) ? _restore_reg[reg] : QByteArray("");
}

QByteArray PECodeDefines::movDWordToReg(uint32_t dword, Register reg)
{
    QByteArray code;

    if(_mov_to_reg.contains(reg))
    {
        code.append(_mov_to_reg[reg]);
        code.append(reinterpret_cast<const char*>(&dword), sizeof(uint32_t));
    }

    return code;
}

QByteArray PECodeDefines::callReg(Register reg)
{
    return _call_reg.contains(reg) ? _call_reg[reg] : QByteArray("");
}

QByteArray PECodeDefines::jmpReg(Register reg)
{
    return _jmp_reg.contains(reg) ? _jmp_reg[reg] : QByteArray("");
}

QByteArray PECodeDefines::testReg(Register reg)
{
    return _test_reg.contains(reg) ? _test_reg[reg] : QByteArray("");
}

QByteArray PECodeDefines::jzRelative(int8_t pos)
{
    return QByteArray(_jz_rel).append(static_cast<char>(pos));
}

QByteArray PECodeDefines::saveAllInternal()
{
    QByteArray code;

    foreach (Register r, internalRegs)
        code.append(saveRegister(r));

    return code;
}

QByteArray PECodeDefines::restoreAllInternal()
{
    QByteArray code;

    foreach (Register r, internalRegs)
        code.append(restoreRegister(r));

    return code;
}

QByteArray PECodeDefines::reserveStackSpace(uint16_t noParams)
{
    QByteArray code;
    int bytes = noParams * 4;

    while(bytes > 0)
    {
        uint8_t tmp_bytes = bytes > 64 ? 64 : bytes;
        bytes -= tmp_bytes;
        code.append(_reserve_stack);
        code.append(static_cast<char>(tmp_bytes));
    }

    return code;
}

QByteArray PECodeDefines::clearStackSpace(uint16_t noParams)
{
    QByteArray code;
    int bytes = noParams * 4;

    while(bytes > 0)
    {
        uint8_t tmp_bytes = bytes > 64 ? 64 : bytes;
        bytes -= tmp_bytes;
        code.append(_clear_stack);
        code.append(static_cast<char>(tmp_bytes));
    }

    return code;
}

QByteArray PECodeDefines::storeDWord(uint32_t dword)
{
    return QByteArray(_store_dword).append(reinterpret_cast<const char*>(&dword), sizeof(uint32_t));
}

QByteArray PECodeDefines::readFromEspMemToReg(Register reg, int8_t base)
{
    QByteArray code;

    if(_esp_mem_to_reg.contains(reg))
    {
        code.append(_esp_mem_to_reg[reg]);
        code.append(static_cast<char>(base));
    }

    return code;
}

QByteArray PECodeDefines::readFromRegToEspMem(Register reg, int8_t base)
{
    QByteArray code;

    if(_reg_to_esp_mem.contains(reg))
    {
        code.append(_reg_to_esp_mem[reg]);
        code.append(static_cast<char>(base));
    }

    return code;
}
