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

    ELFTester tester;
    QString file_name = "bin/my32";

    tester.test_everything_x86(file_name);

    return 0;
}
