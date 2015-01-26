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

#include <ApplicationManager/dlogger.h>
#include <QString>

#include <QJsonArray>
#include <QJsonDocument>
#include <QProcess>

#include <ApplicationManager/DJsonParser/djsonparser.h>
#include <core/adding_methods/wrappers/elfaddingmethods.h>
#include <core/adding_methods/wrappers/peaddingmethods.h>
#include <ApplicationManager/applicationmanager.h>
#include <ApplicationManager/DSourceCodeParser/dsourcecodeparser.h>

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

    DLogger::registerCallback({DLogger::Type::Error, DLogger::Type::Warning, DLogger::Type::Message},
                              [](QString msg)-> void { printf("%s\n", msg.toStdString().c_str()); fflush(0); });

    return app.exec();
}
