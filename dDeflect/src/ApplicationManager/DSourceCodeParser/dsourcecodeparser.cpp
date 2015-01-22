#include "dsourcecodeparser.h"


DSourceCodeParser::DSourceCodeParser()
{

}

QStringList DSourceCodeParser::getFunctions(const QString &path)
{
    QString program = "/home/jsk/static_analysis/llvm/Debug+Asserts/bin/functionFinder";
    QFile functionFinder("/home/jsk/static_analysis/llvm/Debug+Asserts/bin/functionFinder");
    QStringList functionList;

    if(!functionFinder.exists()){
        qDebug() << "path to function finder is not correct";
        return functionList;
    }

    QStringList arguments;
    QString path2 = path;
    path2 = path2.remove("file://");
    arguments << path2 << "--";

    QProcess * myProcess = new QProcess();
    myProcess->start(program, arguments);
    if(!myProcess->waitForFinished()){
        qDebug() << "timeout passed, functionFinder not executed.";
        return functionList;
    }

    read_and_parse("/home/jsk/functions.txt", functionList);
    //qDebug()<<functionList;
    return functionList;
}

void DSourceCodeParser::insertMethods(const QString &path, const QString& functionName, const QString& code)
{
    QString program = "/home/jsk/static_analysis/llvm/Debug+Asserts/bin/methodInsert";
    QFile methodInsert("/home/jsk/static_analysis/llvm/Debug+Asserts/bin/methodInsert");

    if(!methodInsert.exists()){
        qDebug() << "path to methodInsert is not correct";
        return;
    }

    QStringList arguments;
    QString codeToInsert;

    codeToInsert.append("{");
    codeToInsert.append(code);
    codeToInsert.append("}");

    QString fn = functionName;
    fn = fn.remove(" ");

    arguments << path << "--" << fn << codeToInsert;

    QProcess * myProcess = new QProcess();
    myProcess->start(program, arguments);
    if(!myProcess->waitForFinished()){
        qDebug() << "timeout passed, methodInsert not executed.";
        return;
    }
}
bool DSourceCodeParser::read_and_parse(const QString &fname, QStringList &vals) {
    QFile f(fname);
    if (!f.open(QIODevice::ReadOnly))
        return false;

    QString content = f.readAll();
    QStringList lines = content.split('\n');

    // filter lines
    QString tline;
    int idx;
    foreach (QString line, lines) {
        tline = line.remove(QRegExp("(\\n\\t\\r)"));
        idx = tline.indexOf('(');
        if (idx != -1)
            tline.remove(0, idx + 1);
        idx = tline.lastIndexOf(')');
        if (idx != -1)
            tline.remove(idx, 1);
        if (!tline.isEmpty())
            vals.push_back(tline);
    }

    return true;
}
