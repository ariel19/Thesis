#include <QApplication>
#include <QQmlApplicationEngine>
#include <QtQml>
#include <QDebug>
#include <QDir>

#include "/home/jsk/code/Thesis/dDeflect/src/gui/method.h"
#include "/home/jsk/code/Thesis/dDeflect/src/gui/methodlist.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    QQmlApplicationEngine engine;
    engine.load(QUrl(QStringLiteral("qrc:///qml/main.qml")));
    qmlRegisterType<Method>("com.jsk.mcomponents", 1, 0, "Method");
    qmlRegisterType<MethodList>("com.jsk.mcomponents", 1, 0, "MethodList");


    return app.exec();
}
