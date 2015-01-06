#ifndef DJSONPARSER_H
#define DJSONPARSER_H

#include <QJsonArray>
#include <QJsonDocument>

#include <core/adding_methods/wrappers/elfaddingmethods.h>
#include <core/adding_methods/wrappers/peaddingmethods.h>

class DJsonParser
{
    DAddingMethods::Wrapper<Registers_x86> *injectionRead;
    QString m_path;
public:
    DJsonParser(QString path);

    template <typename Reg>
    DAddingMethods::Wrapper<Reg> *loadInjectDescription(QString name);

    bool saveIncjectDescription(QString name, DAddingMethods::Wrapper<Registers_x86> &inj);

    DAddingMethods::Wrapper<Registers_x86> *getInjectionRead() const;
};

#endif // DJSONPARSER_H
