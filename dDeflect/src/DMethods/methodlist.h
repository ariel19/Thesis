#ifndef METHODLIST_H
#define METHODLIST_H
#include <../QtQml/qqmllist.h>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QList>
#include <QString>

#include <DMethods/method.h>

class MethodList : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QQmlListProperty<Method> methods READ methods NOTIFY methodsChanged)
    Q_PROPERTY(QVariantList names READ names NOTIFY namesChanged)
    Q_PROPERTY(QString path READ path WRITE setPath NOTIFY pathChanged)

public:
    explicit MethodList(QObject *parent = 0);
    MethodList(QList<Method*> &m, QString path = QString("methods.json"),QObject *parent=0);

    QQmlListProperty<Method> methods();
    QVariantList names();

    void read(const QJsonObject &json);
    void write(QJsonObject &json) const;

    bool loadList();
    bool saveList();

    QString path() const;
    void setPath(const QString &p);
public slots:

signals:
    void pathChanged();
    void namesChanged();
    void methodsChanged();

    friend QDebug operator<< (QDebug d, const MethodList &ml);
private:
    QList<Method*> m_methods;
    QString m_path;
    QVariantList m_names;
};

#endif // METHODLIST_H
