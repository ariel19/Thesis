#include "test_elf.h"

#include <QFile>
#include <QDir>
#include <QVariant>
#include <QDebug>
#include <core/file_types/elffile.h>
#include <core/adding_methods/wrappers/elfaddingmethods.h>

#include <ApplicationManager/DJsonParser/djsonparser.h>
#include <ApplicationManager/dsettings.h>
#include <ApplicationManager/dlogger.h>

QList<QString> ELFTester::methods_x64 = {
    "lin_x64_cc",
    "lin_x64_ptrace",
    "lin_x64_rtdsc"
};

QList<QString> ELFTester::methods_x86 = {
    "lin_x86_cc",
    "lin_x86_ptrace",
    "lin_x86_rtdsc",
    "lin_x86_sigtrap"
};

QList<QString> ELFTester::handlers_x86 = {
    "lin_x86_exit",
    "lin_x86_exit_group"
};

QList<QString> ELFTester::handlers_x64 = {
    "lin_x64_exit",
    "lin_x64_exit_group"
};

QList<QString> ELFTester::wrappers_x86 = {
    "lin_x86_oepwrapper",
    "lin_x86_threadwrapper",
    "lin_x86_trampolinewrapper"
};

QList<QString> ELFTester::wrappers_x64 = {
    "lin_x64_oepwrapper",
    "lin_x64_threadwrapper",
    "lin_x64_trampolinewrapper"
};

bool ELFTester::test_one(QString input, QString output, ELFTester::Method type, QString method, QString handler) {
    QDir().mkdir("elf_test_outputs");

    QFile in(input);
    if(!in.open(QFile::ReadOnly))
        return false;

    QByteArray data = in.readAll();

    ELF elf(data);

    if(!elf.is_valid())
        return false;

    if(elf.is_x64() && !test_one_ex<Registers_x64>(&elf, type, method, handler))
        return false;

    if(elf.is_x86() && !test_one_ex<Registers_x86>(&elf, type, method, handler))
        return false;

    QFile out(QFileInfo(QString("elf_test_outputs"), output).absoluteFilePath());
    if(!out.open(QFile::WriteOnly))
        return false;

    out.write(elf.getData());

    out.close();
    in.close();

    return true;
}

bool ELFTester::test_all_methods(QString input, ELFTester::Method type, QString handler) {
    QFile in(input);
    if(!in.open(QFile::ReadOnly))
        return false;

    QByteArray data = in.readAll();

    ELF elf(data);
    if(!elf.is_valid())
        return false;

    bool is_x64 = elf.is_x64();

    in.close();

    QList<QString> *methods = is_x64 ? &methods_x64 : &methods_x86;

    int i = 1;
    foreach(QString method, *methods) {
        LOG_MSG(QString("\nStarting test %1 / %2...").arg(i).arg(methods->length()));
        // TODO: change!!!!
        // QString t = type == Method::EntryPoint ? "EntryPoint" : (type == Method::TLS ? "TLS" : "Trampoline");
        // QString output = QString("test_method_%1_%2_%3.exe").arg(method.split('.')[0]).arg(t).arg(handler.split('.')[0]);
        //if(!test_one(input, output, type, method, handler))
        //    return false;

        LOG_MSG(QString("Test %1 / %2 finished.").arg(i).arg(methods->length()));

        ++i;
    }

    return true;
}

bool ELFTester::test_all_handlers(QString input, ELFTester::Method type, QString method) {

    QFile in(input);
    if(!in.open(QFile::ReadOnly))
        return false;

    QByteArray data = in.readAll();

    ELF elf(data);
    if(!elf.is_valid())
        return false;

    bool is_x64 = elf.is_x64();

    in.close();

    QList<QString> *handlers = is_x64 ? &handlers_x64 : &handlers_x86;

    int i = 1;
    foreach(QString handler, *handlers) {
        LOG_MSG(QString("\nStarting test %1 / %2...").arg(i).arg(handlers->length()));
        // TODO: change
        // QString t = type == Method::EntryPoint ? "EntryPoint" : (type == Method::TLS ? "TLS" : "Trampoline");
        // QString output = QString("test_handler_%1_%2_%3.exe").arg(method.split('.')[0]).arg(t).arg(handler.split('.')[0]);
        // if(!test_one(input, output, type, method, handler))
        //    return false;

        LOG_MSG(QString("Test %1 / %2 finished.").arg(i).arg(handlers->length()));

        ++i;
    }

    return true;

}

bool ELFTester::test_all_wrappers(QString input, ELFTester::Method type, QString handler, QString method) {
    return true;
}

bool ELFTester::test_everything(QString input) {

    int errors = 0;
    /*
    LOG_MSG("Testing methods [EntryPoint]...");
    if(test_all_methods(input, PETester::Method::EntryPoint, "win_x86_handler_message_box"))
        LOG_MSG("Tests done (methods) [EntryPoint]!");
    else
    {
        errors++;
        LOG_MSG("Tests failed!");
    }

    LOG_MSG("Testing methods [Trampoline]...");
    if(test_all_methods(input, PETester::Method::Trampoline, "win_x86_handler_message_box"))
        LOG_MSG("Tests done (methods) [Trampoline]!");
    else
    {
        errors++;
        LOG_MSG("Tests failed!");
    }

    LOG_MSG("Testing methods [TLS]...");
    if(test_all_methods(input, PETester::Method::TLS, "win_x86_handler_message_box"))
        LOG_MSG("Tests done (methods) [TLS]!");
    else
    {
        errors++;
        LOG_MSG("Tests failed!");
    }


    LOG_MSG("Testing handlers [EntryPoint]...");
    if(test_all_handlers(input, PETester::Method::EntryPoint, "win_x86_is_debugger_present"))
        LOG_MSG("Tests done (handlers) [EntryPoint]!");
    else
    {
        errors++;
        LOG_MSG("Tests failed!");
    }

    LOG_MSG("Testing handlers [Trampoline]...");
    if(test_all_handlers(input, PETester::Method::Trampoline, "win_x86_is_debugger_present"))
        LOG_MSG("Tests done (handlers) [Trampoline]!");
    else
    {
        errors++;
        LOG_MSG("Tests failed!");
    }

    LOG_MSG("Testing handlers [TLS]...");
    if(test_all_handlers(input, PETester::Method::TLS, "win_x86_is_debugger_present"))
        LOG_MSG("Tests done (handlers) [TLS]!");
    else
    {
        errors++;
        LOG_MSG("Tests failed!");
    }

    if(errors)
        LOG_MSG(QString("%1 tests failed!").arg(errors));
    */
    return errors == 0;


}

template <typename Reg>
bool ELFTester::test_one_ex(ELF *elf, ELFTester::Method type, QString method, QString handler)
{
    DJsonParser parser(DSettings::getSettings().getDescriptionsPath<Reg>());

    ELFAddingMethods<Reg> adder(elf);

    typename DAddingMethods<Reg>::Wrapper *meth = parser.loadInjectDescription<Reg>(QString("%1.json").arg(method));
    if(!meth)
        return false;

    if(meth->ret != Reg::None)
    {
        meth->detect_handler = parser.loadInjectDescription<Reg>(QString("%1.json").arg(handler));

        if(!meth->detect_handler)
            return false;
    }

    typename DAddingMethods<Reg>::InjectDescription id;
    id.adding_method = meth;
    id.cm = static_cast<typename DAddingMethods<Reg>::CallingMethod>(type);
    QList<typename DAddingMethods<Reg>::InjectDescription*> ids = { &id };

    adder.secure(ids);

    if(!meth->detect_handler)
        delete meth->detect_handler;

    delete meth;

    return true;
}
