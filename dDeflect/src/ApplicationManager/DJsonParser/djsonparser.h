#ifndef DJSONPARSER_H
#define DJSONPARSER_H

#include <QJsonArray>
#include <QJsonDocument>

#include <core/adding_methods/wrappers/elfaddingmethods.h>
#include <core/adding_methods/wrappers/peaddingmethods.h>

class DJsonParser
{
    Wrapper<Registers_x86> *injectionRead;
    QString m_path;
public:
    DJsonParser();
    DJsonParser(QString path);

    template <typename Reg>
    Wrapper<Reg> *loadInjectDescription(QString name);

    bool saveIncjectDescription(QString name, Wrapper<Registers_x86> &inj);

    Wrapper<Registers_x86> *getInjectionRead() const;
    QString path() const;
    void setPath(const QString &path);
};

#endif // DJSONPARSER_H
