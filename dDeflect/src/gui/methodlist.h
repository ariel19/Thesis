#ifndef METHODLIST_H
#define METHODLIST_H
#include <QQmlListProperty>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QList>

#include "../gui/method.h"

class MethodList : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QQmlListProperty<Method> methods READ methods)
public:
    explicit MethodList(QObject *parent = 0);
    MethodList(QList<Method*> &m,QObject *parent=0);

    QQmlListProperty<Method> methods();

    void read(const QJsonObject &json);
    void write(QJsonObject &json) const;

    bool loadList();
    bool saveList();

    friend QDebug operator<< (QDebug d, const MethodList &ml);
private:
    QList<Method*> m_methods;
};

#endif // METHODLIST_H
