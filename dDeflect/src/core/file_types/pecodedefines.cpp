#include "pecodedefines.h"

const QByteArray PECodeDefines::startFunc = QByteArray("\x55\x89\xE5");
const QByteArray PECodeDefines::endFunc = QByteArray("\x5D");

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


QByteArray PECodeDefines::saveRegister(Register reg)
{
    return _save_reg.contains(reg) ? _save_reg[reg] : QByteArray("");
}

QByteArray PECodeDefines::restoreRegister(Register reg)
{
    return _restore_reg.contains(reg) ? _restore_reg[reg] : QByteArray("");
}
