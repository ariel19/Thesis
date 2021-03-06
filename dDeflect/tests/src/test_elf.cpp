#include "test_elf.h"

#include <QFile>
#include <QDir>
#include <QVariant>
#include <QDebug>
#include <QProcess>
#include <core/file_types/elffile.h>
#include <core/adding_methods/wrappers/elfaddingmethods.h>

#include <ApplicationManager/DJsonParser/djsonparser.h>
#include <ApplicationManager/dsettings.h>
#include <ApplicationManager/dlogger.h>

QList<QString> ELFTester::methods_x64 = {
    "lin_x64_cc",
    "lin_x64_ptrace",
    "lin_x64_rtdsc"
};

QList<QString> ELFTester::methods_x86 = {
    "lin_x86_cc",
    "lin_x86_ptrace",
    "lin_x86_rtdsc",
    "lin_x86_sigtrap"
};

QList<QString> ELFTester::handlers_x86 = {
    "lin_x86_exit",
    "lin_x86_exit_group",
    "lin_x86_fpe",
    "lin_x86_ud2",
    "lin_x86_jmp",
    "lin_x86_mov"
};

QList<QString> ELFTester::handlers_x64 = {
    "lin_x64_exit",
    "lin_x64_exit_group",
    "lin_x64_fpe",
    "lin_x64_ud2",
    "lin_x64_jmp",
    "lin_x64_mov"
};

QMap<ELFTester::Method, QString> ELFTester::wrappers_x86 = {
    { ELFTester::Method::OEP, "lin_x86_oepwrapper" },
    { ELFTester::Method::Thread, "lin_x86_threadwrapper" },
    { ELFTester::Method::Trampoline, "lin_x86_trampolinewrapper" },
    { ELFTester::Method::CTORS, "lin_x86_trampolinewrapper" },
    { ELFTester::Method::INIT, "lin_x86_trampolinewrapper" },
    { ELFTester::Method::INIT_ARRAY, "lin_x86_trampolinewrapper" }
};

QMap<ELFTester::Method, QString> ELFTester::wrappers_x64 = {
    { ELFTester::Method::OEP, "lin_x64_oepwrapper" },
    { ELFTester::Method::Thread, "lin_x64_threadwrapper"},
    { ELFTester::Method::Trampoline, "lin_x64_trampolinewrapper" },
    { ELFTester::Method::CTORS, "lin_x64_trampolinewrapper" },
    { ELFTester::Method::INIT, "lin_x64_trampolinewrapper" },
    { ELFTester::Method::INIT_ARRAY, "lin_x64_trampolinewrapper" }
};

QMap<ELFTester::Method, QString> ELFTester::smethods = {
    { ELFTester::Method::OEP, "OEP" },
    { ELFTester::Method::Thread, "Thread" },
    { ELFTester::Method::Trampoline, "Trampoline" },
    { ELFTester::Method::INIT, "INIT" },
    { ELFTester::Method::INIT_ARRAY, "INIT_ARRAY" },
    { ELFTester::Method::CTORS, "CTORS" }
};

bool ELFTester::test_one(QString input, QString output, ELFTester::Method type, QString method,
                         QString handler, bool x, bool obfuscate, bool pack) {
    QDir().mkdir(secured_files_dir);

    QFile in(input);
    if(!in.open(QFile::ReadOnly))
        return false;

    QByteArray data = in.readAll();

    ELF elf(data);

    if(!elf.is_valid())
        return false;

    SecuredState ss;
    if(elf.is_x64()) {
        ss = test_one_ex<Registers_x64>(&elf, type, method, handler, x, obfuscate);
        if (ss == SecuredState::NONCOMPATIBLE)
            return true;
        if (ss != SecuredState::SECURED)
            return false;
    }

    if(elf.is_x86()) {
        ss = test_one_ex<Registers_x86>(&elf, type, method, handler, x, obfuscate);
        if (ss == SecuredState::NONCOMPATIBLE)
            return true;
        if (ss != SecuredState::SECURED)
            return false;
    }

    QString fullpath = QFileInfo(secured_files_dir, output).absoluteFilePath();
    QFile out(fullpath);
    if(!out.open(QFile::WriteOnly))
        return false;

    out.write(elf.getData());

    out.close();
    in.close();

    // make file executable
    QProcess chmod;
    LOG_MSG("Making output file an executable");

    chmod.start(QString("chmod +x %1").arg(fullpath));

    if(!chmod.waitForFinished(5 * 60 * 1000)) {
        LOG_ERROR("chmod failed.");
        return false;
    }

    if (pack) {
        if (elf.is_x86()) {
            if (!DAddingMethods<Registers_x86>::pack(fullpath))
                return false;
        }
        else if (elf.is_x64()) {
            if (!DAddingMethods<Registers_x64>::pack(fullpath))
                return false;
        }
    }


    return true;
}

