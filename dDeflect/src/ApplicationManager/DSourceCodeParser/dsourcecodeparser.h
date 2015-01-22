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

#include <core/adding_methods/wrappers/daddingmethods.h>

using namespace std;

class DSourceCodeParser
{
public:
    template <typename Reg>
    using IDList = QList< Wrapper<Reg>*>;
    template <typename Reg>
    using FIDMapping = QHash<QString, IDList<Reg>>;

    DSourceCodeParser();
    QStringList getFunctions(const QString &path);
    void insertMethods(const QString &path, const QString& functionName, const QString& code);
private:
    bool read_and_parse(const QString &fname, QStringList &vals);
};

#endif // DSOURCECODEPARSER_H
