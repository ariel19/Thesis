#include <DMethods/method.h>
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
Method::Method(Wrapper<Registers_x86> *w, QObject * parent): QObject(parent)
{
   m_name = w->name;
   m_arch_type = static_cast<ArchitectureType>(w->arch_type);
   m_desc = w->description;
   m_system_type = static_cast<SystemType>(w->system_type);
   m_returns = (w->ret != Registers_x86::None);
   m_isThread = (w->wrapper_type==Wrapper<Registers_x86>::WrapperType::ThreadWrapper);
}
Method::Method(Wrapper<Registers_x64> *w, QObject * parent): QObject(parent)
{
   m_name = w->name;
   m_arch_type = static_cast<ArchitectureType>(w->arch_type);
   m_desc = w->description;
   m_system_type = static_cast<SystemType>(w->system_type);
   m_returns = (w->ret != Registers_x64::None);
   m_isThread = (w->wrapper_type==Wrapper<Registers_x64>::WrapperType::ThreadWrapper);
}

void Method::setName( QString n){
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
    return d<<"Name: "<<m.m_name<<" Description: "<<m.m_desc;
}


