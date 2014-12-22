#include <QApplication>
#include <QQmlApplicationEngine>
#include <QtQml>
#include <QDebug>
#include <QDir>

#include <DMethods/method.h>
#include <DMethods/methodlist.h>

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    QQmlApplicationEngine engine;

    qmlRegisterType<Method>("jsk.components", 1, 0, "Method");
    qmlRegisterType<MethodList>("jsk.components", 1, 0, "MethodList");


    engine.load(QUrl(QStringLiteral("qrc:///qml/main.qml")));

    return app.exec();
}
