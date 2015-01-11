#include "test_pe.h"

#include <QFile>
#include <QDir>
#include <QByteArray>
#include <QList>
#include <core/file_types/pefile.h>
#include <core/adding_methods/wrappers/peaddingmethods.h>
#include <ApplicationManager/DJsonParser/djsonparser.h>
#include <ApplicationManager/dsettings.h>

QList<QString> PETester::methods_x64 =
{
    "win_x64_anti_step_over",
    "win_x64_anti_step_over2",
    "win_x64_anti_step_over3",
    "win_x64_being_debugged",
    "win_x64_check_remote_debugger_present",
    "win_x64_dbg_break_point",
    "win_x64_find_window",
    "win_x64_heap_flags",
    "win_x64_is_debugger_present",
    "win_x64_nt_global_flag",
    "win_x64_nt_set_information_thread",
    "win_x64_open_process",
    "win_x64_parent_process",
    "win_x64_process_debug_flags",
    "win_x64_process_debug_object_handle",
    "win_x64_process_debug_port",
    "win_x64_tls_ep_breakpoint"
};

QList<QString> PETester::methods_x86 =
{
    "win_x86_anti_step_over",
    "win_x86_anti_step_over2",
    "win_x86_anti_step_over3",
    "win_x86_being_debugged",
    "win_x86_check_remote_debugger_present",
    "win_x86_dbg_break_point",
    "win_x86_find_window",
    "win_x86_heap_flags",
    "win_x86_is_debugger_present",
    "win_x86_nt_global_flag",
    "win_x86_nt_set_information_thread",
    "win_x86_open_process",
    "win_x86_parent_process",
    "win_x86_process_debug_flags",
    "win_x86_process_debug_object_handle",
    "win_x86_process_debug_port",
    "win_x86_tls_ep_breakpoint",
    "win_x86_fld",
    "win_x86_create_file"
};

QList<QString> PETester::handlers_x86 =
{
    "win_x86_handler_abort",
    "win_x86_handler_assert",
    "win_x86_handler_divide",
    "win_x86_handler_exit",
    "win_x86_handler_exit_process",
    "win_x86_handler_loop",
    "win_x86_handler_memleak",
    "win_x86_handler_message_box",
    "win_x86_handler_sigsegv",
    "win_x86_handler_stack",
    "win_x86_handler_ud2"
};

QList<QString> PETester::handlers_x64 =
{
    "win_x64_handler_abort",
    "win_x64_handler_assert",
    "win_x64_handler_divide",
    "win_x64_handler_exit",
    "win_x64_handler_exit_process",
    "win_x64_handler_loop",
    "win_x64_handler_memleak",
    "win_x64_handler_message_box",
    "win_x64_handler_sigsegv",
    "win_x64_handler_stack",
    "win_x64_handler_ud2"
};

PETester::PETester()
{
}

bool PETester::test_one(QString input, QString output, PETester::Method type, QString method, QString handler)
{
    QDir().mkdir("tests_output");

    QFile in(input);
    if(!in.open(QFile::ReadOnly))
        return false;

    QByteArray data = in.readAll();

    PEFile pe(data);

    if(!pe.is_valid())
        return false;

    if(pe.is_x64() && !test_one_ex<Registers_x64>(&pe, type, method, handler))
        return false;

    if(pe.is_x86() && !test_one_ex<Registers_x86>(&pe, type, method, handler))
        return false;

    QFile out(QFileInfo(QString("tests_output"), output).absoluteFilePath());
    if(!out.open(QFile::WriteOnly))
        return false;

    out.write(pe.getData());

    out.close();
    in.close();

    return true;
}

bool PETester::test_all_methods(QString input, Method type, QString handler)
{
    QFile in(input);
    if(!in.open(QFile::ReadOnly))
        return false;

    QByteArray data = in.readAll();

    PEFile pe(data);
    if(!pe.is_valid())
        return false;

    bool is_x64 = pe.is_x64();

    in.close();

    QList<QString> *methods = is_x64 ? &methods_x64 : &methods_x86;

    foreach(QString method, *methods)
    {
        QString t = type == Method::EntryPoint ? "EntryPoint" : (type == Method::TLS ? "TLS" : "Trampoline");
        QString output = QString("test_method_%1_%2_%3.exe").arg(method.split('.')[0]).arg(t).arg(handler.split('.')[0]);
        if(!test_one(input, output, type, method, handler))
            return false;
    }

    return true;
}

bool PETester::test_all_handlers(QString input, Method type, QString method)
{
    QFile in(input);
    if(!in.open(QFile::ReadOnly))
        return false;

    QByteArray data = in.readAll();

    PEFile pe(data);
    if(!pe.is_valid())
        return false;

    bool is_x64 = pe.is_x64();

    in.close();

    QList<QString> *handlers = is_x64 ? &handlers_x64 : &handlers_x86;

    foreach(QString handler, *handlers)
    {
        QString t = type == Method::EntryPoint ? "EntryPoint" : (type == Method::TLS ? "TLS" : "Trampoline");
        QString output = QString("test_handler_%1_%2_%3.exe").arg(method.split('.')[0]).arg(t).arg(handler.split('.')[0]);
        if(!test_one(input, output, type, method, handler))
            return false;
    }

    return true;
}

template <typename Reg>
bool PETester::test_one_ex(PEFile *pe, PETester::Method type, QString method, QString handler)
{
    DJsonParser parser(DSettings::getSettings().getDescriptionsPath<Reg>());

    PEAddingMethods<Reg> adder(pe);

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
