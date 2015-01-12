#ifndef TEST_PE_H
#define TEST_PE_H

#include <QString>

#include <core/adding_methods/wrappers/daddingmethods.h>
#include <core/adding_methods/wrappers/peaddingmethods.h>

class PETester
{
public:
    enum class Method {
        EntryPoint = 0,
        Trampoline = 2,
        TLS = 6
    };

    PETester();
    bool test_one(QString input, QString output, Method type, QString method, QString handler);
    bool test_all_methods(QString input, Method type, QString handler);
    bool test_all_handlers(QString input, Method type, QString method);
    bool test_thread(QString input, QString output, QString method, QString handler);
    bool test_everything(QString input);

private:
    template <typename Reg>
    bool test_one_ex(PEFile *pe, PETester::Method type, QString method, QString handler);

    template <typename Reg>
    bool test_thread_ex(PEFile *pe, QString method, QString handler);

    static QList<QString> methods_x86;
    static QList<QString> methods_x64;
    static QList<QString> handlers_x86;
    static QList<QString> handlers_x64;
};

#endif // TEST_PE_H
