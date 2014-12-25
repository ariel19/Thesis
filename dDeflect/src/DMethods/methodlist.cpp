#include <DMethods/methodlist.h>
#include <QQmlListProperty>
#include <QDebug>

MethodList::MethodList(QObject *parent) :
    QObject(parent)
{
}
MethodList::MethodList(QList<Method *> &m, QString path, QObject *parent) : m_methods(m), QObject(parent), m_path(path)
{
}
QQmlListProperty<Method> MethodList::methods()
{
    return QQmlListProperty<Method>(this,m_methods);
}

QVariantList MethodList::names()
{
    loadList();
    // TODO: free Memory
    foreach(Method* mp, m_methods){
        QVariant* p = new QVariant(QString(mp->name()));
        m_names.append(*p);
        qDebug()<<mp->name();
    }

    return m_names;
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
    QFile loadFile(m_path);

    if (!loadFile.open(QIODevice::ReadOnly)) {
        qWarning("Couldn't open save file.");
        return false;
    }

    QByteArray saveData = loadFile.readAll();

    QJsonDocument loadDoc(QJsonDocument::fromJson(saveData));

    read(loadDoc.object());

    return true;
}

bool MethodList::   saveList()
{
    QFile saveFile(m_path);

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

QString MethodList::path() const {return m_path;}

void MethodList::setPath(const QString &p){
    if(p!=m_path){
        m_path = p;
        emit pathChanged();
    }
}

QDebug operator<<(QDebug d, const MethodList &ml)
{
    foreach(Method* mp, ml.m_methods) {

        d<<*mp;
   }
    return d;
}

