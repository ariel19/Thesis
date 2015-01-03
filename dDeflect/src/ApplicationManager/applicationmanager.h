#ifndef APPLICATIONMANAGER_H
#define APPLICATIONMANAGER_H

#include <QObject>
#include <QDir>
#include <QDirIterator>

#include <core/adding_methods/wrappers/linux/daddingmethods.h>
#include <DJsonParser/djsonparser.h>

class ApplicationManager : public QObject
{
    Q_OBJECT
    Q_ENUMS(State)
    Q_PROPERTY(State state READ state WRITE setState NOTIFY stateChanged)

public:
    explicit ApplicationManager(QObject *parent = 0);
    enum State{IDLE, EXEC, SOURCE};

    void setState(State state);
    State state() const;

signals:
    void stateChanged(State);
public slots:
    void fileOpened(QString);
private:
    State m_state;
    QList<DAddingMethods::InjectDescription<DAddingMethods::Registers_x86>*> m_x86methodsList;
    QList<DAddingMethods::InjectDescription<DAddingMethods::Registers_x64>*> m_x64methodsList;
    QString m_targetPath;
    QStringList sourceExtensionList;
    DJsonParser jsonParser;
};

#endif // APPLICATIONMANAGER_H
