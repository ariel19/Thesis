#include <cstdio>
#include <QFile>
#include <QDir>
#include <QVariant>
#include <QDebug>
#include <iostream>

#include <core/file_types/elffile.h>
#include <core/file_types/pefile.h>

//void wrapper()
//{
//    int is_dbg = 0;

//    asm();

//    asm("db !__METHOD_DATA___!");

//    asm("mov is_dbg, !__RETURN_VALUE__!");

//    // TODO: akcja
//}

//void wrapper()
//{
//    volatile int is_dbg = 0;

//    asm("push ecx");
//    asm("push eax");
//    asm("push edx");

//    asm("db 0x64, 0x8B, 0x0D, 0x30, 0x00, 0x00, 0x00, 0x31, 0xC0, 0x8A, 0x51, 0x68, 0x80, 0xE2, 0x70, 0x80, 0xFA, 0x70, 0x0F, 0x94, 0xC0");

//    asm("mov byte is_dbg, al");

//    asm("pop edx");
//    asm("pop eax");
//    asm("pop ecx");

//    if(is_dbg)
//        printf("DEBUG!");
//}

// TODO: should be wrapper
int oep_ptrace(const QString &elf_fname, const QString &ptrace_fname, const QString &elf_out) {
    ELF elf(elf_fname);
    qDebug() << "valid: " << elf.is_valid();
    qDebug() << "segments no: " << elf.get_number_of_segments();

    /*
    for (int i = 0; i < elf.get_number_of_segments(); ++i) {
        std::cout << "segment {" << i << "}: 0x" << std::hex<< elf.get_ph_seg_offset(i);
    }
    */

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

    QByteArray nf = elf.extend_segment(whole, false, nva);

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

    /*
    for (int i = 0; i < elf.get_number_of_segments(); ++i) {
        std::cout << "segment {" << i << "}: 0x" << std::hex<< elf.get_ph_seg_offset(i);
    }
    */

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

    QByteArray nf = elf.extend_segment(whole, false, nva);

    if (!elf.set_entry_point(nva, nf))
        return 1;

    qDebug() << "new entry point: " << QString("0x%1").arg(nva, 0, 16);

    elf.write_to_file(elf_out, nf);

    qDebug() << "saving to file: " << elf_out;

    return 0;
}

int main() {
    /*
    // test oep + ptrace
    if (oep_ptrace("a.out", "ptrace", "a2.out")) {
        qDebug() << "something went wrong :(";
        return 1;
    }
    */

    // test oep + thread with dummy write function
    if (oep_ptrace("a.out", "thread", "a2.out")) {
        qDebug() << "something went wrong :(";
        return 1;
    }

    return 0;
}
