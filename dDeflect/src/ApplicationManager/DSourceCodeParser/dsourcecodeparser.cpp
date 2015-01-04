#include "dsourcecodeparser.h"


DSourceCodeParser::DSourceCodeParser()
{

}

QStringList DSourceCodeParser::getMethods(const QString &path)
{
    QString program = "/home/jsk/static_analysis/llvm/Debug+Asserts/bin/functionFinder";
    QFile functionFinder("/home/jsk/static_analysis/llvm/Debug+Asserts/bin/functionFinder");
    QStringList functionList;

    if(!functionFinder.exists()){
        qDebug() << "path to function finder is not correct";
        return functionList;
    }

    QStringList arguments;
    arguments << path << "--";

    QProcess * myProcess = new QProcess();
    myProcess->start(program, arguments);
    if(!myProcess->waitForFinished()){
        qDebug() << "timeout passed, functionFinder not executed.";
        return functionList;
    }

    fstream functionFile;
    string methodName;
    unsigned pos;
    functionFile.open("/home/jsk/functions.txt", std::ios_base::in);
    if(!functionFile.is_open()){
        qDebug()<<"Cant open /home/jsk/functions.txt";
        return functionList;
    }

    while(functionFile >> methodName >> pos)
        functionList.append(QString::fromStdString(methodName));

    return functionList;
}
