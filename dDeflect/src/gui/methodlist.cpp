#include "methodlist.h"
#include <QQmlListProperty>
#include<QDebug>
MethodList::MethodList(QObject *parent) :
    QObject(parent)
{
}
MethodList::MethodList(QList<Method *> &m, QObject *parent) : m_methods(m), QObject(parent)
{
}
QQmlListProperty<Method> MethodList::methods()
{
    return QQmlListProperty<Method>(this,m_methods);
}

void MethodList::read(const QJsonObject &json)
{
    // TODO: free memory
    m_methods.clear();
    QJsonArray methodArray = json["methods"].toArray();
    for(int i = 0; i< methodArray.size(); ++i){
        QJsonObject mObject = methodArray[i].toObject();
        Method *m = new Method();
        m->read(mObject);
        m_methods.append(m);
    }
}

void MethodList::write(QJsonObject &json) const
{
    QJsonArray methodArray;
    foreach (const Method *m, m_methods) {
        QJsonObject mObject;
        m->write(mObject);
        methodArray.append(mObject);
    }
    json["methods"] = methodArray;
}

bool MethodList::loadList()
{
    QFile loadFile(QStringLiteral("methods.json"));

    if (!loadFile.open(QIODevice::ReadOnly)) {
        qWarning("Couldn't open save file.");
        return false;
    }

    QByteArray saveData = loadFile.readAll();

    QJsonDocument loadDoc(QJsonDocument::fromJson(saveData));

    read(loadDoc.object());

    return true;
}

bool MethodList::saveList()
{
    QFile saveFile(QStringLiteral("save.json"));

    if (!saveFile.open(QIODevice::WriteOnly)) {
        qWarning("Couldn't open save file.");
        return false;
    }

    QJsonObject gameObject;
    write(gameObject);
    QJsonDocument saveDoc(gameObject);
    saveFile.write(saveDoc.toJson());

    return true;
}

QDebug operator<<(QDebug d, const MethodList &ml)
{
   foreach(Method* mp, ml.m_methods) {

       d<<*mp;
   }
}

