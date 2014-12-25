#include <QFile>
#include <QDir>
#include <QVariant>
#include <QDebug>

#include <core/file_types/elffile.h>
#include <core/adding_methods/wrappers/linux/daddingmethods.h>

int oep_ptrace(const QString &elf_fname, const QString &ptrace_fname, const QString &elf_out) {
    ELF elf(elf_fname);
    qDebug() << "valid: " << elf.is_valid();
    qDebug() << "segments no: " << elf.get_number_of_segments();

    QFile first_test(ptrace_fname);
    if (!first_test.open(QIODevice::ReadOnly))
        return 1;
    QByteArray whole = first_test.readAll();
    // TODO: should be in wrapper after add jump instruction here
    Elf64_Addr oldep;
    if (!elf.get_entry_point(oldep))
        return 1;

    QByteArray jmp;
    if (elf.is_x86() || oldep < 0x100000000) {
        jmp.append(0xb8); // mov eax, addr
        // oldep -= 0x5;
        for(uint i = 0; i < sizeof(Elf32_Addr); ++i) {
            int a = (oldep >> (i * 8) & 0xff);
            jmp.append(static_cast<char>(a));
        }
        jmp.append("\xff\xe0", 2); // jmp eax
    }
    else if (elf.is_x64()) {
        jmp.append("\x48\xb8", 2); // mov rax, addr
        for(uint i = 0; i < sizeof(Elf64_Addr); ++i) {
            int a = (oldep >> (i * 8) & 0xff);
            jmp.append(static_cast<char>(a));
        }
        jmp.append("\xff\xe0", 2);
    }
    else return 1;

    // QByteArray nops(12, '\x90');
    Elf64_Addr nva;

    whole.append(jmp);

    QByteArray nf = elf.extend_segment(whole, true, nva);

    if (!elf.set_entry_point(nva, nf))
        return 1;

    qDebug() << "new entry point: " << QString("0x%1").arg(nva, 0, 16);

    elf.write_to_file(elf_out, nf);

    qDebug() << "saving to file: " << elf_out;

    return 0;
}

int create_thread(const QString &elf_fname, const QString &thread_fname, const QString &elf_out) {
    ELF elf(elf_fname);
    qDebug() << "valid: " << elf.is_valid();
    qDebug() << "segments no: " << elf.get_number_of_segments();

    QFile first_test(thread_fname);
    if (!first_test.open(QIODevice::ReadOnly))
        return 1;
    QByteArray whole = first_test.readAll();
    // TODO: should be in wrapper after add jump instruction here
    Elf64_Addr oldep;
    if (!elf.get_entry_point(oldep))
        return 1;

    QByteArray jmp;
    if (elf.is_x86() || oldep < 0x100000000) {
        jmp.append(0xb8); // mov eax, addr
        // oldep -= 0x5;
        for(uint i = 0; i < sizeof(Elf32_Addr); ++i) {
            int a = (oldep >> (i * 8) & 0xff);
            jmp.append(static_cast<char>(a));
        }
        jmp.append("\xff\xe0", 2); // jmp eax
    }
    else if (elf.is_x64()) {
        jmp.append("\x48\xb8", 2); // mov rax, addr
        for(uint i = 0; i < sizeof(Elf64_Addr); ++i) {
            int a = (oldep >> (i * 8) & 0xff);
            jmp.append(static_cast<char>(a));
        }
        jmp.append("\xff\xe0", 2);
    }
    else return 1;

    // QByteArray nops(12, '\x90');
    Elf64_Addr nva;

    whole.append(jmp);

    QByteArray nf = elf.extend_segment(whole, true, nva);

    if (!elf.set_entry_point(nva, nf))
        return 1;

    qDebug() << "new entry point: " << QString("0x%1").arg(nva, 0, 16);

    elf.write_to_file(elf_out, nf);

    qDebug() << "saving to file: " << elf_out;

    return 0;
}

