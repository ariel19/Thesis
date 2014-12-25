#include "pehelpers.h"

template <typename Register>
const QString Wrapper<Register>::methodsPath = "..\\..\\..\\..\\dDeflect\\data\\os\\win\\x86\\src\\"; // TODO: wczytywanie z config

template <typename Register>
const QString Wrapper<Register>::nasmPath = "X:\\Programy\\Nasm\\nasm.exe"; // TODO: wczytywanie z config

template <typename Register>
InjectDescription<Register>::InjectDescription(CallingMethod _method, Wrapper<Register> *_wrapper) :
    callingMethod(_method),
    wrapper(_wrapper)
{ }

template <typename Register>
InjectDescription<Register>::~InjectDescription()
{
    if(wrapper)
        delete wrapper;
}

template <typename Register>
Wrapper<Register> *InjectDescription<Register>::getWrapper()
{
    return wrapper;
}

template <typename Register>
CallingMethod InjectDescription<Register>::getCallingMethod() const
{
    return callingMethod;
}

template <typename Register>
ThreadWrapper<Register>::ThreadWrapper(QByteArray _code, QList<Wrapper<Register> *> _thread, uint16_t _sleepTime, QList<Register> _regToSave,
                             QMap<Register, QString> _params, Register _returns, Wrapper<Register> *_action) :
    Wrapper<Register>(_code, _regToSave, _params, _returns, _action),
    threadCodes(_thread),
    sleepTime(_sleepTime)
{ }

template <typename Register>
ThreadWrapper<Register>::~ThreadWrapper()
{
    foreach(Wrapper<Register> *w, threadCodes)
        if(w)
            delete w;
}

template <typename Register>
QList<Wrapper<Register> *> ThreadWrapper<Register>::getThreadWrappers()
{
    return threadCodes;
}

template <typename Register>
uint16_t ThreadWrapper<Register>::getSleepTime()
{
    return sleepTime;
}

template <typename Register>
Wrapper<Register>::Wrapper(QByteArray _code, QList<Register> _regToSave, QMap<Register, QString> _params,
                 Register _returns, Wrapper<Register> *_action) :
    code(_code),
    registersToSave(_regToSave),
    parameters(_params),
    returns(_returns),
    action(_action)
{ }

template <typename Register>
Wrapper<Register>::~Wrapper()
{
    if(action)
        delete action;
}

template <>
Wrapper<Register_x86> *Wrapper<Register_x86>::fromFile(QString name, bool thread_code)
{
    QByteArray code;
    QList<Register_x86> regToSave;
    QMap<Register_x86, QString> params;
    Register_x86 returns;
    Wrapper *action = nullptr;

    // TODO: jak wczytać akcję? podać z zewnątrz?
    // TODO: implementacja
    // TODO: czytanie z json

    // TODO: ścieżki i nazwy z config?
    if(QProcess::execute(nasmPath, {"-f", "bin", "-o", methodsPath + "data.bin", name}))
        return nullptr;

    QFileInfo fi(name);
    QFile f(QFileInfo(methodsPath, "data.bin").absoluteFilePath());
    if(!f.open(QFile::ReadOnly))
        return nullptr;

    code = f.readAll();
    f.close();

    // temp
    if(name.contains("create_thread"))
    {
        returns = Register_x86::None;
        regToSave.append({Register_x86::EAX, Register_x86::ECX, Register_x86::EDX, Register_x86::EDI});
        params.insert(Register_x86::EDX, "kernel32!CreateThread");
        params.insert(Register_x86::EDI, "THREAD!THREAD");
    }
    else if(name.contains("load_functions"))
    {
        returns = Register_x86::None;
        regToSave.append({Register_x86::EAX, Register_x86::EBX, Register_x86::ECX, Register_x86::EDX, Register_x86::ESI, Register_x86::EDI});
    }
    else if(name.contains("heap_flags"))
    {
        returns = Register_x86::EAX;
        regToSave.append({Register_x86::EAX, Register_x86::EBX, Register_x86::ECX, Register_x86::EDX, Register_x86::ESI, Register_x86::EDI});
        action = fromFile(Wrapper::methodsPath + "handlers\\message_box.asm");
        params.insert(Register_x86::EAX, "kernel32!GetVersion");
    }
    else
    {
        returns = Register_x86::EAX;
        regToSave.append({Register_x86::EAX, Register_x86::EBX, Register_x86::ECX, Register_x86::EDX, Register_x86::ESI, Register_x86::EDI});
        params.insert(Register_x86::EDI, "kernel32!ExitProcess");
        params.insert(Register_x86::ECX, "user32!MessageBoxA");
    }

    return thread_code ?
                new (std::nothrow) ThreadWrapper<Register_x86>(code, {Wrapper::fromFile(Wrapper::methodsPath + "methods\\heap_flags.asm")}, 5, regToSave, params, returns, action) :
                new (std::nothrow) Wrapper(code, regToSave, params, returns, action);
}

template <typename Register>
QByteArray Wrapper<Register>::getCode()
{
    return code;
}

template <typename Register>
QList<Register> Wrapper<Register>::getRegistersToSave()
{
    return registersToSave;
}

template <typename Register>
QMap<Register, QString> Wrapper<Register>::getParameters()
{
    return parameters;
}

template <typename Register>
Register Wrapper<Register>::getReturns()
{
    return returns;
}

template <typename Register>
Wrapper<Register> *Wrapper<Register>::getAction()
{
    return action;
}
