#ifndef TEST_ELF_H
#define TEST_ELF_H

#include <QFile>
#include <QDir>
#include <QVariant>
#include <QDebug>
#include <core/file_types/elffile.h>
#include <core/adding_methods/wrappers/linux/daddingmethods.h>

int oep_ptrace(const QString &elf_fname, const QString &ptrace_fname, const QString &elf_out);
int create_thread(const QString &elf_fname, const QString &thread_fname, const QString &elf_out);
void test();

bool test_oep_wrappers(const QString &elf_fname, const QString &wrapper,
                       const QString &method, const QString &handl);

bool test_thread_wrappers(const QString &elf_fname, const QString &wrapper,
                          const QString &method, const QString &handl);

bool test_init_oep_wrappers(const QString &elf_fname, const QString &wrapper,
                            const QString &method, const QString &handl);

bool test_initarray_oep_wrappers(const QString &elf_fname, const QString &wrapper,
                                 const QString &method, const QString &handl);

bool test_ctors_oep_wrappers(const QString &elf_fname, const QString &wrapper,
                             const QString &method, const QString &handl);

void test_wrappers();


#endif // TEST_ELF_H
