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
#include <core/adding_methods/wrappers/elfaddingmethods.h>
#include <core/adding_methods/wrappers/peaddingmethods.h>
#include "ApplicationManager/applicationmanager.h"
#include <ApplicationManager/DSourceCodeParser/dsourcecodeparser.h>
#include "../DMethods/method.h"

// Not clean, but fast
//QProcess *g_process = NULL;

// Needed as a signal catcher



int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    QQmlApplicationEngine engine;
    ApplicationManager manager;

    engine.rootContext()->setContextProperty("applicationManager",&manager);

    qmlRegisterType<Method>("jsk.components", 1, 0, "Method");
    qmlRegisterType<MethodList>("jsk.components", 1, 0, "MethodList");
    qmlRegisterType<ApplicationManager>("jsk.components", 1, 0, "Manager");
    qmlRegisterType<SourceCodeDescription>("jsk.components", 1, 0, "SourceWrapper");

    engine.load(QUrl(QStringLiteral("qrc:///qml/main.qml")));



    return app.exec();
}
