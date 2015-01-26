#include "dsourcecodeparser.h"
#include <ApplicationManager/dsettings.h>
#include <ApplicationManager/dlogger.h>

DSourceCodeParser::DSourceCodeParser() {}

QStringList DSourceCodeParser::getFunctions(const QString &path) {

    static QStringList emptyList;
    QString program(DSettings::getSettings().getFunctionFinder());

    // FIXME: race condition!!!!!!!!!!!!!
    if(!QFile(program).exists()) {
        LOG_ERROR(QString("path to function finder: %1 is not correct").arg(program));
        return emptyList;
    }

    QStringList functionList;
    QStringList arguments;
    QString path2 = path;
    path2 = path2.remove("file://");
    arguments << path2 << "--";

    QProcess *myProcess = new(std::nothrow) QProcess();
    if (!myProcess)
        return emptyList;

    myProcess->start(program, arguments);
    if(!myProcess->waitForFinished()) {
        LOG_ERROR("timeout passed, functionFinder not executed.");
        delete myProcess;

        return functionList;
    }

    read_and_parse(DSettings::getSettings().getFunctionsPath(), functionList);

    //qDebug()<<functionList;

    delete myProcess;

    return functionList;
}

void DSourceCodeParser::insertMethods(const QString &path, const QString& functionName,
                                      const QString& code) {

    QString program(DSettings::getSettings().getMethodsInserter());

    // FIXME: race condition!!!!!!!!!!!!!
    if(!QFile(program).exists()) {
        LOG_ERROR(QString("path to methods inserter: %1 is not correct").arg(program));
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

    QProcess *myProcess = new(std::nothrow) QProcess();
    if (!myProcess)
        return;

    myProcess->start(program, arguments);
    if(!myProcess->waitForFinished()) {
        LOG_ERROR("timeout passed, methodInsert not executed.");
        delete myProcess;

        return;
    }

    delete myProcess;
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
