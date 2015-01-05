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

#include "test_elf.h"

#include <core/file_types/pefile.h>


int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    QFile f("C:\\Users\\jablonskim\\Desktop\\Programy\\putty.exe");
    //QFile f("C:\\Windows\\System32\\calc.exe");
    //QFile f("C:\\Users\\jablonskim\\Desktop\\Project1.exe");

    if(!f.open(QFile::ReadOnly))
        return 1;

    PEFile pe(f.readAll());

    f.close();

    if(!pe.isValid())
    {
        puts("Bad format!");
        return 1;
    }

    QList<DAddingMethods::InjectDescription<Registers_x86>*> ids;

    DAddingMethods::Wrapper<Registers_x86> wrapper;

    DAddingMethods::InjectDescription<Registers_x86> method1;
    method1.cm = DAddingMethods::CallingMethod::OEP;
    method1.adding_method = &wrapper;
    ids.append(&method1);
    //ids.append(new (std::nothrow) InjectDescription<Register_x86>(CallingMethod::EntryPoint, Wrapper<Register_x86>::fromFile(Wrapper<Register_x86>::helpersPath + "create_thread.asm", true)));
    //ids.append(new (std::nothrow) InjectDescription<Register_x86>(CallingMethod::TLS, Wrapper<Register_x86>::fromFile(Wrapper<Register_x86>::methodsPath + "handlers\\message_box.asm")));
    //ids.append(new (std::nothrow) InjectDescription<Registers_x86>(CallingMethod::Trampoline, Wrapper<Registers_x86>::fromFile(Wrapper<Registers_x86>::methodsPath + "nt_set_information_thread.asm")));

    //QList<InjectDescription<Register_x64>*> ids;
    //ids.append(new (std::nothrow) InjectDescription<Register_x64>(CallingMethod::EntryPoint, Wrapper<Register_x64>::fromFile(Wrapper<Register_x64>::methodsPath + "create_thread.asm", true)));
    //ids.append(new (std::nothrow) InjectDescription<Register_x64>(CallingMethod::TLS, Wrapper<Register_x64>::fromFile(Wrapper<Register_x64>::methodsPath + "handlers\\message_box.asm")));
    //ids.append(new (std::nothrow) InjectDescription<Register_x64>(CallingMethod::Trampoline, Wrapper<Register_x64>::fromFile(Wrapper<Register_x64>::methodsPath + "printf_test.asm")));

    PEAddingMethods adder(&pe);
    if(adder.injectCode(ids, 5))
        puts("Success!");
    else
        puts("Failed!");


    QFile nf("new_example.exe");
    if(!nf.open(QFile::WriteOnly))
        return 1;
    nf.write(pe.getData());
    nf.close();

    puts("OK!");

    return 0;
}

/*int main() {
    // test();
    // oep_ptrace("test", "ptrace", "test3");
    // test_flagx("a", "a2");
    // test_oep_wrappers("my32", "oep_t.asm", "ptrace_t.asm", "exit_t.asm");
    // test_oep_wrappers("my64", "oep64_t.asm", "ptrace64_t.asm", "exit64_t.asm");
    // test_thread_wrappers("my32", "thread_t.asm", "ptrace_t.asm", "exit_t.asm");
    // test_thread_wrappers("my64", "thread64_t.asm", "ptrace64_t.asm", "exit64_t.asm");
    // test_thread_wrappers("my64", "thread64p_t.asm", "ptrace64_t.asm", "exit_group64_t.asm");
    // test_thread_wrappers("my32", "threadp_t.asm", "ptrace_t.asm", "exit_group_t.asm");

    test_wrappers();
    return 0;
}*/
