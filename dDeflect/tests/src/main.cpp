#include <cstdio>
#include <QFile>
#include <QDir>
#include <iostream>

#include <core/file_types/pefile.h>
#include <QCoreApplication>

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    //QFile f("C:\\Users\\jablonskim\\Desktop\\Programy\\putty.exe");
    //QFile f("C:\\Windows\\System32\\calc.exe");
    QFile f("C:\\Users\\jablonskim\\Desktop\\Project1.exe");

    if(!f.open(QFile::ReadOnly))
        return 1;

    PEFile pe(f.readAll());

    f.close();

    if(!pe.isValid())
    {
        puts("Bad format!");
        return 1;
    }

    //QList<InjectDescription<Register_x86>*> ids;
    //ids.append(new (std::nothrow) InjectDescription<Register_x86>(CallingMethod::EntryPoint, Wrapper<Register_x86>::fromFile(Wrapper<Register_x86>::methodsPath + "create_thread.asm", true)));
    //ids.append(new (std::nothrow) InjectDescription<Register_x86>(CallingMethod::TLS, Wrapper<Register_x86>::fromFile(Wrapper<Register_x86>::methodsPath + "handlers\\message_box.asm")));
    //ids.append(new (std::nothrow) InjectDescription<Register_x86>(CallingMethod::Trampoline, Wrapper<Register_x86>::fromFile(Wrapper<Register_x86>::methodsPath + "printf_test.asm")));

    QList<InjectDescription<Register_x64>*> ids;
    //ids.append(new (std::nothrow) InjectDescription<Register_x64>(CallingMethod::EntryPoint, Wrapper<Register_x64>::fromFile(Wrapper<Register_x64>::methodsPath + "create_thread.asm", true)));
    //ids.append(new (std::nothrow) InjectDescription<Register_x64>(CallingMethod::TLS, Wrapper<Register_x64>::fromFile(Wrapper<Register_x64>::methodsPath + "handlers\\message_box.asm")));
    ids.append(new (std::nothrow) InjectDescription<Register_x64>(CallingMethod::Trampoline, Wrapper<Register_x64>::fromFile(Wrapper<Register_x64>::methodsPath + "printf_test.asm")));

    if(pe.injectCode(ids))
        puts("Success!");
    else
        puts("Failed!");

    qDeleteAll(ids);

    QFile nf("new_example.exe");
    if(!nf.open(QFile::WriteOnly))
        return 1;
    nf.write(pe.getData());
    nf.close();

    puts("OK!");

    return 0;
}
