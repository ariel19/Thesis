#include <cstdio>
#include <QFile>
#include <QDir>
#include <iostream>

#include "pefile.h"
#include "elffile.h"

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

int main()
{
    //std::cout << QDir::currentPath().toStdString() << std::endl;

    ELF elf("a.out");
    std::cout << "valid: " << elf.is_valid() << std::endl;
    std::cout << "segments no: " << elf.get_number_of_segments() << std::endl;
    for (int i = 0; i < elf.get_number_of_segments(); ++i) {
        std::cout << "segment {" << i << "}: 0x" << std::hex<< elf.get_ph_seg_offset(i);
    }

    QByteArray nops(12, '\x00');
    Elf64_Addr nva;

    QByteArray nf = elf.extend_segment(nops, false, nva);

    if (elf.set_entry_point(nva, nf))
        return 1;

    elf.write_to_file("a2.out", nf);


    /*
    QFile f("example.exe");
    if(!f.open(QFile::ReadOnly))
        return 1;

    PEFile pe(f.readAll());

    f.close();

    if(!pe.isValid())
    {
        puts("Bad format!");
        return 1;
    }

    printf("%d, %d, %d\n", pe.getLastSectionNumberRaw(), pe.getLastSectionNumberMem(), pe.getNumberOfSections());

    for(unsigned int i = 0; i < pe.getNumberOfSections(); ++i)
        printf("Sekcja %u: %u\n", i, pe.getSectionFreeSpace(i));

    QByteArray nd(100, '\x90');
    nd[10] = '\xEB';
    nd[11] = '\xFE';

    unsigned int new_offset, new_mem_offset;
    if(!pe.addNewSection(QString(".test"), nd, new_offset, new_mem_offset) || !pe.setNewEntryPoint(new_mem_offset))
    {
        puts("Failed!");
        return 1;
    }

    printf("Offset: %x, Mem: %x\n", new_offset, new_mem_offset);

    QFile nf("new_example.exe");
    if(!nf.open(QFile::WriteOnly))
        return 1;
    nf.write(pe.getData());
    nf.close();

    puts("OK!");
    */


    return 0;
}
