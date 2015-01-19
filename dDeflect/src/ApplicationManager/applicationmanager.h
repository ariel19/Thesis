#ifndef APPLICATIONMANAGER_H
#define APPLICATIONMANAGER_H

#include <QObject>
#include <QDir>
#include <QDirIterator>
#include <../QtQml/qqmllist.h>
#include <QList>


#include <core/adding_methods/wrappers/peaddingmethods.h>
#include <core/adding_methods/wrappers/elfaddingmethods.h>
#include <ApplicationManager/DJsonParser/djsonparser.h>
#include <ApplicationManager/DSourceCodeParser/dsourcecodeparser.h>
#include <ApplicationManager/dsettings.h>
#include <DMethods/method.h>

class ApplicationManager : public QObject
{
    Q_OBJECT
    Q_ENUMS(State)
    Q_ENUMS(Arch)
    Q_PROPERTY(State state READ state WRITE setState NOTIFY stateChanged)
    Q_PROPERTY(Arch archType READ archType WRITE setArchType NOTIFY archTypeChanged)
    Q_PROPERTY(QVariantList x86MethodsNames READ x86MethodsNames NOTIFY x86MethodsNamesChanged)

    Q_PROPERTY(QQmlListProperty<Method> x86methods READ x86methods NOTIFY x86methodsChanged)
    Q_PROPERTY(QQmlListProperty<Method> x64methods READ x64methods NOTIFY x64methodsChanged)

    Q_PROPERTY(QQmlListProperty<Method> x86handlers READ x86handlers NOTIFY x86handlersChanged)
    Q_PROPERTY(QQmlListProperty<Method> x64handlers READ x64handlers NOTIFY x64handlersChanged)

public:
    template <typename Reg>
    using IDList = QList< Wrapper<Reg>*>;
    template <typename Reg>
    using FIDMapping = QHash<QString, IDList<Reg>>;

    explicit ApplicationManager(QObject *parent = 0);
    virtual ~ApplicationManager();
    enum State{IDLE, EXEC, SOURCE};
    enum Arch{X86,X64};
    void setState(State state);
    void setArchType(Arch t);

    State state() const;
    Arch archType();
    QQmlListProperty<Method> x86methods();
    QQmlListProperty<Method> x64methods();
    QQmlListProperty<Method> x86handlers();
    QQmlListProperty<Method> x64handlers();
    QVariantList x86MethodsNames();

signals:
    void stateChanged(State);
    void x86MethodsNamesChanged();
    void archTypeChanged();

    void x86methodsChanged();
    void x64methodsChanged();

    void x86handlersChanged();
    void x64handlersChanged();

    void pathChanged();

public slots:
    void fileOpened(QString);
    void applyClicked(QVariantList methodsChosen);
    void insertMethods(FIDMapping<Registers_x86>);
private:
    State m_state;
    IDList<Registers_x86> m_x86methodsList;
    QVariantList m_x86MethodsNames;
    IDList<Registers_x64> m_x64methodsList;
    QVariantList m_x64MethodsNames;

    QString m_targetPath;
    QStringList sourceExtensionList;
    DJsonParser jsonParser;
    DSourceCodeParser *sourceParser;
    Arch m_archType;

    QList<Method*> m_methodsx86;
    QList<Method*> m_methodsx64;

    QList<Method*> m_handlersx86;
    QList<Method*> m_handlersx64;

};

#endif // APPLICATIONMANAGER_H
