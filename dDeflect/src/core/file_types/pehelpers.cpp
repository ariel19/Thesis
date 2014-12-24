#include "pehelpers.h"

const QString Wrapper::methodsPath = "..\\..\\..\\..\\dDeflect\\data\\os\\win\\x86\\src\\"; // TODO: wczytywanie z config
const QString Wrapper::nasmPath = "X:\\Programy\\Nasm\\nasm.exe"; // TODO: wczytywanie z config

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

CallingMethod InjectDescription::getCallingMethod() const
{
    return callingMethod;
}

ThreadWrapper::ThreadWrapper(QByteArray _code, QList<Wrapper*> _thread, uint16_t _sleepTime, QList<Register> _regToSave,
                             QMap<Register, QString> _params, Register _returns, Wrapper *_action) :
    Wrapper(_code, _regToSave, _params, _returns, _action),
    threadCodes(_thread),
    sleepTime(_sleepTime)
{ }

ThreadWrapper::~ThreadWrapper()
{
    foreach(Wrapper *w, threadCodes)
        if(w)
            delete w;
}

QList<Wrapper*> ThreadWrapper::getThreadWrappers()
{
    return threadCodes;
}

uint16_t ThreadWrapper::getSleepTime()
{
    return sleepTime;
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

Wrapper *Wrapper::fromFile(QString name, bool thread_code)
{
    QByteArray code;
    QList<Register> regToSave;
    QMap<Register, QString> params;
    Register returns;
    Wrapper *action = nullptr;

    // TODO: jak wczytać akcję? podać z zewnątrz?
    // TODO: implementacja
    // TODO: czytanie z json

    // TODO: ścieżki i nazwy z config?
    if(QProcess::execute(nasmPath, {"-f", "bin", "-o", methodsPath + "data.bin", name}))
        return nullptr;

    QFileInfo fi(name);
    QFile f(QFileInfo(fi.absoluteDir(), "data.bin").absoluteFilePath());
    if(!f.open(QFile::ReadOnly))
        return nullptr;

    code = f.readAll();
    f.close();

    // temp
    if(name.contains("create_thread"))
    {
        returns = Register::None;
        regToSave.append({Register::EAX, Register::ECX, Register::EDX, Register::EDI});
        params.insert(Register::EDX, "kernel32!CreateThread");
        params.insert(Register::EDI, "THREAD!THREAD");
    }
    else if(name.contains("load_functions"))
    {
        returns = Register::None;
        regToSave.append({Register::EAX, Register::EBX, Register::ECX, Register::EDX, Register::ESI, Register::EDI});
    }
    else
    {
        returns = Register::None;
        regToSave.append({Register::EAX, Register::EBX, Register::ECX, Register::EDX, Register::ESI, Register::EDI});
        params.insert(Register::EAX, "user32!MessageBoxA");
    }

    return thread_code ?
                new (std::nothrow) ThreadWrapper(code, {Wrapper::fromFile(Wrapper::methodsPath + "thread_example.asm")}, 5, regToSave, params, returns, action) :
                new (std::nothrow) Wrapper(code, regToSave, params, returns, action);
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
