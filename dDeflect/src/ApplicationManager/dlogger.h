#ifndef DLOGGER_H
#define DLOGGER_H

#include <QString>
#include <QList>
#include <QMap>
#include <functional>

class DLogger
{
public:
    enum class Type
    {
        Error,
        Warning,
        Message,
        Debug
    };

    static void write(Type type, QString msg);
    static void registerCallback(QList<Type> types, std::function<void(QString)> f);

private:
    DLogger();
    static DLogger &getLogger();

    QMap<Type, QList<std::function<void(QString)>>> callbacks;
};

#endif // DLOGGER_H
