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
