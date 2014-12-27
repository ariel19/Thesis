#include "pehelpers.h"

template <>
const QString Wrapper<Register_x86>::methodsPath = "..\\..\\..\\..\\dDeflect\\data\\os\\win\\x86\\src\\"; // TODO: wczytywanie z config

template <>
const QString Wrapper<Register_x64>::methodsPath = "..\\..\\..\\..\\dDeflect\\data\\os\\win\\x64\\x64\\src\\"; // TODO: wczytywanie z config

template <typename Register>
const QString Wrapper<Register>::nasmPath = "C:\\jablonskim\\Programy\\nasm\\nasm.exe"; // TODO: wczytywanie z config

template <typename Register>
InjectDescription<Register>::InjectDescription(CallingMethod _method, Wrapper<Register> *_wrapper) :
    callingMethod(_method),
    wrapper(_wrapper)
{ }
template InjectDescription<Register_x86>::InjectDescription(CallingMethod _method, Wrapper<Register_x86> *_wrapper);
template InjectDescription<Register_x64>::InjectDescription(CallingMethod _method, Wrapper<Register_x64> *_wrapper);

template <typename Register>
InjectDescription<Register>::~InjectDescription()
{
    if(wrapper)
        delete wrapper;
}
template InjectDescription<Register_x86>::~InjectDescription();
template InjectDescription<Register_x64>::~InjectDescription();

template <typename Register>
Wrapper<Register> *InjectDescription<Register>::getWrapper()
{
    return wrapper;
}
template Wrapper<Register_x86> *InjectDescription<Register_x86>::getWrapper();
template Wrapper<Register_x64> *InjectDescription<Register_x64>::getWrapper();

template <typename Register>
CallingMethod InjectDescription<Register>::getCallingMethod() const
{
    return callingMethod;
}
template CallingMethod InjectDescription<Register_x86>::getCallingMethod() const;
template CallingMethod InjectDescription<Register_x64>::getCallingMethod() const;

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
template QList<Wrapper<Register_x86> *> ThreadWrapper<Register_x86>::getThreadWrappers();
template QList<Wrapper<Register_x64> *> ThreadWrapper<Register_x64>::getThreadWrappers();

template <typename Register>
uint16_t ThreadWrapper<Register>::getSleepTime()
{
    return sleepTime;
}
template uint16_t ThreadWrapper<Register_x86>::getSleepTime();
template uint16_t ThreadWrapper<Register_x64>::getSleepTime();

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
    typedef Register_x86 Register;

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

    QFile f(QFileInfo(methodsPath, "data.bin").absoluteFilePath());
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
    else if(name.contains("heap_flags"))
    {
        returns = Register::EAX;
        regToSave.append({Register::EAX, Register::EBX, Register::ECX, Register::EDX, Register::ESI, Register::EDI});
        action = fromFile(Wrapper::methodsPath + "handlers\\message_box.asm");
        params.insert(Register::EAX, "kernel32!GetVersion");
    }
    else
    {
        returns = Register::EAX;
        regToSave.append({Register::EAX, Register::EBX, Register::ECX, Register::EDX, Register::ESI, Register::EDI});
        params.insert(Register::EDI, "kernel32!ExitProcess");
        params.insert(Register::ECX, "user32!MessageBoxA");
    }

    return thread_code ?
                new (std::nothrow) ThreadWrapper<Register>(code, {Wrapper::fromFile(Wrapper::methodsPath + "methods\\heap_flags.asm")}, 5, regToSave, params, returns, action) :
                new (std::nothrow) Wrapper(code, regToSave, params, returns, action);
}

template <>
Wrapper<Register_x64> *Wrapper<Register_x64>::fromFile(QString name, bool thread_code)
{
    typedef Register_x64 Register;

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

    QFile f(QFileInfo(methodsPath, "data.bin").absoluteFilePath());
    if(!f.open(QFile::ReadOnly))
        return nullptr;

    code = f.readAll();
    f.close();

    // temp
    if(name.contains("create_thread"))
    {
        returns = Register::None;
        regToSave.append({Register::RAX, Register::RCX, Register::RDX, Register::RDI});
        params.insert(Register::RDX, "kernel32!CreateThread");
        params.insert(Register::RDI, "THREAD!THREAD");
    }
    else if(name.contains("load_functions"))
    {
        returns = Register::None;
        regToSave.append({Register::RAX, Register::RBX, Register::RCX, Register::RDX, Register::RSI, Register::RDI});
    }
    else if(name.contains("heap_flags"))
    {
        returns = Register::RAX;
        regToSave.append({Register::RAX, Register::RBX, Register::RCX, Register::RDX, Register::RSI, Register::RDI});
        action = fromFile(Wrapper::methodsPath + "handlers\\message_box.asm");
        params.insert(Register::RAX, "kernel32!GetVersion");
    }
    else
    {
        returns = Register::RAX;
        regToSave.append({Register::RAX, Register::RBX, Register::RCX, Register::RDX, Register::RSI, Register::RDI});
        params.insert(Register::RDI, "kernel32!ExitProcess");
        params.insert(Register::RCX, "user32!MessageBoxA");
    }

    return thread_code ?
                new (std::nothrow) ThreadWrapper<Register>(code, {Wrapper::fromFile(Wrapper::methodsPath + "methods\\heap_flags.asm")}, 5, regToSave, params, returns, action) :
                new (std::nothrow) Wrapper(code, regToSave, params, returns, action);
}


template <typename Register>
QByteArray Wrapper<Register>::getCode()
{
    return code;
}
template QByteArray Wrapper<Register_x86>::getCode();
template QByteArray Wrapper<Register_x64>::getCode();


template <typename Register>
QList<Register> Wrapper<Register>::getRegistersToSave()
{
    return registersToSave;
}
template QList<Register_x86> Wrapper<Register_x86>::getRegistersToSave();
template QList<Register_x64> Wrapper<Register_x64>::getRegistersToSave();


template <typename Register>
QMap<Register, QString> Wrapper<Register>::getParameters()
{
    return parameters;
}
template QMap<Register_x86, QString> Wrapper<Register_x86>::getParameters();
template QMap<Register_x64, QString> Wrapper<Register_x64>::getParameters();


template <typename Register>
Register Wrapper<Register>::getReturns()
{
    return returns;
}
template Register_x86 Wrapper<Register_x86>::getReturns();
template Register_x64 Wrapper<Register_x64>::getReturns();


template <typename Register>
Wrapper<Register> *Wrapper<Register>::getAction()
{
    return action;
}
template Wrapper<Register_x86> *Wrapper<Register_x86>::getAction();
template Wrapper<Register_x64> *Wrapper<Register_x64>::getAction();
