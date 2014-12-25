#ifndef DJSONPARSER_H
#define DJSONPARSER_H

#include <QJsonArray>
#include <QJsonDocument>

#include "../core/adding_methods/wrappers/linux/daddingmethods.h"
class DJsonParser
{
    DAddingMethods::InjectDescription<DAddingMethods::Registers_x86> *injectionRead;
    QString m_path;
public:
    DJsonParser();
    bool loadInjectDescription();
    bool saveIncjectionDescription(DAddingMethods::InjectDescription<DAddingMethods::Registers_x86> &inj);
};

#endif // DJSONPARSER_H