void test() {
    qDebug() << "=========================================";
    qDebug() << "Testing OEP + ptrace for my 32-bit app...";
    // test oep + ptrace
    if (oep_ptrace("my32", "ptrace", "_my32")) {
        qDebug() << "something went wrong :(";
    }
    qDebug() << "Testing OEP + ptrace for my 32-bit app done";
    qDebug() << "=========================================";

    qDebug() << "Testing OEP + ptrace for my 64-bit app...";
    if (oep_ptrace("my64", "ptrace64", "_my64")) {
        qDebug() << "something went wrong :(";
    }
    qDebug() << "Testing OEP + ptrace for my 64-bit app done";
    qDebug() << "=========================================";

    qDebug() << "Testing OEP + ptrace for derby 32-bit app...";
    // test oep + ptrace
    if (oep_ptrace("derby32", "ptrace", "_derby32")) {
        qDebug() << "something went wrong :(";
    }
    qDebug() << "Testing OEP + ptrace for derby 32-bit app done";
    qDebug() << "=========================================";

    qDebug() << "Testing OEP + ptrace for derby 64-bit app...";
    if (oep_ptrace("derby64", "ptrace64", "_derby64")) {
        qDebug() << "something went wrong :(";
    }
    qDebug() << "Testing OEP + ptrace for derby 64-bit app done";
    qDebug() << "=========================================";

    qDebug() << "Testing OEP + ptrace for edb 64-bit app...";
    if (oep_ptrace("edb", "ptrace64", "_edb64")) {
        qDebug() << "something went wrong :(";
    }
    qDebug() << "Testing OEP + ptrace for edb 64-bit app done";
    qDebug() << "=========================================";

    qDebug() << "Testing OEP + ptrace for dDeflect 64-bit app...";
    if (oep_ptrace("dDeflect", "ptrace64", "_dDeflect64")) {
        qDebug() << "something went wrong :(";
    }
    qDebug() << "Testing OEP + ptrace for dDeflect 64-bit app done";
    qDebug() << "=========================================";

    qDebug() << "Testing OEP + thread for my 32-bit app...";
    // test oep + ptrace
    if (oep_ptrace("my32", "thread", "_my32t")) {
        qDebug() << "something went wrong :(";
    }
    qDebug() << "Testing OEP + thread for my 32-bit app done";
    qDebug() << "=========================================";

    qDebug() << "Testing OEP + thread for my 64-bit app...";
    if (oep_ptrace("my64", "thread64", "_my64t")) {
        qDebug() << "something went wrong :(";
    }
    qDebug() << "Testing OEP + thread for my 64-bit app done";
    qDebug() << "=========================================";

    qDebug() << "Testing OEP + thread for derby 32-bit app...";
    // test oep + ptrace
    if (oep_ptrace("derby32", "thread", "_derby32t")) {
        qDebug() << "something went wrong :(";
    }
    qDebug() << "Testing OEP + thread for derby 32-bit app done";
    qDebug() << "=========================================";

    qDebug() << "Testing OEP + thread for derby 64-bit app...";
    if (oep_ptrace("derby64", "thread64", "_derby64t")) {
        qDebug() << "something went wrong :(";
    }
    qDebug() << "Testing OEP + thread for derby 64-bit app done";
    qDebug() << "=========================================";

    qDebug() << "Testing OEP + thread for edb 64-bit app...";
    if (oep_ptrace("edb", "thread64", "_edb64t")) {
        qDebug() << "something went wrong :(";
    }
    qDebug() << "Testing OEP + thread for edb 64-bit app done";
    qDebug() << "=========================================";

    qDebug() << "Testing OEP + thread for dDeflect 64-bit app...";
    if (oep_ptrace("dDeflect", "thread64", "_dDeflect64t")) {
        qDebug() << "something went wrong :(";
    }
    qDebug() << "Testing OEP + thread for dDeflect 64-bit app done";
}

int test_flagx(const QString &elf_fname, const QString &elf_out) {
    ELF elf(elf_fname);
    qDebug() << "valid: " << elf.is_valid();
    qDebug() << "segments no: " << elf.get_number_of_segments();

    // QByteArray nops(12, '\x90');
    QByteArray nops(5, '\x00');
    // TODO: should be in wrapper after add jump instruction here
    Elf64_Addr oldep;
    if (!elf.get_entry_point(oldep))
        return 1;

    Elf64_Addr nva;

    QByteArray nf = elf.extend_segment(nops, true, nva);

    /*if (!elf.set_entry_point(nva, nf))
        return 1;*/

    qDebug() << "new entry point: " << QString("0x%1").arg(nva, 0, 16);

    elf.write_to_file(elf_out, nf);

    qDebug() << "saving to file: " << elf_out;

    return 0;
}

bool test_wrappers(const QString &elf_fname, const QString &wrapper,
                   const QString &method, const QString &handl) {
    // DAddingMethods::InjectDescription inject_desc;
    ELF elf(elf_fname);
    qDebug() << "valid: " << elf.is_valid();
    if (!elf.is_valid())
        return false;

    QFile code;

    DAddingMethods dam;
    if (elf.is_x86()) {
        DAddingMethods::InjectDescription<DAddingMethods::Registers_x86> inject_desc;

        DAddingMethods::OEPWrapper<DAddingMethods::Registers_x86> oepwrapper;
        DAddingMethods::Wrapper<DAddingMethods::Registers_x86> handler;
        DAddingMethods::Wrapper<DAddingMethods::Registers_x86> oepaction;

        code.setFileName(wrapper);
        if (!code.open(QIODevice::ReadOnly))
            return false;

        handler.params = { { "ret", "127" } };
        handler.detect_handler = nullptr;
        handler.code = code.readAll();

        code.close();
        code.setFileName(handl);
        if (!code.open(QIODevice::ReadOnly))
            return false;

        oepaction.code = code.readAll();
        oepaction.detect_handler = nullptr;
        oepaction.ret = DAddingMethods::Registers_x86::EAX;
        oepaction.used_regs = { DAddingMethods::Registers_x86::EAX, DAddingMethods::Registers_x86::ECX,
                                DAddingMethods::Registers_x86::EBX, DAddingMethods::Registers_x86::EDX,
                                DAddingMethods::Registers_x86::ESI };

        code.close();
        code.setFileName(method);
        if (!code.open(QIODevice::ReadOnly))
            return false;

        oepwrapper.detect_handler = &handler;
        oepwrapper.oep_action = &oepaction;
        oepwrapper.code = code.readAll();
        oepwrapper.used_regs = { DAddingMethods::Registers_x86::EDI };

        code.close();

        inject_desc.cm = DAddingMethods::CallingMethod::OEP;
        inject_desc.adding_method = &oepwrapper;

        dam.secure_elf<DAddingMethods::Registers_x86>(elf, inject_desc);
    }
    else {
        DAddingMethods::InjectDescription<DAddingMethods::Registers_x64> inject_desc;
        inject_desc.cm = DAddingMethods::CallingMethod::OEP;
        DAddingMethods::OEPWrapper<DAddingMethods::Registers_x64> oepwrapper;
        DAddingMethods::Wrapper<DAddingMethods::Registers_x64> handler;
        DAddingMethods::Wrapper<DAddingMethods::Registers_x64> oepaction;
    }


    return true;
}

int main() {
    // test();
    // oep_ptrace("test", "ptrace", "test3");
    // test_flagx("a", "a2");
    test_wrappers("my32", "oep_t.asm", "ptrace_t.asm", "exit_t.asm");

    return 0;
}
