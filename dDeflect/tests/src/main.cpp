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
