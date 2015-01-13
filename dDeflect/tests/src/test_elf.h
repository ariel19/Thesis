#ifndef TEST_ELF_H
#define TEST_ELF_H

#include <QFile>
#include <QDir>
#include <QVariant>
#include <QDebug>
#include <core/file_types/elffile.h>
#include <core/adding_methods/wrappers/daddingmethods.h>

class ELFTester {
public:
    enum class Method {
        OEP = 0,
        Thread,
        Trampoline,
        INIT,
        INIT_ARRAY,
        CTORS
    };
    ELFTester() {}
    bool test_one(QString input, QString output, Method type, QString method, QString handler);
    bool test_all_methods(QString input, Method type, QString handler);
    bool test_all_handlers(QString input, Method type, QString method);
    bool test_everything_x86(QString input);
    bool test_everything_x64(QString input);

private:
    template <typename Reg>
    bool test_one_ex(ELF *elf, Method type, QString method, QString handler);

    static QList<QString> methods_x86;
    static QList<QString> methods_x64;
    static QList<QString> handlers_x86;
    static QList<QString> handlers_x64;
    static QMap<Method, QString> wrappers_x86;
    static QMap<Method, QString> wrappers_x64;
    static QMap<Method, QString> smethods;
};


#endif // TEST_ELF_H
