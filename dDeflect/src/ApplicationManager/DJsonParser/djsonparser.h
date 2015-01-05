#ifndef DJSONPARSER_H
#define DJSONPARSER_H

#include <QJsonArray>
#include <QJsonDocument>

#include "../../core/adding_methods/wrappers/linux/daddingmethods.h"
class DJsonParser
{
    DAddingMethods::InjectDescription<DAddingMethods::Registers_x86> *injectionRead;
    QString m_path;
public:
    DJsonParser(QString path);
    DAddingMethods::InjectDescription<DAddingMethods::Registers_x86> * loadInjectDescription(QString name);
    bool saveIncjectDescription(QString name, DAddingMethods::InjectDescription<DAddingMethods::Registers_x86> &inj);

    DAddingMethods::InjectDescription<DAddingMethods::Registers_x86> *getInjectionRead() const;
};

#endif // DJSONPARSER_H
