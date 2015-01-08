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

void DSourceCodeParser::insertMethods(const QString &path, FIDMapping<DAddingMethods::Registers_x86> map)
{
    QString program = "/home/jsk/static_analysis/llvm/Debug+Asserts/bin/methodInsert";
    QFile methodInsert("/home/jsk/static_analysis/llvm/Debug+Asserts/bin/methodInsert");

    if(!methodInsert.exists()){
        qDebug() << "path to methodInsert is not correct";
        return;
    }

    QString correctPath = path;
    correctPath = correctPath.remove("file://");
    QStringList arguments;

    // TODO: przygotuj kod do wstawienia tak aby działał
    QString codeToInsert;
    foreach(DAddingMethods::InjectDescription<DAddingMethods::Registers_x86>* id, map["main"]){
        codeToInsert.append("__asm__(");
        codeToInsert.append(id->adding_method->code);
        codeToInsert.append(");\n");
    }
    arguments << correctPath << "--" << "main" << codeToInsert;

    QProcess * myProcess = new QProcess();
    myProcess->start(program, arguments);
    if(!myProcess->waitForFinished()){
        qDebug() << "timeout passed, methodInsert not executed.";
        return;
    }
}
