#include <cstdio>
#include <QFile>
#include <QDir>
#include <iostream>
#include <QVariant>
#include <QDebug>
#include <QCoreApplication>

#include <core/file_types/elffile.h>
#include <core/adding_methods/wrappers/daddingmethods.h>
#include <core/adding_methods/wrappers/peaddingmethods.h>
#include <ApplicationManager/DJsonParser/djsonparser.h>
#include <ApplicationManager/dsettings.h>
#include <ApplicationManager/dlogger.h>

#include "test_elf.h"

#include <core/file_types/pefile.h>


int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    //QFile f("C:\\Users\\jablonskim\\Desktop\\Programy\\putty.exe");
    //QFile f("C:\\Windows\\System32\\calc.exe");
    QFile f("C:\\Users\\jablonskim\\Desktop\\putty.exe");

    if(!f.open(QFile::ReadOnly))
        return 1;

    PEFile pe(f.readAll());

    f.close();

    if(!pe.is_valid())
    {
        puts("Bad format!");
        return 1;
    }

    DLogger::registerCallback({DLogger::Type::Error, DLogger::Type::Warning, DLogger::Type::Message},
                              [](QString msg)-> void { printf("%s\n", msg.toStdString().c_str()); });

    LOG_MSG("Start!");

    QList<DAddingMethods<Registers_x86>::InjectDescription*> ids;

    DJsonParser parser(DSettings::getSettings().getDescriptionsPath<Registers_x86>());
    DAddingMethods<Registers_x86>::Wrapper *wrapper = parser.loadInjectDescription<Registers_x86>("win_x86_is_debugger_present.json");
    wrapper->detect_handler = parser.loadInjectDescription<Registers_x86>("win_x86_handler_message_box.json");
    DAddingMethods<Registers_x86>::InjectDescription method1;
    method1.cm = DAddingMethods<Registers_x86>::CallingMethod::Trampoline;
    method1.adding_method = wrapper;
    ids.append(&method1);
    //ids.append(new (std::nothrow) InjectDescription<Register_x86>(CallingMethod::EntryPoint, Wrapper<Register_x86>::fromFile(Wrapper<Register_x86>::helpersPath + "create_thread.asm", true)));
    //ids.append(new (std::nothrow) InjectDescription<Register_x86>(CallingMethod::TLS, Wrapper<Register_x86>::fromFile(Wrapper<Register_x86>::methodsPath + "handlers\\message_box.asm")));
    //ids.append(new (std::nothrow) InjectDescription<Registers_x86>(CallingMethod::Trampoline, Wrapper<Registers_x86>::fromFile(Wrapper<Registers_x86>::methodsPath + "nt_set_information_thread.asm")));

    //QList<InjectDescription<Register_x64>*> ids;
    //ids.append(new (std::nothrow) InjectDescription<Register_x64>(CallingMethod::EntryPoint, Wrapper<Register_x64>::fromFile(Wrapper<Register_x64>::methodsPath + "create_thread.asm", true)));
    //ids.append(new (std::nothrow) InjectDescription<Register_x64>(CallingMethod::TLS, Wrapper<Register_x64>::fromFile(Wrapper<Register_x64>::methodsPath + "handlers\\message_box.asm")));
    //ids.append(new (std::nothrow) InjectDescription<Register_x64>(CallingMethod::Trampoline, Wrapper<Register_x64>::fromFile(Wrapper<Register_x64>::methodsPath + "printf_test.asm")));

    PEAddingMethods<Registers_x86> adder(&pe);
    adder.setCodeCoverage(5);
    if(adder.secure(ids) && adder.obfuscate(30))
        LOG_MSG("Success!");
    else
        LOG_ERROR("Failed!");


    QFile nf("new_example.exe");
    if(!nf.open(QFile::WriteOnly))
        return 1;
    nf.write(pe.getData());
    nf.close();

    if(!DAddingMethods<Registers_x86>::pack("new_example.exe"))
        LOG_ERROR("failed");

    LOG_MSG("File saved.");

    return 0;
}

//int main() {
    // test();
    // oep_ptrace("test", "ptrace", "test3");
    // test_flagx("a", "a2");
    // test_oep_wrappers("my32", "oep_t.asm", "ptrace_t.asm", "exit_t.asm");
    // test_oep_wrappers("my64", "oep64_t.asm", "ptrace64_t.asm", "exit64_t.asm");
    // test_thread_wrappers("my32", "thread_t.asm", "ptrace_t.asm", "exit_t.asm");
    // test_thread_wrappers("my64", "thread64_t.asm", "ptrace64_t.asm", "exit64_t.asm");
    // test_thread_wrappers("my64", "thread64p_t.asm", "ptrace64_t.asm", "exit_group64_t.asm");
    // test_thread_wrappers("my32", "threadp_t.asm", "ptrace_t.asm", "exit_group_t.asm");

    //test_wrappers();
    //return 0;
//}
