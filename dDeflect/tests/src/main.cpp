#include <cstdio>
#include <QFile>
#include <QDir>
#include <iostream>

//#include <core/file_types/elffile.h>
//#include <core/file_types/pefile.h>

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

// create thread wrapper
//void create_thread() {
//    asm("push eax");
//    asm("push edx");
//    asm("psuh ecx");

//    // create a thread using clone
//    // clone syscall + CLONE_THREAD
//    // stack address: http://linux.die.net/man/2/clone
//    // clone syscall number: 120
//    // documentation: http://docs.cs.up.ac.za/programming/asm/derick_tut/syscalls.html
//    // links: http://nlo.lists.kernelnewbies.narkive.com/npChDJyH/pt-regs-structure-for-sys-clone

//    // ptrace syscall
//    // link: http://mikecvet.wordpress.com/2010/08/14/ptrace-tutorial/
//    asm("mov eax, 26"); // sys_ptrace



//    asm("pop ecx");
//    asm("pop edx");
//    asm("pop eax");
//}

QByteArray create_ep_code_32(uint32_t oldEP, uint32_t imageBase)
{
    QByteArray code;

    code.append(QByteArray(10, '\x90'));
    unsigned char data[] = {
    0xE8,0x06,0x00,0x00,0x00,0x4B,0x45,0x52,0x4E,0x45,0x4C,0x5E,0x64,0xA1,0x30,0x00,
    0x00,0x00,0x8B,0x40,0x0C,0x8B,0x40,0x14,0x8B,0x50,0x28,0x85,0xD2,0x0F,0x84,0xB3,
    0x00,0x00,0x00,0x31,0xC9,0x83,0xF9,0x06,0x74,0x16,0x0F,0xB6,0x1C,0x0E,0x3A,0x1C,
    0x4A,0x74,0x0A,0x80,0xC3,0x20,0x3A,0x1C,0x4A,0x74,0x02,0xEB,0x15,0x41,0xEB,0xE5,
    0x83,0xC2,0x0C,0x80,0x3A,0x33,0x75,0x0A,0x83,0xC2,0x02,0x80,0x3A,0x32,0x75,0x02,
    0x74,0x04,0x8B,0x00,0xEB,0xC2,0x8B,0x40,0x10,0x50,0x03,0x40,0x3C,0x8B,0x40,0x78,
    0x03,0x04,0x24,0x50,0x8B,0x48,0x18,0x8B,0x40,0x20,0x03,0x44,0x24,0x04,0xE8,0x0E,
    0x00,0x00,0x00,0x47,0x65,0x74,0x50,0x72,0x6F,0x63,0x41,0x64,0x64,0x72,0x65,0x73,
    0x73,0x5E,0x8B,0x7C,0x88,0xFC,0x03,0x7C,0x24,0x04,0x31,0xDB,0x83,0xFB,0x0E,0x74,
    0x13,0x0F,0xB6,0x14,0x1F,0x3A,0x14,0x1E,0x75,0x03,0x43,0xEB,0xEF,0x83,0xE9,0x01,
    0x75,0xE0,0xEB,0x2F,0x5A,0x58,0x8B,0x7A,0x24,0x01,0xC7,0x0F,0xB7,0x7C,0x4F,0xFE,
    0x8B,0x4A,0x1C,0x01,0xC1,0x8B,0x14,0xB9,0x01,0xC2,0x52,0xE8,0x0D,0x00,0x00,0x00,
    0x4C,0x6F,0x61,0x64,0x4C,0x69,0x62,0x72,0x61,0x72,0x79,0x41,0x00,0x50,0xFF,0xD2,
    0x5A,0xEB,0x07,0x83,0xC4,0x08,0x31,0xC0,0x31,0xD2
    };
    code.append(QByteArray((char*)data, sizeof(data)));

    uint32_t old_ep = imageBase + oldEP;
    code.append('\xB8'); // mov eax, old_ep
    code.append(QByteArray::fromRawData(reinterpret_cast<const char *>(&old_ep), sizeof(uint32_t)));
    code.append("\xFF\xE0"); // jmp eax

    return code;
}

QByteArray create_ep_code_64(uint32_t oldEP, uint64_t imageBase)
{
    // TODO: przetestować
    QByteArray code;

    code.append(QByteArray(10, '\x90'));

    uint64_t old_ep = imageBase + oldEP;
    code.append("\x48\xB8"); // movabs rax,oldep
    code.append(QByteArray::fromRawData(reinterpret_cast<const char *>(&old_ep), sizeof(uint64_t)));
    code.append("\xFF\xE0"); // jmp eax

    return code;
}

int main()
{
    /*std::cout << QDir::currentPath().toStdString() << std::endl;

    ELF elf("a.out");
    std::cout << "valid: " << elf.is_valid() << std::endl;
    std::cout << "segments no: " << elf.get_number_of_segments() << std::endl;
    for (int i = 0; i < elf.get_number_of_segments(); ++i) {
        std::cout << "segment {" << i << "}: 0x" << std::hex<< elf.get_ph_seg_offset(i);
    }

    QByteArray nops(12, '\x90');
    Elf64_Addr nva;

    QByteArray nf = elf.extend_segment(nops, false, nva);

    if (!elf.set_entry_point(nva, nf))
        return 1;

    elf.write_to_file("a2.out", nf);*/

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

    QByteArray nd = create_ep_code_32(pe.getEntryPoint(), pe.getImageBase());

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

    return 0;
}
