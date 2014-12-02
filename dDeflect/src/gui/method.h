#ifndef METHOD_H
#define METHOD_H

#include <QObject>
#include <QString>
#include <QJsonObject>
#include <QList>

class Method : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)
    Q_PROPERTY(QString description READ description WRITE setDescription NOTIFY descriptionChanged)
public:
    explicit Method(QObject *parent = 0);
    Method(QString n, QString d, QObject * parent=0);

    void setName(const QString &n);
    void setDescription(const QString &d);

    QString name() const {return m_name;}
    QString description() const {return m_desc;}

    void read(const QJsonObject &json);
    void write(QJsonObject &json) const;

    friend QDebug operator<<(QDebug d, const Method& m);
signals:
    void nameChanged();
    void descriptionChanged();
private:
    QString m_name;
    QString m_desc;

};

#endif // METHOD_H
