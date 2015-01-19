#ifndef METHOD_H
#define METHOD_H

#include <QObject>
#include <QString>
#include <QJsonObject>
#include <QList>

#include "../core/adding_methods/wrappers/daddingmethods.h"

class Method : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)
    Q_PROPERTY(QString description READ description WRITE setDescription NOTIFY descriptionChanged)
    Q_PROPERTY(WrapperType wrapper_type READ wrapper_type NOTIFY wrapper_typeChanged)
    Q_PROPERTY(ArchitectureType arch_type READ arch_type NOTIFY arch_typeChanged)
    Q_PROPERTY(bool isThread READ isThread NOTIFY isThreadChanged)
    Q_PROPERTY(bool returns READ returns NOTIFY returnsChanged)
    Q_ENUMS(WrapperType)
    Q_ENUMS(ArchitectureType)
    Q_ENUMS(SystemType)


public:
    enum class WrapperType {
        Handler,
        Method,
        Helper,
        ThreadWrapper,
        OepWrapper,
        TrampolineWrapper
    };
    enum class ArchitectureType {
        BITS32,
        BITS64
    };
    enum class SystemType {
        Windows,
        Linux
    };


    explicit Method(QObject *parent = 0);
    Method(QString n, QString d, QObject * parent=0);
    Method(Wrapper<Registers_x86> * w, QObject * parent=0);
    Method(Wrapper<Registers_x64> * w, QObject * parent=0);
    void setName(QString n);
    void setDescription(const QString &d);

    QString name() const {return m_name;}
    QString description() const {return m_desc;}
    WrapperType wrapper_type() const {return m_wrapper_type; }
    ArchitectureType arch_type() const {return m_arch_type;}
    bool isThread() const {return m_isThread;}
    bool returns() const { return m_returns; }

    void read(const QJsonObject &json);
    void write(QJsonObject &json) const;

    friend QDebug operator<<(QDebug d, const Method& m);
signals:
    void nameChanged();
    void descriptionChanged();
    void wrapper_typeChanged();
    void arch_typeChanged();
    void isThreadChanged();
    void returnsChanged();
public:
    QString m_name;
    QString m_desc;
    WrapperType m_wrapper_type;
    SystemType m_system_type;
    ArchitectureType m_arch_type;
    bool m_isThread;
    bool m_returns;
};

#endif // METHOD_H
