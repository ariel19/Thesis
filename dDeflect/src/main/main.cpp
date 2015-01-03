#include <QApplication>
#include <QQmlApplicationEngine>
#include <QtQml>
#include <QDebug>
#include <QDir>
#include <QMap>

#include <DMethods/method.h>
#include <DMethods/methodlist.h>

#include <QJsonArray>
#include <QJsonDocument>

#include "DJsonParser/djsonparser.h"
#include "./core/adding_methods/wrappers/linux/daddingmethods.h"
#include "ApplicationManager/applicationmanager.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    QQmlApplicationEngine engine;
    ApplicationManager manager;

    engine.rootContext()->setContextProperty("applicationManager",&manager);

    qmlRegisterType<Method>("jsk.components", 1, 0, "Method");
    qmlRegisterType<MethodList>("jsk.components", 1, 0, "MethodList");


    engine.load(QUrl(QStringLiteral("qrc:///qml/main.qml")));

    return app.exec();
}
