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

#define LOG_ERROR(msg) DLogger::write(DLogger::Type::Error, msg)
#define LOG_WARN(msg) DLogger::write(DLogger::Type::Warning, msg)
#define LOG_MSG(msg) DLogger::write(DLogger::Type::Message, msg)
#define LOG_DBG(msg) DLogger::write(DLogger::Type::Debug, msg)

#endif // DLOGGER_H
