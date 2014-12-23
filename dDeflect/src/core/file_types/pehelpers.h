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
    static Wrapper *fromFile(QString name, bool thread_code = false);
    static const QString methodsPath;
    static const QString nasmPath;

    QByteArray getCode();
    QList<Register> getRegistersToSave();
    QMap<Register, QString> getParameters();
    Register getReturns();
    Wrapper *getAction();

    Wrapper(const Wrapper&) = delete;
    Wrapper& operator=(const Wrapper&) = delete;
};

class ThreadWrapper : public Wrapper
{
private:
    QList<Wrapper*> threadCodes;
    uint16_t sleepTime;

public:
    ThreadWrapper(QByteArray _code, QList<Wrapper *> _thread, uint16_t _sleepTime, QList<Register> _regToSave, QMap<Register, QString> _params,
                  Register _returns = Register::None, Wrapper *_action = nullptr);
    ~ThreadWrapper();

    QList<Wrapper*> getThreadWrappers();
    uint16_t getSleepTime();

    ThreadWrapper(const ThreadWrapper&) = delete;
    ThreadWrapper& operator=(const ThreadWrapper&) = delete;
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
    CallingMethod getCallingMethod() const;

    InjectDescription(const InjectDescription&) = delete;
    InjectDescription& operator=(const InjectDescription&) = delete;
};

#endif // PEHELPERS_H
