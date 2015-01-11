#ifndef TEST_ELF_H
#define TEST_ELF_H

#include <QFile>
#include <QDir>
#include <QVariant>
#include <QDebug>
#include <core/file_types/elffile.h>
#include <core/adding_methods/wrappers/daddingmethods.h>

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

void test_obfuscation();

// NEW TESTS

void test_trampoline_wrapper();

void test_trampoline_wrapper_x86(const QString &bin_fld, const QString &dmeth_fld, const QString &out_fld, const QList<QString> &bin,
                                 const QList<QString> &meth, const QString &mode, const QString &handl_fld, const QList<QString> &hand);

void test_trampoline_wrapper_x64(const QString &bin_fld, const QString &dmeth_fld, const QString &out_fld, const QList<QString> &bin,
                                 const QList<QString> &meth, const QString &mode, const QString &handl_fld, const QList<QString> &hand);

void test_ctors_wrapper();

void test_ctors_wrapper_x86(const QString &bin_fld, const QString &dmeth_fld, const QString &out_fld, const QList<QString> &bin,
                            const QList<QString> &meth, const QString &mode, const QString &handl_fld, const QList<QString> &hand);

void test_ctors_wrapper_x64(const QString &bin_fld, const QString &dmeth_fld, const QString &out_fld, const QList<QString> &bin,
                            const QList<QString> &meth, const QString &mode, const QString &handl_fld, const QList<QString> &hand);

void test_init_array_wrapper();

void test_init_array_wrapper_x86(const QString &bin_fld, const QString &dmeth_fld, const QString &out_fld, const QList<QString> &bin,
                                 const QList<QString> &meth, const QString &mode, const QString &handl_fld, const QList<QString> &hand);

void test_init_array_wrapper_x64(const QString &bin_fld, const QString &dmeth_fld, const QString &out_fld, const QList<QString> &bin,
                                 const QList<QString> &meth, const QString &mode, const QString &handl_fld, const QList<QString> &hand);

void test_oep_wrapper();

void test_oep_wrapper_x86(const QString &bin_fld, const QString &dmeth_fld, const QString &out_fld, const QList<QString> &bin,
                          const QList<QString> &meth, const QString &mode, const QString &handl_fld, const QList<QString> &hand);

void test_oep_wrapper_x64(const QString &bin_fld, const QString &dmeth_fld, const QString &out_fld, const QList<QString> &bin,
                          const QList<QString> &meth, const QString &mode, const QString &handl_fld, const QList<QString> &hand);

void test_init_wrapper();

void test_init_wrapper_x86(const QString &bin_fld, const QString &dmeth_fld, const QString &out_fld, const QList<QString> &bin,
                           const QList<QString> &meth, const QString &mode, const QString &handl_fld, const QList<QString> &hand);

void test_init_wrapper_x64(const QString &bin_fld, const QString &dmeth_fld, const QString &out_fld, const QList<QString> &bin,
                           const QList<QString> &meth, const QString &mode, const QString &handl_fld, const QList<QString> &hand);


void test_thread_wrapper();

void test_thread_wrapper_x86(const QString &bin_fld, const QString &dmeth_fld, const QString &out_fld, const QList<QString> &bin,
                             const QList<QString> &meth, const QString &mode, const QString &handl_fld, const QList<QString> &hand);

void test_thread_wrapper_x64(const QString &bin_fld, const QString &dmeth_fld, const QString &out_fld, const QList<QString> &bin,
                             const QList<QString> &meth, const QString &mode, const QString &handl_fld, const QList<QString> &hand);


#endif // TEST_ELF_H
