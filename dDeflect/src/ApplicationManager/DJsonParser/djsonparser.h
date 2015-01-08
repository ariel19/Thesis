#ifndef DJSONPARSER_H
#define DJSONPARSER_H

#include <QJsonArray>
#include <QJsonDocument>

#include <core/adding_methods/wrappers/elfaddingmethods.h>
#include <core/adding_methods/wrappers/peaddingmethods.h>

class DJsonParser
{
    DAddingMethods<Registers_x86>::Wrapper *injectionRead;
    QString m_path;
public:
    DJsonParser(QString path);

    template <typename Reg>
    typename DAddingMethods<Reg>::Wrapper *loadInjectDescription(QString name);

    bool saveIncjectDescription(QString name, DAddingMethods<Registers_x86>::Wrapper &inj);

    DAddingMethods<Registers_x86>::Wrapper *getInjectionRead() const;
};

#endif // DJSONPARSER_H
