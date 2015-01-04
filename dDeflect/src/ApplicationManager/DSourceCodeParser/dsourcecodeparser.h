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

using namespace std;

class DSourceCodeParser
{
public:
    //static QProcess * g_process;
    DSourceCodeParser();
    QStringList getMethods(const QString &path);
private:

};

#endif // DSOURCECODEPARSER_H
