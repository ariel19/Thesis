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

#include "test_pe.h"
#include "test_elf.h"

#include <core/file_types/pefile.h>

bool read_and_parse(const QString &fname, QStringList &vals) {
    QFile f(fname);
    if (!f.open(QIODevice::ReadOnly))
        return false;

    QString content = f.readAll();
    QStringList lines = content.split('\n');

    // filter lines
    QString tline;
    int idx;
    foreach (QString line, lines) {
        tline = line.remove(QRegExp("(\\n\\t\\r)"));
        idx = tline.indexOf('(');
        if (idx != -1)
            tline.remove(0, idx + 1);
        idx = tline.lastIndexOf(')');
        if (idx != -1)
            tline.remove(idx, 1);
        if (!tline.isEmpty())
            vals.push_back(tline);
    }

    return true;
}

/*
int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);

    DLogger::registerCallback({DLogger::Type::Error, DLogger::Type::Warning, DLogger::Type::Message},
                              [](QString msg)-> void { printf("%s\n", msg.toStdString().c_str()); fflush(0); });

    LOG_MSG("Start!");

    PETester tester;
    QString file_name = "C:\\Users\\jablonskim\\Desktop\\putty.exe";

    tester.test_everything(file_name);

    return 0;
}
*/

int main(int argc, char **argv) {
    QCoreApplication app(argc, argv);

    DLogger::registerCallback({DLogger::Type::Error, DLogger::Type::Warning, DLogger::Type::Message},
                              [](QString msg)-> void { printf("%s\n", msg.toStdString().c_str()); fflush(0); });


    LOG_MSG("Start!");

    ELFTester tester("elf_test_outputs");
    QList<QString> file_names_x86 = { "bin/my32", "bin/myaslr32", "bin/derby32" };
    QList<QString> file_names_x64 = { "bin/my64", "bin/myaslr64", "bin/derby64", "bin/edb", "bin/dDeflect", "bin/telnet" };
    // QList<QString> file_names_x86 = { "bin/my32" };
    // QList<QString> file_names_x64 = { "bin/edb"/*, "bin/telnet"*/ };

    /*
    foreach (QString fname, file_names_x86)
        tester.test_everything_x86(fname, true);

    foreach (QString fname, file_names_x64)
        tester.test_everything_x64(fname, false);
    */

    QStringList lines;
    if (!read_and_parse("test", lines))
        return -1;

    foreach (QString line, lines)
        LOG_MSG(line);

    /*
    tester.test_one("bin/derby32", "derby32_ud2_x86", ELFTester::Method::Thread, "lin_x86_ptrace", "lin_x86_ud2", false, false, false);
    tester.test_one("bin/derby32", "derby32_fpe_x86", ELFTester::Method::Thread, "lin_x86_ptrace", "lin_x86_fpe", false, false, false);
    tester.test_one("bin/derby32", "derby32_mov_x86", ELFTester::Method::Thread, "lin_x86_ptrace", "lin_x86_mov", false, false, false);
    tester.test_one("bin/derby32", "derby32_jmp_x86", ELFTester::Method::Thread, "lin_x86_ptrace", "lin_x86_jmp", false, false, false);

    tester.test_one("bin/edb", "__edb_ud2_x64", ELFTester::Method::OEP, "lin_x64_ptrace", "lin_x64_ud2", false, true, false);
    tester.test_one("bin/edb", "__edb_fpe_x64", ELFTester::Method::OEP, "lin_x64_ptrace", "lin_x64_fpe", false, true, false);
    tester.test_one("bin/edb", "__edb_mov_x64", ELFTester::Method::OEP, "lin_x64_ptrace", "lin_x64_mov", false, true, false);
    tester.test_one("bin/edb", "__edb_jmp_x64", ELFTester::Method::OEP, "lin_x64_ptrace", "lin_x64_jmp", false, true, false);
    */

    return 0;
}
