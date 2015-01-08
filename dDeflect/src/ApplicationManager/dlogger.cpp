#include "dlogger.h"

DLogger::DLogger()
{
}

DLogger &DLogger::getLogger()
{
    static DLogger l;
    return l;
}

void DLogger::write(DLogger::Type type, QString msg)
{
    DLogger log = getLogger();

    if(log.callbacks.contains(type))
    {
        foreach(auto cbk, log.callbacks[type])
            cbk(msg);
    }
}

void DLogger::registerCallback(QList<DLogger::Type> types, std::function<void (QString)> f)
{
    DLogger log = getLogger();

    foreach(auto t, types)
        log.callbacks[t].append(f);
}
