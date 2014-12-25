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

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    QQmlApplicationEngine engine;

    qmlRegisterType<Method>("jsk.components", 1, 0, "Method");
    qmlRegisterType<MethodList>("jsk.components", 1, 0, "MethodList");

    DAddingMethods::InjectDescription<DAddingMethods::Registers_x86> w;
    w.cm = DAddingMethods::CallingMethod::Thread;

    DAddingMethods::Wrapper<DAddingMethods::Registers_x86> wrapper;
    wrapper.code = QString("Code");
    QMap<QString,QString> m;
    m.insert("Arg1","RAX");
    m.insert("arg2","EBX");
    wrapper.params = m;
    wrapper.used_regs.append(DAddingMethods::Registers_x86::EAX);
    wrapper.used_regs.append(DAddingMethods::Registers_x86::EBX);
    wrapper.ret = DAddingMethods::Registers_x86::EDI;
    wrapper.ddetec_handler = nullptr;

    w.adding_method = &wrapper;
    DJsonParser parser;
    parser.saveIncjectionDescription(w);
    parser.loadInjectDescription();
    engine.load(QUrl(QStringLiteral("qrc:///qml/main.qml")));

    return app.exec();
}
