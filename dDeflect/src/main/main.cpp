#include <QApplication>
#include <QQmlApplicationEngine>
#include <QtQml>
#include <QDebug>
#include <QDir>
#include <QMap>
#include <QProcess>
#include <QObject>

#include <DMethods/method.h>
#include <DMethods/methodlist.h>

#include <QJsonArray>
#include <QJsonDocument>
#include <QProcess>

#include "ApplicationManager/DJsonParser/djsonparser.h"
#include "core/adding_methods/wrappers/linux/daddingmethods.h"
#include "ApplicationManager/applicationmanager.h"
#include <ApplicationManager/DSourceCodeParser/dsourcecodeparser.h>

// Not clean, but fast
//QProcess *g_process = NULL;

// Needed as a signal catcher



int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

//    ProcOut *procOut = new ProcOut();
//    g_process        = new QProcess();

//    QObject::connect (g_process, SIGNAL(readyReadStandardOutput()),
//      procOut, SLOT(readyRead()));
//    QObject::connect (g_process, SIGNAL(finished (int, QProcess::ExitStatus)),
//      procOut, SLOT(finished()));

//    QStringList arguments;
//    arguments << "elffile.cpp" << "--";

//    g_process->start ("/home/jsk/static_analysis/llvm/Debug+Asserts/bin/functionFinder", arguments);
//    //g_process->start ("/home/jsk/main", arguments);
//    g_process->waitForFinished();
    DJsonParser d("./");

    DAddingMethods::InjectDescription<DAddingMethods::Registers_x86> id;
    id.name = "Kalinka";
    id.cm = DAddingMethods::CallingMethod::OEP;
    DAddingMethods::Wrapper<DAddingMethods::Registers_x86> w;
    w.code = "Kod la la la";
    w.ddetec_handler = NULL;
    QMap<QString,QString> m;
    m.insert("Param1","wart1");
    m.insert("Param2","wart2");
    w.params = m;
    w.ret = DAddingMethods::Registers_x86::EAX;
    QList<DAddingMethods::Registers_x86> l;
    l<<DAddingMethods::Registers_x86::EAX;
    l<<DAddingMethods::Registers_x86::EBP;
    w.used_regs = l;
    id.adding_method = &w;
    d.saveIncjectDescription("nazwa",id);

    //d.loadInjectDescription("nazwa2");
    d.loadInjectDescription("nazwa.json");
    QQmlApplicationEngine engine;
    ApplicationManager manager;

    engine.rootContext()->setContextProperty("applicationManager",&manager);

    qmlRegisterType<Method>("jsk.components", 1, 0, "Method");
    qmlRegisterType<MethodList>("jsk.components", 1, 0, "MethodList");


    engine.load(QUrl(QStringLiteral("qrc:///qml/main.qml")));



    return app.exec();
}