bool ELFTester::test_all_methods(QString input, ELFTester::Method type, QString handler, bool x, bool obfuscate, bool pack) {
    QFile in(input);
    if(!in.open(QFile::ReadOnly))
        return false;

    QByteArray data = in.readAll();

    ELF elf(data);
    if(!elf.is_valid())
        return false;

    bool is_x64 = elf.is_x64();

    in.close();

    QList<QString> *methods = is_x64 ? &methods_x64 : &methods_x86;

    int i = 1;
    foreach(QString method, *methods) {
        LOG_MSG(QString("\nStarting test %1 / %2...").arg(i).arg(methods->length()));
        QString output = QString("%1_%2_%3_%4_%5_%6").arg(input.split('/')[1]).arg(smethods[type]).arg(method.split('.')[0]).arg(handler.split('.')[0]).arg(x ? "x" : "nx").arg(obfuscate ? "obfuscate" : "");
        if(!test_one(input, output, type, method, handler, x, obfuscate, pack))
            return false;

        LOG_MSG(QString("Test %1 / %2 finished.").arg(i).arg(methods->length()));

        ++i;
    }

    return true;
}

bool ELFTester::test_all_handlers(QString input, ELFTester::Method type, QString method, bool x, bool obfuscate, bool pack) {

    QFile in(input);
    if(!in.open(QFile::ReadOnly))
        return false;

    QByteArray data = in.readAll();

    ELF elf(data);
    if(!elf.is_valid())
        return false;

    bool is_x64 = elf.is_x64();

    in.close();

    QList<QString> *handlers = is_x64 ? &handlers_x64 : &handlers_x86;

    int i = 1;
    foreach(QString handler, *handlers) {
        LOG_MSG(QString("\nStarting test %1 / %2...").arg(i).arg(handlers->length()));
        QString output = QString("%1_%2_%3_%4_%5_%6").arg(input.split('/')[1]).arg(smethods[type]).arg(method.split('.')[0]).arg(handler.split('.')[0]).arg(x ? "x" : "nx").arg(obfuscate ? "obfuscate" : "");
        if(!test_one(input, output, type, method, handler, x, obfuscate, pack))
            return false;

        LOG_MSG(QString("Test %1 / %2 finished.").arg(i).arg(handlers->length()));

        ++i;
    }

    return true;

}

bool ELFTester::test_everything_x86(QString input, bool pack) {
    int errors = 0;
    bool ret_val;

    // testing x86
    LOG_MSG("Testing x86 ...");

    foreach (Method type, smethods.keys()) {
        LOG_MSG(QString("Testing methods [%1] ...").arg(smethods[type]));
        foreach (QString m, methods_x86) {
            ret_val = test_all_handlers(input, type, m, false, false, pack);
            if (!ret_val)
                ++errors;
            ret_val = test_all_handlers(input, type, m, true, false, pack);
            if (!ret_val)
                ++errors;
            ret_val = test_all_handlers(input, type, m, false, true, pack);
            if (!ret_val)
                ++errors;
            ret_val = test_all_handlers(input, type, m, true, true, pack);
            if (!ret_val)
                ++errors;
        }
        LOG_MSG(QString("Testing methods [%1] done").arg(smethods[type]));
    }

    foreach (Method type, smethods.keys()) {
        LOG_MSG(QString("Testing methods [%1] ...").arg(smethods[type]));
        foreach (QString h, handlers_x86) {
            ret_val = test_all_methods(input, type, h, false, false, pack);
            if (!ret_val)
                ++errors;
            ret_val = test_all_methods(input, type, h, true, false, pack);
            if (!ret_val)
                ++errors;
            ret_val = test_all_methods(input, type, h, false, true, pack);
            if (!ret_val)
                ++errors;
            ret_val = test_all_methods(input, type, h, true, true, pack);
            if (!ret_val)
                ++errors;
        }
        LOG_MSG(QString("Testing methods [%1] done").arg(smethods[type]));
    }

    LOG_MSG("Testing x86 done");

    return errors == 0;
}

