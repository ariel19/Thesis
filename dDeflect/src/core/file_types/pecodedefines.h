#ifndef PECODEDEFINES_H
#define PECODEDEFINES_H

#include <QByteArray>
#include <core/file_types/pehelpers.h>

class PECodeDefines
{
private:
    static const QMap<Register, QByteArray> _save_reg;
    static const QMap<Register, QByteArray> _restore_reg;

public:
    static const QByteArray startFunc;
    static const QByteArray endFunc;
    static QByteArray saveRegister(Register reg);
    static QByteArray restoreRegister(Register reg);
};

#endif // PECODEDEFINES_H
