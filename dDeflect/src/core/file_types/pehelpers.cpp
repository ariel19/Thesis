#include "pehelpers.h"


InjectDescription::InjectDescription(CallingMethod _method, Wrapper *_wrapper) :
    callingMethod(_method),
    wrapper(_wrapper)
{ }

InjectDescription::~InjectDescription()
{
    if(wrapper)
        delete wrapper;
}

Wrapper *InjectDescription::getWrapper()
{
    return wrapper;
}

CallingMethod InjectDescription::getCallingMethod()
{
    return callingMethod;
}

ThreadWrapper::ThreadWrapper(QByteArray _code, Wrapper *_thread, QList<Register> _regToSave,
                             QMap<Register, QString> _params, Register _returns, Wrapper *_action) :
    Wrapper(_code, _regToSave, _params, _returns, _action),
    threadCode(_thread)
{ }

ThreadWrapper::~ThreadWrapper()
{
    if(threadCode)
        delete threadCode;
}

Wrapper *ThreadWrapper::getThreadWrapper()
{
    return threadCode;
}


Wrapper::Wrapper(QByteArray _code, QList<Register> _regToSave, QMap<Register, QString> _params,
                 Register _returns, Wrapper *_action) :
    code(_code),
    registersToSave(_regToSave),
    parameters(_params),
    returns(_returns),
    action(_action)
{ }

Wrapper::~Wrapper()
{
    if(action)
        delete action;
}

Wrapper *Wrapper::fromFile(QString name)
{
    QByteArray code;
    QList<Register> regToSave;
    QMap<Register, QString> params;
    Register returns;
    Wrapper *action = nullptr;

    // TODO: jak wczytać akcję? podać z zewnątrz?
    // TODO: implementacja

    if(QProcess::execute("nasm", {"-f", "bin", "-o", "data.bin", name}))
        return nullptr;

    QFileInfo fi(name);
    QFile f(QFileInfo(fi.absoluteDir(), "data.bin").absoluteFilePath());
    if(!f.open(QFile::ReadOnly))
        return nullptr;

    code = f.readAll();
    f.close();

    // temp
    if(name.contains("load_functions"))
    {
        returns = Register::None;
        regToSave.append({Register::EAX, Register::EBX, Register::ECX, Register::EDX, Register::ESI, Register::EDI});
    }
    else
    {
        returns = Register::None;
        regToSave.append({Register::EAX, Register::EBX, Register::ECX, Register::EDX, Register::ESI, Register::EDI});
    }

    return new (std::nothrow) Wrapper(code, regToSave, params, returns, action);
}

QByteArray Wrapper::getCode()
{
    return code;
}

QList<Register> Wrapper::getRegistersToSave()
{
    return registersToSave;
}

QMap<Register, QString> Wrapper::getParameters()
{
    return parameters;
}

Register Wrapper::getReturns()
{
    return returns;
}

Wrapper *Wrapper::getAction()
{
    return action;
}
