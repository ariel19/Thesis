#ifndef PEHELPERS_H
#define PEHELPERS_H

#include <QList>
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

enum class Register_x86
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

enum class Register_x64
{
    RAX,
    RBX,
    RCX,
    RDX,
    RSI,
    RDI,
    RSP,
    RBP,
    R8,
    R9,
    R10,
    R11,
    R12,
    R13,
    R14,
    R15,
    None
};

template <typename Register>
class Wrapper
{
private:
    QByteArray code;
    QList<Register> registersToSave;
    QMap<Register, QString> parameters;
    Register returns;
    Wrapper<Register> *action;

public:
    Wrapper(QByteArray _code, QList<Register> _regToSave, QMap<Register, QString> _params,
            Register _returns = Register::None, Wrapper<Register> *_action = nullptr);
    virtual ~Wrapper();
    static Wrapper *fromFile(QString name, bool thread_code = false);
    static const QString methodsPath;
    static const QString nasmPath;
    static const QString ndisasmPath;

    QByteArray getCode();
    QList<Register> getRegistersToSave();
    QMap<Register, QString> getParameters();
    Register getReturns();
    Wrapper<Register> *getAction();

    Wrapper(const Wrapper&) = delete;
    Wrapper& operator=(const Wrapper&) = delete;
};

template <typename Register>
class ThreadWrapper : public Wrapper<Register>
{
private:
    QList<Wrapper<Register>*> threadCodes;
    uint16_t sleepTime;

public:
    ThreadWrapper(QByteArray _code, QList<Wrapper<Register> *> _thread, uint16_t _sleepTime, QList<Register> _regToSave, QMap<Register, QString> _params,
                  Register _returns = Register::None, Wrapper<Register> *_action = nullptr);
    ~ThreadWrapper();

    QList<Wrapper<Register>*> getThreadWrappers();
    uint16_t getSleepTime();

    ThreadWrapper(const ThreadWrapper&) = delete;
    ThreadWrapper& operator=(const ThreadWrapper&) = delete;
};

template <typename Register>
class InjectDescription
{
private:
    CallingMethod callingMethod;
    Wrapper<Register> *wrapper;

public:
    InjectDescription(CallingMethod _method, Wrapper<Register> *_wrapper);
    ~InjectDescription();
    Wrapper<Register> *getWrapper();
    CallingMethod getCallingMethod() const;

    InjectDescription(const InjectDescription&) = delete;
    InjectDescription& operator=(const InjectDescription&) = delete;
};

template <typename Register>
class BinaryCode
{
private:
    QByteArray code;
    QList<uint64_t> relocations;
    static const uint8_t addrSize;

public:
    BinaryCode() { }
    void append(QByteArray _code, bool relocation = false);
    QByteArray getBytes();
    QList<uint64_t> getRelocations(uint64_t codeBase);
    int length();
};

struct RelocationTable
{
    struct TypeOffset
    {
        uint8_t Type;
        uint16_t Offset;
    };

    uint32_t VirtualAddress;
    uint32_t SizeOfBlock;
    QList<TypeOffset> TypeOffsets;

    bool addOffset(uint16_t offset, uint8_t type);
    QByteArray toBytes();
};

#endif // PEHELPERS_H
