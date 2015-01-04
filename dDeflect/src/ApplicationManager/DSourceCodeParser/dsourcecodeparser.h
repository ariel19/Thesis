#ifndef DSOURCECODEPARSER_H
#define DSOURCECODEPARSER_H

#include <iostream>
#include <string>
#include <QStringList>
#include <QProcess>
#include <QFile>
#include <QDebug>
#include <QTextStream>
#include <fstream>

#include <core/adding_methods/wrappers/linux/daddingmethods.h>

using namespace std;

class DSourceCodeParser
{
public:
    template <typename Reg>
    using IDList = QList<DAddingMethods::InjectDescription<Reg>*>;
    template <typename Reg>
    using FIDMapping = QHash<QString, IDList<Reg>>;

    DSourceCodeParser();
    QStringList getFunctions(const QString &path);
    void insertMethods(const QString &path, FIDMapping<DAddingMethods::Registers_x86>);
private:

};

#endif // DSOURCECODEPARSER_H
