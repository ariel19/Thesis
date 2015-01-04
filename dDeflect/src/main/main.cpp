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

    QQmlApplicationEngine engine;
    ApplicationManager manager;

    engine.rootContext()->setContextProperty("applicationManager",&manager);

    qmlRegisterType<Method>("jsk.components", 1, 0, "Method");
    qmlRegisterType<MethodList>("jsk.components", 1, 0, "MethodList");


    engine.load(QUrl(QStringLiteral("qrc:///qml/main.qml")));



    return app.exec();
}
