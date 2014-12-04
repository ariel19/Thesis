#include <gui/method.h>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QDebug>


Method::Method(QObject *parent) :
    QObject(parent)
{
}

Method::Method(QString n, QString d, QObject *parent) : m_name(n), m_desc(d), QObject(parent)
{
}

void Method::setName(const QString &n){
    if(n!=m_name){
        m_name = n;
        emit nameChanged();
    }
}

void Method::setDescription(const QString &d){
    if(d!=m_desc){
        m_desc = d;
        emit descriptionChanged();
    }
}

void Method::read(const QJsonObject &json)
{
    m_name = json["name"].toString();
    m_desc = json["description"].toString();
}

void Method::write(QJsonObject &json) const
{
    json["name"] = m_name;
    json["description"] = m_desc;
}

QDebug operator<<(QDebug d, const Method &m)
{
    d<<"Name: "<<m.m_name<<" Description: "<<m.m_desc;
}


