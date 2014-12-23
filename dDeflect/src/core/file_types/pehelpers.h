#ifndef PEHELPERS_H
#define PEHELPERS_H

#include <Qlist>
#include <QMap>
#include <QString>
#include <QProcess>
#include <QFile>
#include <QFileInfo>
#include <QDir>

enum class CallingMethod
{
    EntryPoint,
    TLS,
    Trampoline
};

enum class Register
{
    EAX,
    EBX,
    ECX,
    EDX,
    ESI,
    EDI,
    ESP,
    EBP,
    None
};

class Wrapper
{
private:
    QByteArray code;
    QList<Register> registersToSave;
    QMap<Register, QString> parameters;
    Register returns;
    Wrapper *action;

public:
    Wrapper(QByteArray _code, QList<Register> _regToSave, QMap<Register, QString> _params,
            Register _returns = Register::None, Wrapper *_action = nullptr);
    virtual ~Wrapper();
    static Wrapper *fromFile(QString name);

    QByteArray getCode();
    QList<Register> getRegistersToSave();
    QMap<Register, QString> getParameters();
    Register getReturns();
    Wrapper *getAction();
};

class ThreadWrapper : public Wrapper
{
private:
    Wrapper *threadCode;

public:
    ThreadWrapper(QByteArray _code, Wrapper *_thread, QList<Register> _regToSave, QMap<Register, QString> _params,
                  Register _returns = Register::None, Wrapper *_action = nullptr);
    ~ThreadWrapper();

    Wrapper *getThreadWrapper();
};

class InjectDescription
{
private:
    CallingMethod callingMethod;
    Wrapper *wrapper;

public:
    InjectDescription(CallingMethod _method, Wrapper *_wrapper);
    ~InjectDescription();
    Wrapper *getWrapper();
    CallingMethod getCallingMethod();
};

#endif // PEHELPERS_H