bool ELFTester::test_everything_x64(QString input, bool pack) {
    int errors = 0;
    bool ret_val;

    LOG_MSG("Testing x64 ...");

    foreach (Method type, smethods.keys()) {
        LOG_MSG(QString("Testing methods [%1] ...").arg(smethods[type]));
        foreach (QString m, methods_x64) {
            ret_val = test_all_handlers(input, type, m, false, false, pack);
            if (!ret_val)
                ++errors;
            ret_val = test_all_handlers(input, type, m, true, false, pack);
            if (!ret_val)
                ++errors;
            ret_val = test_all_handlers(input, type, m, false, true, pack);
            if (!ret_val)
                ++errors;
            ret_val = test_all_handlers(input, type, m, true, true, pack);
            if (!ret_val)
                ++errors;
        }
        LOG_MSG(QString("Testing methods [%1] done").arg(smethods[type]));
    }

    foreach (Method type, smethods.keys()) {
        LOG_MSG(QString("Testing methods [%1] ...").arg(smethods[type]));
        foreach (QString h, handlers_x64) {
            ret_val = test_all_methods(input, type, h, false, false, pack);
            if (!ret_val)
                ++errors;
            ret_val = test_all_methods(input, type, h, true, false, pack);
            if (!ret_val)
                ++errors;
            ret_val = test_all_methods(input, type, h, false, true, pack);
            if (!ret_val)
                ++errors;
            ret_val = test_all_methods(input, type, h, true, true, pack);
            if (!ret_val)
                ++errors;
        }
        LOG_MSG(QString("Testing methods [%1] done").arg(smethods[type]));
    }

    LOG_MSG("Testing x64 done");

    return errors == 0;
}

template <typename Reg>
ELFTester::SecuredState ELFTester::test_one_ex(ELF *elf, ELFTester::Method type, QString method,
                                               QString handler, bool x, bool obfuscate)
{
    DJsonParser parser(DSettings::getSettings().getDescriptionsPath<Reg>());

    ELFAddingMethods<Reg> adder(elf);

    Wrapper<Reg> *meth = parser.loadInjectDescription<Reg>(QString("%1.json").arg(method));
    Wrapper<Reg> *wrapper =
            parser.loadInjectDescription<Reg>(QString("%1.json").arg(elf->is_x86() ? wrappers_x86[type] : wrappers_x64[type]));

    if(!meth)
        return SecuredState::PARSINGERROR;

    if (!wrapper)
        return SecuredState::PARSINGERROR;

    // set detection handler
    wrapper->detect_handler = parser.loadInjectDescription<Reg>(QString("%1.json").arg(handler));

    if(!wrapper->detect_handler)
        return SecuredState::PARSINGERROR;

    // set return register for wrapper as return register for method
    wrapper->ret = meth->ret;

    switch (type) {
    case Method::OEP: {
        OEPWrapper<Reg> *oepwrapper =
                dynamic_cast<OEPWrapper<Reg>*>(wrapper);
        if (!oepwrapper)
            return SecuredState::DYNCASTERROR;
        oepwrapper->oep_action = meth;
        break;
    }
    case Method::Thread: {
        ThreadWrapper<Reg> *twrapper =
                dynamic_cast<ThreadWrapper<Reg>*>(wrapper);
        if (!twrapper)
            return SecuredState::DYNCASTERROR;
        twrapper->thread_actions = { meth };
        break;
    }
    case Method::Trampoline:
    case Method::INIT_ARRAY:
    case Method::CTORS :
    case Method::INIT: {
        TrampolineWrapper<Reg> *trmwrapper =
                dynamic_cast<TrampolineWrapper<Reg>*>(wrapper);
        if (!trmwrapper)
            return SecuredState::DYNCASTERROR;
        trmwrapper->tramp_action = meth;
        break;
    }
    default:
        return SecuredState::ERROR;
    }

    typename DAddingMethods<Reg>::InjectDescription id;
    id.adding_method = wrapper;
    id.cm = static_cast<typename DAddingMethods<Reg>::CallingMethod>(type);

    // TODO: check if everything is ok
    if (!meth->allowed_methods.contains(static_cast<typename DAddingMethods<Reg>::CallingMethod>(type)) ||
        !wrapper->detect_handler->allowed_methods.contains(static_cast<typename DAddingMethods<Reg>::CallingMethod>(type)) ||
        (meth->only_rwx & x) || (!meth->obfuscation & obfuscate))
        return SecuredState::NONCOMPATIBLE;


    id.change_x_only = x;
    QList<typename DAddingMethods<Reg>::InjectDescription*> ids = { &id };

    LOG_MSG(QString("secure using wrapper: %1 \n\tmethod: %2\n\thandler: %3").arg(elf->is_x86() ? wrappers_x86[type] : wrappers_x64[type],
                                                                                   method, handler));

    bool s = adder.secure(ids);
    if (obfuscate)
        s &= adder.obfuscate(5, 10, 20);

    if(!meth->detect_handler)
        delete meth->detect_handler;

    delete meth;

    return s ? SecuredState::SECURED : SecuredState::ELFSECERROR;
}
