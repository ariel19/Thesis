#include "test_elf.h"

#include <QFile>
#include <QDir>
#include <QVariant>
#include <QDebug>
#include <core/file_types/elffile.h>
#include <core/adding_methods/wrappers/elfaddingmethods.h>


bool test_trampoline_wrappers(const QString &elf_fname, const QString &wrapper,
                              const QString &method, const QString &handl) {
    // DAddingMethods::InjectDescription inject_desc;
    QFile f(elf_fname);
    if(!f.open(QFile::ReadOnly)) {
        qDebug() << "Error opening file: " << elf_fname;
        return false;
    }
    ELF elf(f.readAll());
    // qDebug() << "valid: " << elf.is_valid();
    if (!elf.is_valid())
        return false;
    QFile code;
    if (elf.is_x86()) {
        ELFAddingMethods<Registers_x86> dam(&elf);
        DAddingMethods<Registers_x86>::InjectDescription inject_desc;
        DAddingMethods<Registers_x86>::TrampolineWrapper oepwrapper;
        DAddingMethods<Registers_x86>::Wrapper handler;
        DAddingMethods<Registers_x86>::Wrapper oepaction;
        // debugger detection handler
        code.setFileName(handl);
        if (!code.open(QIODevice::ReadOnly))
            return false;
        handler.static_params = { { "ret", "127" } };
        handler.detect_handler = nullptr;
        handler.code = code.readAll();
        // qDebug() << handler.code;
        code.close();
        // debugger detection method
        code.setFileName(method);
        if (!code.open(QIODevice::ReadOnly))
            return false;
        oepaction.code = code.readAll();
        // for cc code
        QPair<QByteArray, Elf64_Addr> section_data;
        if (!elf.get_section_content(ELF::SectionType::TEXT, section_data))
            return false;

        // FXIME: FOR CC METHOD
        oepaction.static_params = { { "vsize", QString::number(section_data.first.size()) },
                                    { "vaddr", QString::number(section_data.second) } };
        oepaction.detect_handler = nullptr;
        oepaction.ret = Registers_x86::EAX;
        oepaction.used_regs = { Registers_x86::EAX, Registers_x86::ECX,
                                Registers_x86::EBX, Registers_x86::EDX,
                                Registers_x86::ESI };
        // qDebug() << oepaction.code;
        code.close();
        // wrapper like OEP or thread
        code.setFileName(wrapper);
        if (!code.open(QIODevice::ReadOnly))
            return false;
        oepwrapper.detect_handler = &handler;
        oepwrapper.tramp_action = &oepaction;
        oepwrapper.code = code.readAll();
        oepwrapper.used_regs = { Registers_x86::EDI };
        oepwrapper.ret = oepaction.ret;
        // qDebug() << oepwrapper.code;
        code.close();
        inject_desc.cm = DAddingMethods<Registers_x86>::CallingMethod::Trampoline;
        inject_desc.adding_method = &oepwrapper;
        inject_desc.saved_fname = elf_fname + QString("_tram_sig");

        if (!dam.secure({&inject_desc}))
            return false;
    }
    else {
        ELFAddingMethods<Registers_x64> dam(&elf);
        DAddingMethods<Registers_x64>::InjectDescription inject_desc;
        inject_desc.cm = DAddingMethods<Registers_x64>::CallingMethod::OEP;
        DAddingMethods<Registers_x64>::TrampolineWrapper oepwrapper;
        DAddingMethods<Registers_x64>::Wrapper handler;
        DAddingMethods<Registers_x64>::Wrapper oepaction;
        // debugger detection handler
        code.setFileName(handl);
        if (!code.open(QIODevice::ReadOnly))
            return false;
        handler.static_params = { { "ret", "188" } };
        handler.detect_handler = nullptr;
        handler.code = code.readAll();
        // qDebug() << handler.code;
        code.close();
        // debugger detection method
        code.setFileName(method);
        if (!code.open(QIODevice::ReadOnly))
            return false;
        oepaction.code = code.readAll();

        // for cc code
        QPair<QByteArray, Elf64_Addr> section_data;
        if (!elf.get_section_content(ELF::SectionType::TEXT, section_data))
            return false;

        // FXIME: FOR CC METHOD
        oepaction.static_params = { { "vsize", QString::number(section_data.first.size()) },
                                    { "vaddr", QString::number(section_data.second) } };

        oepaction.detect_handler = nullptr;
        oepaction.ret = Registers_x64::RAX;
        oepaction.used_regs = { Registers_x64::RAX, Registers_x64::RDI, Registers_x64::RCX,
                                Registers_x64::RSI, Registers_x64::RDX, Registers_x64::RBX,
                                Registers_x64::RBP, Registers_x64::R10,
                                Registers_x64::R15, Registers_x64::R14 };
        // qDebug() << oepaction.code;
        code.close();
        // wrapper like OEP or thread
        code.setFileName(wrapper);
        if (!code.open(QIODevice::ReadOnly))
            return false;
        oepwrapper.detect_handler = &handler;
        oepwrapper.tramp_action = &oepaction;
        oepwrapper.code = code.readAll();
        oepwrapper.used_regs = { Registers_x64::R11, Registers_x64::R12 };
        oepwrapper.ret = oepaction.ret;
        // qDebug() << oepwrapper.code;
        code.close();
        inject_desc.cm = DAddingMethods<Registers_x64>::CallingMethod::Trampoline;
        inject_desc.adding_method = &oepwrapper;
        inject_desc.saved_fname = elf_fname + QString("_tram_sig");

        if (!dam.secure({&inject_desc}))
            return false;
    }
    // rename file on hard drive
    // QFile::rename("template", QString("_%1o").arg(elf_fname));
    // qDebug() << "saving to file: " << QString("_%1o").arg(elf_fname);
    return true;
}

bool test_oep_wrappers(const QString &elf_fname, const QString &wrapper,
                       const QString &method, const QString &handl) {
    // DAddingMethods::InjectDescription inject_desc;
    QFile f(elf_fname);
    if(!f.open(QFile::ReadOnly)) {
        qDebug() << "Error opening file: " << elf_fname;
        return false;
    }
    ELF elf(f.readAll());
    // qDebug() << "valid: " << elf.is_valid();
    if (!elf.is_valid())
        return false;
    QFile code;
    if (elf.is_x86()) {
        ELFAddingMethods<Registers_x86> dam(&elf);
        DAddingMethods<Registers_x86>::InjectDescription inject_desc;
        DAddingMethods<Registers_x86>::OEPWrapper oepwrapper;
        DAddingMethods<Registers_x86>::Wrapper handler;
        DAddingMethods<Registers_x86>::Wrapper oepaction;
        // debugger detection handler
        code.setFileName(handl);
        if (!code.open(QIODevice::ReadOnly))
            return false;
        handler.static_params = { { "ret", "127" } };
        handler.detect_handler = nullptr;
        handler.code = code.readAll();
        // qDebug() << handler.code;
        code.close();
        // debugger detection method
        code.setFileName(method);
        if (!code.open(QIODevice::ReadOnly))
            return false;
        oepaction.code = code.readAll();
        // for cc code
        QPair<QByteArray, Elf64_Addr> section_data;
        if (!elf.get_section_content(ELF::SectionType::TEXT, section_data))
            return false;

        // FXIME: FOR CC METHOD
        oepaction.static_params = { { "vsize", QString::number(section_data.first.size()) },
                                    { "vaddr", QString::number(section_data.second) } };
        oepaction.detect_handler = nullptr;
        oepaction.ret = Registers_x86::EAX;
        oepaction.used_regs = { Registers_x86::EAX, Registers_x86::ECX,
                                Registers_x86::EBX, Registers_x86::EDX,
                                Registers_x86::ESI };
        // qDebug() << oepaction.code;
        code.close();
        // wrapper like OEP or thread
        code.setFileName(wrapper);
        if (!code.open(QIODevice::ReadOnly))
            return false;
        oepwrapper.detect_handler = &handler;
        oepwrapper.oep_action = &oepaction;
        oepwrapper.code = code.readAll();
        oepwrapper.used_regs = { Registers_x86::EDI };
        oepwrapper.ret = oepaction.ret;
        // qDebug() << oepwrapper.code;
        code.close();
        inject_desc.cm = DAddingMethods<Registers_x86>::CallingMethod::OEP;
        //inject_desc.cm = DAddingMethods::CallingMethod::Trampoline;
        inject_desc.adding_method = &oepwrapper;
        inject_desc.saved_fname = elf_fname + QString("_sig_cc");

        if (!dam.secure({&inject_desc}))
            return false;
    }
    else {
        ELFAddingMethods<Registers_x64> dam(&elf);
        DAddingMethods<Registers_x64>::InjectDescription inject_desc;
        inject_desc.cm = DAddingMethods<Registers_x64>::CallingMethod::OEP;
        DAddingMethods<Registers_x64>::OEPWrapper oepwrapper;
        DAddingMethods<Registers_x64>::Wrapper handler;
        DAddingMethods<Registers_x64>::Wrapper oepaction;
        // debugger detection handler
        code.setFileName(handl);
        if (!code.open(QIODevice::ReadOnly))
            return false;
        handler.static_params = { { "ret", "188" } };
        handler.detect_handler = nullptr;
        handler.code = code.readAll();
        // qDebug() << handler.code;
        code.close();
        // debugger detection method
        code.setFileName(method);
        if (!code.open(QIODevice::ReadOnly))
            return false;
        oepaction.code = code.readAll();

        // for cc code
        QPair<QByteArray, Elf64_Addr> section_data;
        if (!elf.get_section_content(ELF::SectionType::TEXT, section_data))
            return false;

        // FXIME: FOR CC METHOD
        oepaction.static_params = { { "vsize", QString::number(section_data.first.size()) },
                                    { "vaddr", QString::number(section_data.second) } };

        oepaction.detect_handler = nullptr;
        oepaction.ret = Registers_x64::RAX;
        oepaction.used_regs = { Registers_x64::RAX, Registers_x64::RDI, Registers_x64::RCX,
                                Registers_x64::RSI, Registers_x64::RDX, Registers_x64::RBX,
                                Registers_x64::RBP, Registers_x64::R10,
                                Registers_x64::R15, Registers_x64::R14 };
        // qDebug() << oepaction.code;
        code.close();
        // wrapper like OEP or thread
        code.setFileName(wrapper);
        if (!code.open(QIODevice::ReadOnly))
            return false;
        oepwrapper.detect_handler = &handler;
        oepwrapper.oep_action = &oepaction;
        oepwrapper.code = code.readAll();
        oepwrapper.used_regs = { Registers_x64::R11, Registers_x64::R12 };
        oepwrapper.ret = oepaction.ret;
        // qDebug() << oepwrapper.code;
        code.close();
        inject_desc.cm = DAddingMethods<Registers_x64>::CallingMethod::OEP;
        // inject_desc.cm = DAddingMethods::CallingMethod::Trampoline;
        inject_desc.adding_method = &oepwrapper;
        inject_desc.saved_fname = elf_fname + QString("_sig_cc");

        if (!dam.secure({&inject_desc}))
            return false;
    }
    // rename file on hard drive
    // QFile::rename("template", QString("_%1o").arg(elf_fname));
    // qDebug() << "saving to file: " << QString("_%1o").arg(elf_fname);
    return true;
}

bool test_thread_wrappers(const QString &elf_fname, const QString &wrapper,
                          const QString &method, const QString &handl) {
    QFile f(elf_fname);
    if(!f.open(QFile::ReadOnly)) {
        qDebug() << "Error opening file: " << elf_fname;
        return false;
    }
    ELF elf(f.readAll());
    // qDebug() << "valid: " << elf.is_valid();
    if (!elf.is_valid())
        return false;
    QFile code;
    if (elf.is_x86()) {
        ELFAddingMethods<Registers_x86> dam(&elf);
        DAddingMethods<Registers_x86>::InjectDescription inject_desc;
        DAddingMethods<Registers_x86>::ThreadWrapper twrapper;
        DAddingMethods<Registers_x86>::Wrapper handler;
        DAddingMethods<Registers_x86>::Wrapper taction;
        // debugger detection handler
        code.setFileName(handl);
        if (!code.open(QIODevice::ReadOnly))
            return false;
        handler.static_params = { { "ret", "127" } };
        handler.detect_handler = nullptr;
        handler.code = code.readAll();
        // qDebug() << handler.code;
        code.close();
        // debugger detection method
        code.setFileName(method);
        if (!code.open(QIODevice::ReadOnly))
            return false;
        taction.code = code.readAll();
        taction.detect_handler = nullptr;
        taction.ret = Registers_x86::EAX;
        taction.used_regs = { Registers_x86::EAX, Registers_x86::ECX,
                              Registers_x86::EBX, Registers_x86::EDX,
                              Registers_x86::ESI, Registers_x86::EDI };
        // qDebug() << taction.code;
        code.close();
        // wrapper like OEP or thread
        code.setFileName(wrapper);
        if (!code.open(QIODevice::ReadOnly))
            return false;
        twrapper.detect_handler = &handler;
        twrapper.thread_actions = { &taction };
        twrapper.code = code.readAll();
        twrapper.used_regs = { Registers_x86::EAX, Registers_x86::ECX,
                               Registers_x86::EBX, Registers_x86::EDX,
                               Registers_x86::ESI, Registers_x86::EDI };
        twrapper.static_params = { { "sleep1", "0" },
                                   { "sleep2", "5" } };
        twrapper.ret = taction.ret;
        // qDebug() << twrapper.code;
        code.close();
        inject_desc.cm = DAddingMethods<Registers_x86>::CallingMethod::Thread;
        inject_desc.adding_method = &twrapper;
        inject_desc.saved_fname = elf_fname + QString("_thread_sig");

        if (!dam.secure({&inject_desc}))
            return false;
    }
    else {
        ELFAddingMethods<Registers_x64> dam(&elf);
        DAddingMethods<Registers_x64>::InjectDescription inject_desc;
        inject_desc.cm = DAddingMethods<Registers_x64>::CallingMethod::OEP;
        DAddingMethods<Registers_x64>::ThreadWrapper twrapper;
        DAddingMethods<Registers_x64>::Wrapper handler;
        DAddingMethods<Registers_x64>::Wrapper taction;
        // debugger detection handler
        code.setFileName(handl);
        if (!code.open(QIODevice::ReadOnly))
            return false;
        handler.static_params = { { "ret", "188" } };
        handler.detect_handler = nullptr;
        handler.code = code.readAll();
        // qDebug() << handler.code;
        code.close();
        // debugger detection method
        code.setFileName(method);
        if (!code.open(QIODevice::ReadOnly))
            return false;
        taction.code = code.readAll();
        taction.detect_handler = nullptr;
        taction.ret = Registers_x64::RAX;
        taction.used_regs = { Registers_x64::RAX, Registers_x64::RDI,
                              Registers_x64::RSI, Registers_x64::RDX,
                              Registers_x64::R10 };
        // qDebug() << taction.code;
        code.close();
        // wrapper like OEP or thread
        code.setFileName(wrapper);
        if (!code.open(QIODevice::ReadOnly))
            return false;
        twrapper.detect_handler = &handler;
        twrapper.thread_actions = { &taction };
        twrapper.code = code.readAll();
        twrapper.used_regs = { Registers_x64::RAX, Registers_x64::RDI,
                               Registers_x64::RSI, Registers_x64::RDX,
                               Registers_x64::R10, Registers_x64::R8 };
        twrapper.static_params = { { "sleep1", "0" },
                            { "sleep2", "5" } };
        twrapper.ret = taction.ret;
        // qDebug() << twrapper.code;
        code.close();
        inject_desc.cm = DAddingMethods<Registers_x64>::CallingMethod::Thread;
        inject_desc.adding_method = &twrapper;
        inject_desc.saved_fname = elf_fname + QString("_thread_sig");

        if (!dam.secure({&inject_desc}))
            return false;
    }
    return true;
}

bool test_init_oep_wrappers(const QString &elf_fname, const QString &wrapper,
                            const QString &method, const QString &handl) {
    // DAddingMethods::InjectDescription inject_desc;
    QFile f(elf_fname);
    if(!f.open(QFile::ReadOnly)) {
        qDebug() << "Error opening file: " << elf_fname;
        return false;
    }
    ELF elf(f.readAll());
    // qDebug() << "valid: " << elf.is_valid();
    if (!elf.is_valid())
        return false;
    QFile code;
    if (elf.is_x86()) {
        ELFAddingMethods<Registers_x86> dam(&elf);
        DAddingMethods<Registers_x86>::InjectDescription inject_desc;
        DAddingMethods<Registers_x86>::TrampolineWrapper trmwrapper;
        DAddingMethods<Registers_x86>::Wrapper handler;
        DAddingMethods<Registers_x86>::Wrapper trmaction;
        // debugger detection handler
        code.setFileName(handl);
        if (!code.open(QIODevice::ReadOnly))
            return false;
        handler.static_params = { { "ret", "127" } };
        handler.detect_handler = nullptr;
        handler.code = code.readAll();
        // qDebug() << handler.code;
        code.close();
        // debugger detection method
        code.setFileName(method);
        if (!code.open(QIODevice::ReadOnly))
            return false;
        trmaction.code = code.readAll();
        trmaction.detect_handler = nullptr;
        trmaction.ret = Registers_x86::EAX;
        trmaction.used_regs = { Registers_x86::EAX, Registers_x86::ECX,
                                Registers_x86::EBX, Registers_x86::EDX,
                                Registers_x86::ESI };
        // qDebug() << oepaction.code;
        code.close();
        // wrapper like OEP or thread
        code.setFileName(wrapper);
        if (!code.open(QIODevice::ReadOnly))
            return false;
        trmwrapper.detect_handler = &handler;
        trmwrapper.tramp_action = &trmaction;
        trmwrapper.code = code.readAll();
        trmwrapper.used_regs = { Registers_x86::EDI };

        // FIXME: only fo threads
        trmwrapper.used_regs = { Registers_x86::EAX, Registers_x86::ECX,
                               Registers_x86::EBX, Registers_x86::EDX,
                               Registers_x86::ESI, Registers_x86::EDI };
        trmwrapper.static_params = { { "sleep1", "0" },
                              { "sleep2", "5" } };
        trmwrapper.ret = trmaction.ret;

        // qDebug() << oepwrapper.code;
        code.close();
        inject_desc.cm = DAddingMethods<Registers_x86>::CallingMethod::INIT;
        inject_desc.adding_method = &trmwrapper;
        inject_desc.saved_fname = elf_fname + QString("_init_ptrace");

        if (!dam.secure({&inject_desc}))
            return false;
    }
    else {
        ELFAddingMethods<Registers_x64> dam(&elf);
        DAddingMethods<Registers_x64>::InjectDescription inject_desc;
        DAddingMethods<Registers_x64>::TrampolineWrapper trmwrapper;
        DAddingMethods<Registers_x64>::Wrapper handler;
        DAddingMethods<Registers_x64>::Wrapper trmaction;
        // debugger detection handler
        code.setFileName(handl);
        if (!code.open(QIODevice::ReadOnly))
            return false;
        handler.static_params = { { "ret", "188" } };
        handler.detect_handler = nullptr;
        handler.code = code.readAll();
        // qDebug() << handler.code;
        code.close();
        // debugger detection method
        code.setFileName(method);
        if (!code.open(QIODevice::ReadOnly))
            return false;
        trmaction.code = code.readAll();
        trmaction.detect_handler = nullptr;
        trmaction.ret = Registers_x64::RAX;
        trmaction.used_regs = { Registers_x64::RAX, Registers_x64::RDI,
                                Registers_x64::RSI, Registers_x64::RDX,
                                Registers_x64::R10 };
        // qDebug() << oepaction.code;
        code.close();
        // wrapper like OEP or thread
        code.setFileName(wrapper);
        if (!code.open(QIODevice::ReadOnly))
            return false;
        trmwrapper.detect_handler = &handler;
        trmwrapper.tramp_action = &trmaction;
        trmwrapper.code = code.readAll();
        trmwrapper.used_regs = { Registers_x64::R11, Registers_x64::R12 };

        // FIXME: only fo threads
        trmwrapper.used_regs = { Registers_x64::RAX, Registers_x64::RDI,
                               Registers_x64::RSI, Registers_x64::RDX,
                               Registers_x64::R10, Registers_x64::R8 };

        trmwrapper.static_params = { { "sleep1", "0" },
                              { "sleep2", "5" } };
        trmwrapper.ret = trmaction.ret;

        // qDebug() << oepwrapper.code;
        code.close();
        inject_desc.cm = DAddingMethods<Registers_x64>::CallingMethod::INIT;
        inject_desc.adding_method = &trmwrapper;
        inject_desc.saved_fname = elf_fname + QString("_init_ptrace");


        if (!dam.secure({&inject_desc}))
            return false;
    }
    return true;
}

bool test_initarray_oep_wrappers(const QString &elf_fname, const QString &wrapper,
                                 const QString &method, const QString &handl) {
    // DAddingMethods::InjectDescription inject_desc;
    QFile f(elf_fname);
    if(!f.open(QFile::ReadOnly)) {
        qDebug() << "Error opening file: " << elf_fname;
        return false;
    }
    ELF elf(f.readAll());
    // qDebug() << "valid: " << elf.is_valid();
    if (!elf.is_valid())
        return false;
    QFile code;
    if (elf.is_x86()) {
        ELFAddingMethods<Registers_x86> dam(&elf);
        DAddingMethods<Registers_x86>::InjectDescription inject_desc;
        DAddingMethods<Registers_x86>::TrampolineWrapper trmwrapper;
        DAddingMethods<Registers_x86>::Wrapper handler;
        DAddingMethods<Registers_x86>::Wrapper trmaction;
        // debugger detection handler
        code.setFileName(handl);
        if (!code.open(QIODevice::ReadOnly))
            return false;
        handler.static_params = { { "ret", "127" } };
        handler.detect_handler = nullptr;
        handler.code = code.readAll();
        // qDebug() << handler.code;
        code.close();
        // debugger detection method
        code.setFileName(method);
        if (!code.open(QIODevice::ReadOnly))
            return false;
        trmaction.code = code.readAll();
        trmaction.detect_handler = nullptr;
        trmaction.ret = Registers_x86::EAX;
        trmaction.used_regs = { Registers_x86::EAX, Registers_x86::ECX,
                                Registers_x86::EBX, Registers_x86::EDX,
                                Registers_x86::ESI };
        // qDebug() << oepaction.code;
        code.close();
        // wrapper like OEP or thread
        code.setFileName(wrapper);
        if (!code.open(QIODevice::ReadOnly))
            return false;
        trmwrapper.detect_handler = &handler;
        trmwrapper.tramp_action = &trmaction;
        trmwrapper.code = code.readAll();
        trmwrapper.used_regs = { Registers_x86::EDI };
        // qDebug() << oepwrapper.code;

        // FIXME: only fo threads
        trmwrapper.used_regs = { Registers_x86::EAX, Registers_x86::ECX,
                                 Registers_x86::EBX, Registers_x86::EDX,
                                 Registers_x86::ESI, Registers_x86::EDI };
        trmwrapper.static_params = { { "sleep1", "0" },
                              { "sleep2", "5" } };

        trmwrapper.ret = trmaction.ret;
        inject_desc.saved_fname = elf_fname + QString("_initarray_ptrace");

        code.close();
        inject_desc.cm = DAddingMethods<Registers_x86>::CallingMethod::INIT_ARRAY;
        inject_desc.adding_method = &trmwrapper;
        if (!dam.secure({&inject_desc}))
            return false;
    }
    else {
        ELFAddingMethods<Registers_x64> dam(&elf);
        DAddingMethods<Registers_x64>::InjectDescription inject_desc;
        DAddingMethods<Registers_x64>::TrampolineWrapper trmwrapper;
        DAddingMethods<Registers_x64>::Wrapper handler;
        DAddingMethods<Registers_x64>::Wrapper trmaction;
        // debugger detection handler
        code.setFileName(handl);
        if (!code.open(QIODevice::ReadOnly))
            return false;
        handler.static_params = { { "ret", "188" } };
        handler.detect_handler = nullptr;
        handler.code = code.readAll();
        // qDebug() << handler.code;
        code.close();
        // debugger detection method
        code.setFileName(method);
        if (!code.open(QIODevice::ReadOnly))
            return false;
        trmaction.code = code.readAll();
        trmaction.detect_handler = nullptr;
        trmaction.ret = Registers_x64::RAX;
        trmaction.used_regs = { Registers_x64::RAX, Registers_x64::RDI,
                                Registers_x64::RSI, Registers_x64::RDX,
                                Registers_x64::R10 };
        // qDebug() << oepaction.code;
        code.close();
        // wrapper like OEP or thread
        code.setFileName(wrapper);
        if (!code.open(QIODevice::ReadOnly))
            return false;
        trmwrapper.detect_handler = &handler;
        trmwrapper.tramp_action = &trmaction;
        trmwrapper.code = code.readAll();
        trmwrapper.used_regs = { Registers_x64::R11, Registers_x64::R12 };

        // FIXME: only fo threads
        trmwrapper.used_regs = { Registers_x64::RAX, Registers_x64::RDI,
                               Registers_x64::RSI, Registers_x64::RDX,
                               Registers_x64::R10, Registers_x64::R8 };
        trmwrapper.static_params = { { "sleep1", "0" },
                            { "sleep2", "5" } };

        trmwrapper.ret = trmaction.ret;
        inject_desc.saved_fname = elf_fname + QString("_initarray_ptrace");

        // qDebug() << oepwrapper.code;
        code.close();
        inject_desc.cm = DAddingMethods<Registers_x64>::CallingMethod::INIT_ARRAY;
        inject_desc.adding_method = &trmwrapper;
        if (!dam.secure({&inject_desc}))
            return false;
    }
    // rename file on hard drive
    QFile::rename("template", QString("_%1oinita").arg(elf_fname));
    qDebug() << "saving to file: " << QString("_%1oinita").arg(elf_fname);
    return true;
}


bool test_ctors_oep_wrappers(const QString &elf_fname, const QString &wrapper,
                             const QString &method, const QString &handl) {
    // DAddingMethods::InjectDescription inject_desc;
    QFile f(elf_fname);
    if(!f.open(QFile::ReadOnly)) {
        qDebug() << "Error opening file: " << elf_fname;
        return false;
    }
    ELF elf(f.readAll());
    // qDebug() << "valid: " << elf.is_valid();
    if (!elf.is_valid())
        return false;
    QFile code;
    if (elf.is_x86()) {
        ELFAddingMethods<Registers_x86> dam(&elf);
        DAddingMethods<Registers_x86>::InjectDescription inject_desc;
        DAddingMethods<Registers_x86>::TrampolineWrapper trmwrapper;
        DAddingMethods<Registers_x86>::Wrapper handler;
        DAddingMethods<Registers_x86>::Wrapper trmaction;
        // debugger detection handler
        code.setFileName(handl);
        if (!code.open(QIODevice::ReadOnly))
            return false;
        handler.static_params = { { "ret", "127" } };
        handler.detect_handler = nullptr;
        handler.code = code.readAll();
        // qDebug() << handler.code;
        code.close();
        // debugger detection method
        code.setFileName(method);
        if (!code.open(QIODevice::ReadOnly))
            return false;
        trmaction.code = code.readAll();
        trmaction.detect_handler = nullptr;
        trmaction.ret = Registers_x86::EAX;
        trmaction.used_regs = { Registers_x86::EAX, Registers_x86::ECX,
                                Registers_x86::EBX, Registers_x86::EDX,
                                Registers_x86::ESI };
        // qDebug() << oepaction.code;
        code.close();
        // wrapper like OEP or thread
        code.setFileName(wrapper);
        if (!code.open(QIODevice::ReadOnly))
            return false;
        trmwrapper.detect_handler = &handler;
        trmwrapper.tramp_action = &trmaction;
        trmwrapper.code = code.readAll();
        trmwrapper.used_regs = { Registers_x86::EDI };

        trmwrapper.ret = trmaction.ret;
        // qDebug() << oepwrapper.code;

        code.close();
        inject_desc.cm = DAddingMethods<Registers_x86>::CallingMethod::CTORS;
        inject_desc.adding_method = &trmwrapper;
        if (!dam.secure({&inject_desc}))
            return false;
    }
    else {
        ELFAddingMethods<Registers_x64> dam(&elf);
        DAddingMethods<Registers_x64>::InjectDescription inject_desc;
        DAddingMethods<Registers_x64>::TrampolineWrapper trmwrapper;
        DAddingMethods<Registers_x64>::Wrapper handler;
        DAddingMethods<Registers_x64>::Wrapper trmaction;
        // debugger detection handler
        code.setFileName(handl);
        if (!code.open(QIODevice::ReadOnly))
            return false;
        handler.static_params = { { "ret", "188" } };
        handler.detect_handler = nullptr;
        handler.code = code.readAll();
        // qDebug() << handler.code;
        code.close();
        // debugger detection method
        code.setFileName(method);
        if (!code.open(QIODevice::ReadOnly))
            return false;
        trmaction.code = code.readAll();
        trmaction.detect_handler = nullptr;
        trmaction.ret = Registers_x64::RAX;
        trmaction.used_regs = { Registers_x64::RAX, Registers_x64::RDI,
                                Registers_x64::RSI, Registers_x64::RDX,
                                Registers_x64::R10 };
        // qDebug() << oepaction.code;
        code.close();
        // wrapper like OEP or thread
        code.setFileName(wrapper);
        if (!code.open(QIODevice::ReadOnly))
            return false;
        trmwrapper.detect_handler = &handler;
        trmwrapper.tramp_action = &trmaction;
        trmwrapper.code = code.readAll();
        trmwrapper.used_regs = { Registers_x64::R11, Registers_x64::R12 };

        trmwrapper.ret = trmaction.ret;
        // qDebug() << oepwrapper.code;
        code.close();
        inject_desc.cm = DAddingMethods<Registers_x64>::CallingMethod::CTORS;
        inject_desc.adding_method = &trmwrapper;
        if (!dam.secure({&inject_desc}))
            return false;
    }
    // rename file on hard drive
    QFile::rename("template", QString("_%1octors").arg(elf_fname));
    qDebug() << "saving to file: " << QString("_%1octors").arg(elf_fname);
    return true;
}

void test_wrappers() {
    /*
    qDebug() << "=========================================";
    qDebug() << "Testing OEP + ptrace for my 32-bit app...";
    // test oep + ptrace
    if (!test_oep_wrappers("my32", "oep_t.asm", "ptrace_t.asm", "exit_t.asm")) {
        qDebug() << "something went wrong :(";
    }
    qDebug() << "Testing OEP + ptrace for my 32-bit app done";
    qDebug() << "=========================================";
    */
    /*
    qDebug() << "Testing OEP + ptrace for my 64-bit app...";
    if (!test_oep_wrappers("my64", "oep64_t.asm", "ptrace64_t.asm", "exit64_t.asm")) {
        qDebug() << "something went wrong :(";
    }
    qDebug() << "Testing OEP + ptrace for my 64-bit app done";
    qDebug() << "=========================================";
    qDebug() << "Testing OEP + ptrace for derby 32-bit app...";
    // test oep + ptrace
    if (!test_oep_wrappers("derby32", "oep_t.asm", "ptrace_t.asm", "exit_t.asm")) {
        qDebug() << "something went wrong :(";
    }
    qDebug() << "Testing OEP + ptrace for derby 32-bit app done";
    qDebug() << "=========================================";
    qDebug() << "Testing OEP + ptrace for derby 64-bit app...";
    if (!test_oep_wrappers("derby64", "oep64_t.asm", "ptrace64_t.asm", "exit64_t.asm")) {
        qDebug() << "something went wrong :(";
    }
    qDebug() << "Testing OEP + ptrace for derby 64-bit app done";
    qDebug() << "=========================================";
    qDebug() << "Testing OEP + ptrace for edb 64-bit app...";
    if (!test_oep_wrappers("edb", "oep64_t.asm", "ptrace64_t.asm", "exit64_t.asm")) {
        qDebug() << "something went wrong :(";
    }
    qDebug() << "Testing OEP + ptrace for edb 64-bit app done";
    qDebug() << "=========================================";
    qDebug() << "Testing OEP + ptrace for dDeflect 64-bit app...";
    if (!test_oep_wrappers("dDeflect", "oep64_t.asm", "ptrace64_t.asm", "exit64_t.asm")) {
        qDebug() << "something went wrong :(";
    }
    qDebug() << "Testing OEP + ptrace for dDeflect 64-bit app done";
    qDebug() << "=========================================";
    qDebug() << "Testing OEP + thread for my 32-bit app...";
    // test oep + ptrace
    if (!test_thread_wrappers("my32", "threadp_t.asm", "ptrace_t.asm", "exit_group_t.asm")) {
        qDebug() << "something went wrong :(";
    }
    qDebug() << "Testing OEP + thread for my 32-bit app done";
    qDebug() << "=========================================";
    qDebug() << "Testing OEP + thread for my 64-bit app...";
    if (!test_thread_wrappers("my64", "thread64p_t.asm", "ptrace64_t.asm", "exit_group64_t.asm")) {
        qDebug() << "something went wrong :(";
    }
    qDebug() << "Testing OEP + thread for my 64-bit app done";
    qDebug() << "=========================================";
    qDebug() << "Testing OEP + thread for derby 32-bit app...";
    // test oep + ptrace
    if (!test_thread_wrappers("derby32", "threadp_t.asm", "ptrace_t.asm", "exit_group_t.asm")) {
        qDebug() << "something went wrong :(";
    }
    qDebug() << "Testing OEP + thread for derby 32-bit app done";
    qDebug() << "=========================================";
    qDebug() << "Testing OEP + thread for derby 64-bit app...";
    if (!test_thread_wrappers("derby64", "thread64p_t.asm", "ptrace64_t.asm", "exit_group64_t.asm")) {
        qDebug() << "something went wrong :(";
    }
    qDebug() << "Testing OEP + thread for derby 64-bit app done";
    qDebug() << "=========================================";
    qDebug() << "Testing OEP + thread for edb 64-bit app...";
    if (!test_thread_wrappers("edb", "thread64p_t.asm", "ptrace64_t.asm", "exit_group64_t.asm")) {
        qDebug() << "something went wrong :(";
    }
    qDebug() << "Testing OEP + thread for edb 64-bit app done";
    qDebug() << "=========================================";
    qDebug() << "Testing OEP + thread for dDeflect 64-bit app...";
    if (!test_thread_wrappers("dDeflect", "thread64p_t.asm", "ptrace64_t.asm", "exit_group64_t.asm")) {
        qDebug() << "something went wrong :(";
    }
    qDebug() << "Testing OEP + thread for dDeflect 64-bit app done";
    */


    // test oep + ptrace
    if (!test_thread_wrappers("bin/derby32", "thread.asm", "methods/sigtrap.asm", "handlers/exit_group.asm")) {
        qDebug() << "something went wrong :(";
    }

    // test oep + ptrace
    if (!test_thread_wrappers("bin/my32", "thread.asm", "methods/sigtrap.asm", "handlers/exit_group.asm")) {
        qDebug() << "something went wrong :(";
    }

    /*
    // test oep + ptrace
    if (!test_thread_wrappers("bin/my64", "thread64.asm", "methods/ptrace64.asm", "handlers/exit_group64.asm")) {
        qDebug() << "something went wrong :(";
    }

    // test oep + ptrace
    if (!test_thread_wrappers("bin/derby64", "thread64.asm", "methods/ptrace64.asm", "handlers/exit_group64.asm")) {
        qDebug() << "something went wrong :(";
    }

    // test oep + ptrace
    if (!test_thread_wrappers("bin/edb", "thread64.asm", "methods/ptrace64.asm", "handlers/exit_group64.asm")) {
        qDebug() << "something went wrong :(";
    }

    // test oep + ptrace
    if (!test_thread_wrappers("bin/dDeflect", "thread64.asm", "methods/ptrace64.asm", "handlers/exit_group64.asm")) {
        qDebug() << "something went wrong :(";
    }
    */




    /*
    qDebug() << "=========================================";
    qDebug() << "Testing OEP + ptrace for my 32-bit app...";
    // test oep + ptrace
    if (!test_oep_wrappers("my32", "oep_t.asm", "ptrace_t.asm", "exit_t.asm")) {
        qDebug() << "something went wrong :(";
    }
    qDebug() << "Testing OEP + ptrace for my 32-bit app done";
    qDebug() << "=========================================";
    */

    /*
    qDebug() << "Testing OEP + ptrace for my 64-bit app...";
    if (!test_oep_wrappers("my64", "oep64_t.asm", "cc64.asm", "exit64_t.asm")) {
        qDebug() << "something went wrong :(";
    }
    qDebug() << "Testing OEP + ptrace for my 64-bit app done";
    qDebug() << "=========================================";
    */


}

void test_methods() {
    // QList<QString> dmeth_x86 = { "cc.asm", "ptrace.asm" };
}


#if 0
void test_wrappers() {
    /*
    qDebug() << "=========================================";
    qDebug() << "Testing OEP + ctors for ctors 32-bit app...";
    // test oep + ptrace
    if (!test_ctors_oep_wrappers("ctors32", "oep_t.asm", "ptrace_t.asm", "exit_t.asm")) {
        qDebug() << "something went wrong :(";
    }
    qDebug() << "Testing OEP + ctors for my 32-bit ctors done";
    qDebug() << "=========================================";
    qDebug() << "Testing OEP + ctors for my 64-bit ctors...";
    if (!test_ctors_oep_wrappers("ctors64", "oep64_t.asm", "ptrace64_t.asm", "exit64_t.asm")) {
        qDebug() << "something went wrong :(";
    }
    qDebug() << "Testing OEP + ctors for my 64-bit ctors done";

    qDebug() << "=========================================";
    qDebug() << "Testing OEP + ctors for my 32-bit app...";
    // test oep + ptrace
    if (!test_ctors_oep_wrappers("my32", "oep_t.asm", "ptrace_t.asm", "exit_t.asm")) {
        qDebug() << "something went wrong :(";
    }
    qDebug() << "Testing OEP + ctors for my 32-bit app done";
    qDebug() << "=========================================";
    qDebug() << "Testing OEP + ctors for my 64-bit app...";
    if (!test_ctors_oep_wrappers("my64", "oep64_t.asm", "ptrace64_t.asm", "exit64_t.asm")) {
        qDebug() << "something went wrong :(";
    }
    qDebug() << "Testing OEP + ctors for my 64-bit app done";
    qDebug() << "=========================================";
    qDebug() << "Testing OEP + ctors for derby 32-bit app...";
    // test oep + ptrace
    if (!test_ctors_oep_wrappers("derby32", "oep_t.asm", "ptrace_t.asm", "exit_t.asm")) {
        qDebug() << "something went wrong :(";
    }
    qDebug() << "Testing OEP + ctors for derby 32-bit app done";
    qDebug() << "=========================================";
    qDebug() << "Testing OEP + ctors for derby 64-bit app...";
    if (!test_ctors_oep_wrappers("derby64", "oep64_t.asm", "ptrace64_t.asm", "exit64_t.asm")) {
        qDebug() << "something went wrong :(";
    }
    qDebug() << "Testing OEP + ctors for derby 64-bit app done";
    qDebug() << "=========================================";
    qDebug() << "Testing OEP + ctors for edb 64-bit app...";
    if (!test_ctors_oep_wrappers("edb", "oep64_t.asm", "ptrace64_t.asm", "exit64_t.asm")) {
        qDebug() << "something went wrong :(";
    }
    qDebug() << "Testing OEP + ctors for edb 64-bit app done";
    qDebug() << "=========================================";
    qDebug() << "Testing OEP + ctors for dDeflect 64-bit app...";
    if (!test_ctors_oep_wrappers("dDeflect", "oep64_t.asm", "ptrace64_t.asm", "exit64_t.asm")) {
        qDebug() << "something went wrong :(";
    }
    qDebug() << "Testing OEP + ctors for dDeflect 64-bit app done";
    qDebug() << "=========================================";
    */

    /*
    qDebug() << "Testing OEP + init for my 32-bit app...";
    // test oep + ptrace
    if (!test_init_oep_wrappers("my32", "oep_t.asm", "ptrace_t.asm", "exit_group_t.asm")) {
        qDebug() << "something went wrong :(";
    }
    qDebug() << "Testing OEP + init for my 32-bit app done";
    qDebug() << "=========================================";


    qDebug() << "Testing OEP + init for my 64-bit app...";
    if (!test_init_oep_wrappers("my64", "oep64_t.asm", "ptrace64_t.asm", "exit_group64_t.asm")) {
        qDebug() << "something went wrong :(";
    }
    qDebug() << "Testing OEP + init for my 64-bit app done";
    qDebug() << "=========================================";


    qDebug() << "Testing OEP + init for derby 32-bit app...";
    // test oep + ptrace
    if (!test_init_oep_wrappers("derby32", "oep_t.asm", "ptrace_t.asm", "exit_group_t.asm")) {
        qDebug() << "something went wrong :(";
    }
    qDebug() << "Testing OEP + init for derby 32-bit app done";
    qDebug() << "=========================================";

    qDebug() << "Testing OEP + init for derby 64-bit app...";
    if (!test_init_oep_wrappers("derby64", "oep64_t.asm", "ptrace64_t.asm", "exit_group64_t.asm")) {
        qDebug() << "something went wrong :(";
    }
    qDebug() << "Testing OEP + init for derby 64-bit app done";
    qDebug() << "=========================================";

    qDebug() << "Testing OEP + init for edb 64-bit app...";
    if (!test_init_oep_wrappers("edb", "oep64_t.asm", "ptrace64_t.asm", "exit_group64_t.asm")) {
        qDebug() << "something went wrong :(";
    }
    qDebug() << "Testing OEP + init for edb 64-bit app done";
    qDebug() << "=========================================";
    qDebug() << "Testing OEP + init for dDeflect 64-bit app...";
    if (!test_init_oep_wrappers("dDeflect", "oep64_t.asm", "ptrace64_t.asm", "exit_group64_t.asm")) {
        qDebug() << "something went wrong :(";
    }
    qDebug() << "Testing OEP + init for dDeflect 64-bit app done";



    qDebug() << "Testing OEP + initarray for my 32-bit app...";
    // test oep + ptrace
    if (!test_initarray_oep_wrappers("my32", "oep_t.asm", "ptrace_t.asm", "exit_group_t.asm")) {
        qDebug() << "something went wrong :(";
    }
    qDebug() << "Testing OEP + initarray for my 32-bit app done";
    qDebug() << "=========================================";
    qDebug() << "Testing OEP + initarray for my 64-bit app...";
    if (!test_initarray_oep_wrappers("my64", "oep64_t.asm", "ptrace64_t.asm", "exit_group64_t.asm")) {
        qDebug() << "something went wrong :(";
    }
    qDebug() << "Testing OEP + initarray for my 64-bit app done";
    qDebug() << "=========================================";
    qDebug() << "Testing OEP + initarray for derby 32-bit app...";
    // test oep + ptrace
    if (!test_initarray_oep_wrappers("derby32", "oep_t.asm", "ptrace_t.asm", "exit_group_t.asm")) {
        qDebug() << "something went wrong :(";
    }
    qDebug() << "Testing OEP + initarray for derby 32-bit app done";
    qDebug() << "=========================================";
    qDebug() << "Testing OEP + initarray for derby 64-bit app...";
    if (!test_initarray_oep_wrappers("derby64", "oep64_t.asm", "ptrace64_t.asm", "exit_group64_t.asm")) {
        qDebug() << "something went wrong :(";
    }
    qDebug() << "Testing OEP + initarray for derby 64-bit app done";
    qDebug() << "=========================================";
    qDebug() << "Testing OEP + initarray for edb 64-bit app...";
    if (!test_initarray_oep_wrappers("edb", "oep64_t.asm", "ptrace64_t.asm", "exit_group64_t.asm")) {
        qDebug() << "something went wrong :(";
    }
    qDebug() << "Testing OEP + initarray for edb 64-bit app done";
    qDebug() << "=========================================";
    qDebug() << "Testing OEP + initarray for dDeflect 64-bit app...";
    if (!test_initarray_oep_wrappers("dDeflect", "oep64_t.asm", "ptrace64_t.asm", "exit_group64_t.asm")) {
        qDebug() << "something went wrong :(";
    }
    qDebug() << "Testing OEP + initarray for dDeflect 64-bit app done";
    */

    /*
    qDebug() << "Testing THREAD + init for my 32-bit app...";
    // test oep + ptrace
    if (!test_init_oep_wrappers("my32", "thread_t.asm", "ptrace_t.asm", "exit_group_t.asm")) {
        qDebug() << "something went wrong :(";
    }
    qDebug() << "Testing THREAD + init for my 32-bit app done";
    qDebug() << "=========================================";


    qDebug() << "Testing THREAD + init for my 64-bit app...";
    if (!test_init_oep_wrappers("my64", "thread64_t.asm", "ptrace64_t.asm", "exit_group64_t.asm")) {
        qDebug() << "something went wrong :(";
    }
    qDebug() << "Testing THREAD + init for my 64-bit app done";
    qDebug() << "=========================================";


    qDebug() << "Testing THREAD + init for derby 32-bit app...";
    // test oep + ptrace
    if (!test_init_oep_wrappers("derby32", "thread_t.asm", "ptrace_t.asm", "exit_group_t.asm")) {
        qDebug() << "something went wrong :(";
    }
    qDebug() << "Testing THREAD + init for derby 32-bit app done";
    qDebug() << "=========================================";

    qDebug() << "Testing THREAD + init for derby 64-bit app...";
    if (!test_init_oep_wrappers("derby64", "thread64_t.asm", "ptrace64_t.asm", "exit_group64_t.asm")) {
        qDebug() << "something went wrong :(";
    }
    qDebug() << "Testing THREAD + init for derby 64-bit app done";
    qDebug() << "=========================================";

    qDebug() << "Testing THREAD + init for edb 64-bit app...";
    if (!test_init_oep_wrappers("edb", "thread64_t.asm", "ptrace64_t.asm", "exit_group64_t.asm")) {
        qDebug() << "something went wrong :(";
    }
    qDebug() << "Testing THREAD + init for edb 64-bit app done";
    qDebug() << "=========================================";
    qDebug() << "Testing OEP + init for dDeflect 64-bit app...";
    if (!test_init_oep_wrappers("dDeflect", "thread64_t.asm", "ptrace64_t.asm", "exit_group64_t.asm")) {
        qDebug() << "something went wrong :(";
    }
    qDebug() << "Testing THREAD + init for dDeflect 64-bit app done";


    qDebug() << "Testing THREAD + initarray for my 32-bit app...";
    // test oep + ptrace
    if (!test_initarray_oep_wrappers("my32", "thread_t.asm", "ptrace_t.asm", "exit_group_t.asm")) {
        qDebug() << "something went wrong :(";
    }
    qDebug() << "Testing THREAD + initarray for my 32-bit app done";
    qDebug() << "=========================================";
    qDebug() << "Testing THREAD + initarray for my 64-bit app...";
    if (!test_initarray_oep_wrappers("my64", "thread64_t.asm", "ptrace64_t.asm", "exit_group64_t.asm")) {
        qDebug() << "something went wrong :(";
    }
    qDebug() << "Testing THREAD + initarray for my 64-bit app done";
    qDebug() << "=========================================";
    qDebug() << "Testing THREAD + initarray for derby 32-bit app...";
    // test oep + ptrace
    if (!test_initarray_oep_wrappers("derby32", "thread_t.asm", "ptrace_t.asm", "exit_group_t.asm")) {
        qDebug() << "something went wrong :(";
    }
    qDebug() << "Testing THREAD + initarray for derby 32-bit app done";
    qDebug() << "=========================================";
    qDebug() << "Testing THREAD + initarray for derby 64-bit app...";
    if (!test_initarray_oep_wrappers("derby64", "thread64_t.asm", "ptrace64_t.asm", "exit_group64_t.asm")) {
        qDebug() << "something went wrong :(";
    }
    qDebug() << "Testing THREAD + initarray for derby 64-bit app done";
    qDebug() << "=========================================";
    qDebug() << "Testing THREAD + initarray for edb 64-bit app...";
    if (!test_initarray_oep_wrappers("edb", "thread64_t.asm", "ptrace64_t.asm", "exit_group64_t.asm")) {
        qDebug() << "something went wrong :(";
    }
    qDebug() << "Testing THREAD + initarray for edb 64-bit app done";
    qDebug() << "=========================================";
    qDebug() << "Testing THREAD + initarray for dDeflect 64-bit app...";
    if (!test_initarray_oep_wrappers("dDeflect", "thread64_t.asm", "ptrace64_t.asm", "exit_group64_t.asm")) {
        qDebug() << "something went wrong :(";
    }
    qDebug() << "Testing THREAD + initarray for dDeflect 64-bit app done";
    */


}
#endif

void test_thread_wrapper() {
    QString dmeth_fld = "methods";
    QString bin_fld = "bin";
    QString out_fld = "secured";

    QList<QString> dmeth_x86 = { "cc.asm", "ptrace.asm", "sigtrap.asm" };
    QList<QString> dmeth_x64 = { "cc64.asm", "ptrace64.asm" };

    QList<QString> bin_x86 = { "my32", "derby32" };
    QList<QString> bin_x64 = { "my64", "derby64", "edb", "dDeflect" };

    QString mode_x86 = "thread.asm",
            mode_x64 = "thread64.asm";

    QList<QString> handlers_x86 = { "exit_group.asm" };
    QList<QString> handlers_x64 = { "exit_group64.asm" };
    QString handl_fld = "handlers";

    qDebug() << "xxxxxxxxxxxxxxxxxxxxxxxxxx\n"
             << "Testing Thread Wrapper x86\n"
             << "xxxxxxxxxxxxxxxxxxxxxxxxxx\n";

    test_thread_wrapper_x86(bin_fld, dmeth_fld, out_fld, bin_x86, dmeth_x86, mode_x86, handl_fld, handlers_x86);

    qDebug() << "xxxxxxxxxxxxxxxxxxxxxxxxxxx\n"
             << "Finished Thread Wrapper x86\n"
             << "xxxxxxxxxxxxxxxxxxxxxxxxxxx\n";

    qDebug() << "xxxxxxxxxxxxxxxxxxxxxxxxxx\n"
             << "Testing Thread Wrapper x64\n"
             << "xxxxxxxxxxxxxxxxxxxxxxxxxx\n";

    test_thread_wrapper_x64(bin_fld, dmeth_fld, out_fld, bin_x64, dmeth_x64, mode_x64, handl_fld, handlers_x64);

    qDebug() << "xxxxxxxxxxxxxxxxxxxxxxxxxxx\n"
             << "Finished Thread Wrapper x64\n"
             << "xxxxxxxxxxxxxxxxxxxxxxxxxxx\n";

}

void test_thread_wrapper_x86(const QString &bin_fld, const QString &dmeth_fld, const QString &out_fld, const QList<QString> &bin,
                             const QList<QString> &meth, const QString &mode, const QString &handl_fld, const QList<QString> &hand) {

    // TODO: INIT STRUCTURE, SHOULD BE DONE USING A JSON MODULE
    // ============================================================
    // ============================================================
    DAddingMethods<Registers_x86>::InjectDescription inject_desc;
    DAddingMethods<Registers_x86>::TrampolineWrapper oepwrapper;
    DAddingMethods<Registers_x86>::Wrapper handler;
    DAddingMethods<Registers_x86>::Wrapper oepaction;

    handler.static_params = { { "ret", "127" } };
    handler.detect_handler = nullptr;

    oepaction.detect_handler = nullptr;
    oepaction.ret = Registers_x86::EAX;
    oepaction.used_regs = { Registers_x86::EAX, Registers_x86::ECX,
            Registers_x86::EBX, Registers_x86::EDX,
            Registers_x86::ESI, Registers_x86::EDI };

    oepwrapper.detect_handler = &handler;
    oepwrapper.tramp_action = &oepaction;
    oepwrapper.used_regs = { Registers_x86::EAX, Registers_x86::ECX,
                           Registers_x86::EBX, Registers_x86::EDX,
                           Registers_x86::ESI, Registers_x86::EDI };

    oepwrapper.static_params = { { "sleep1", "0" },
                               { "sleep2", "5" } };

    oepwrapper.ret = oepaction.ret;

    // ============================================================
    // ============================================================

    QFile code;
    QString binfp, mfp, hfp;
    foreach (QString b, bin) {
        binfp = QString("%1/%2").arg(bin_fld, b);
        QFile f(binfp);
        if(!f.open(QFile::ReadOnly)) {
            qDebug() << "Error opening file: " << binfp;
            return;
        }

        ELF elf(f.readAll());
        if (!elf.is_valid())
            return;

        ELFAddingMethods<Registers_x86> dam(&elf);

        foreach (QString m, meth) {
            mfp = QString("%1/%2").arg(dmeth_fld, m);
            foreach (QString h, hand) {
                hfp = QString("%1/%2").arg(handl_fld, h);

                // ===========================
                // debugger handler generation
                // ===========================
                code.setFileName(hfp);
                if (!code.open(QIODevice::ReadOnly))
                    return;
                handler.code = code.readAll();
                code.close();

                // =============================
                // debugger detection generation
                // =============================
                code.setFileName(mfp);
                if (!code.open(QIODevice::ReadOnly))
                    return;

                oepaction.code = code.readAll();
                code.close();

                // FOR CC code
                QPair<QByteArray, Elf64_Addr> section_data;
                if (!elf.get_section_content(ELF::SectionType::TEXT, section_data))
                    return;

                oepaction.static_params = { { "vsize", QString::number(section_data.first.size()) },
                                            { "vaddr", QString::number(section_data.second) } };

                // ==============================
                // wrapper information generation
                // ==============================
                code.setFileName(mode);
                if (!code.open(QIODevice::ReadOnly))
                    return;

                oepwrapper.code = code.readAll();
                oepwrapper.ret = oepaction.ret;

                code.close();

                inject_desc.cm = DAddingMethods<Registers_x86>::CallingMethod::Thread;
                inject_desc.adding_method = &oepwrapper;
                inject_desc.saved_fname = QString("thread_%1_%2_%3").arg(b, m, h);

                if (!dam.secure({&inject_desc}))
                    return;

                system(QString("mv %1 %2").arg(inject_desc.saved_fname, out_fld).toStdString().c_str());
            }
        }
    }
}

void test_thread_wrapper_x64(const QString &bin_fld, const QString &dmeth_fld, const QString &out_fld, const QList<QString> &bin,
                             const QList<QString> &meth, const QString &mode, const QString &handl_fld, const QList<QString> &hand) {

    // TODO: INIT STRUCTURE, SHOULD BE DONE USING A JSON MODULE
    // ============================================================
    // ============================================================
    DAddingMethods<Registers_x64>::InjectDescription inject_desc;
    DAddingMethods<Registers_x64>::TrampolineWrapper oepwrapper;
    DAddingMethods<Registers_x64>::Wrapper handler;
    DAddingMethods<Registers_x64>::Wrapper oepaction;

    handler.static_params = { { "ret", "188" } };
    handler.detect_handler = nullptr;

    oepaction.detect_handler = nullptr;
    oepaction.ret = Registers_x64::RAX;
    oepaction.used_regs = { Registers_x64::RAX, Registers_x64::RDI,
                            Registers_x64::RSI, Registers_x64::RDX,
                            Registers_x64::R10 };

    oepwrapper.detect_handler = &handler;
    oepwrapper.tramp_action = &oepaction;
    oepwrapper.used_regs = { Registers_x64::RAX, Registers_x64::RDI,
                             Registers_x64::RSI, Registers_x64::RDX,
                             Registers_x64::R10, Registers_x64::R8 };
    oepwrapper.static_params = { { "sleep1", "0" },
                                                   { "sleep2", "5" } };
    oepwrapper.ret = oepaction.ret;

    // ============================================================
    // ============================================================

    QFile code;
    QString binfp, mfp, hfp;
    foreach (QString b, bin) {
        binfp = QString("%1/%2").arg(bin_fld, b);
        QFile f(binfp);
        if(!f.open(QFile::ReadOnly)) {
            qDebug() << "Error opening file: " << binfp;
            return;
        }

        ELF elf(f.readAll());
        if (!elf.is_valid())
            return;

        ELFAddingMethods<Registers_x64> dam(&elf);

        foreach (QString m, meth) {
            mfp = QString("%1/%2").arg(dmeth_fld, m);
            foreach (QString h, hand) {
                hfp = QString("%1/%2").arg(handl_fld, h);

                // ===========================
                // debugger handler generation
                // ===========================
                code.setFileName(hfp);
                if (!code.open(QIODevice::ReadOnly))
                    return;
                handler.code = code.readAll();
                code.close();

                // =============================
                // debugger detection generation
                // =============================
                code.setFileName(mfp);
                if (!code.open(QIODevice::ReadOnly))
                    return;

                oepaction.code = code.readAll();
                code.close();

                // FOR CC code
                QPair<QByteArray, Elf64_Addr> section_data;
                if (!elf.get_section_content(ELF::SectionType::TEXT, section_data))
                    return;

                oepaction.static_params = { { "vsize", QString::number(section_data.first.size()) },
                                            { "vaddr", QString::number(section_data.second) } };

                // ==============================
                // wrapper information generation
                // ==============================
                code.setFileName(mode);
                if (!code.open(QIODevice::ReadOnly))
                    return;

                oepwrapper.code = code.readAll();
                oepwrapper.ret = oepaction.ret;

                code.close();

                inject_desc.cm = DAddingMethods<Registers_x64>::CallingMethod::Thread;
                inject_desc.adding_method = &oepwrapper;
                inject_desc.saved_fname = QString("thread_%1_%2_%3").arg(b, m, h);

                if (!dam.secure({&inject_desc}))
                    return;

                system(QString("mv %1 %2").arg(inject_desc.saved_fname, out_fld).toStdString().c_str());
            }
        }
    }

}

void test_init_wrapper() {
    QString dmeth_fld = "methods";
    QString bin_fld = "bin";
    QString out_fld = "secured";
    QList<QString> dmeth_x86 = { "cc.asm", "ptrace.asm", "sigtrap.asm" };
    QList<QString> dmeth_x64 = { "cc64.asm", "ptrace64.asm" };
    QList<QString> bin_x86 = { "my32", "derby32" };
    QList<QString> bin_x64 = { "my64", "derby64", "edb", "dDeflect" };
    QString mode_x86 = "single.asm",
            mode_x64 = "single64.asm";

    QList<QString> handlers_x86 = { "exit.asm" };
    QList<QString> handlers_x64 = { "exit64.asm" };
    QString handl_fld = "handlers";

    qDebug() << "xxxxxxxxxxxxxxxxxxxxxxxx\n"
             << "Testing Init Wrapper x86\n"
             << "xxxxxxxxxxxxxxxxxxxxxxxx\n";

    test_thread_wrapper_x86(bin_fld, dmeth_fld, out_fld, bin_x86, dmeth_x86, mode_x86, handl_fld, handlers_x86);

    qDebug() << "xxxxxxxxxxxxxxxxxxxxxxxxx\n"
             << "Finished Init Wrapper x86\n"
             << "xxxxxxxxxxxxxxxxxxxxxxxxx\n";

    qDebug() << "xxxxxxxxxxxxxxxxxxxxxxxx\n"
             << "Testing Init Wrapper x64\n"
             << "xxxxxxxxxxxxxxxxxxxxxxxx\n";

    test_thread_wrapper_x64(bin_fld, dmeth_fld, out_fld, bin_x64, dmeth_x64, mode_x64, handl_fld, handlers_x64);

    qDebug() << "xxxxxxxxxxxxxxxxxxxxxxxxx\n"
             << "Finished Init Wrapper x64\n"
             << "xxxxxxxxxxxxxxxxxxxxxxxxx\n";


}

void test_init_wrapper_x86(const QString &bin_fld, const QString &dmeth_fld, const QString &out_fld, const QList<QString> &bin,
                           const QList<QString> &meth, const QString &mode, const QString &handl_fld, const QList<QString> &hand) {

    // TODO: INIT STRUCTURE, SHOULD BE DONE USING A JSON MODULE
    // ============================================================
    // ============================================================
    DAddingMethods<Registers_x86>::InjectDescription inject_desc;
    DAddingMethods<Registers_x86>::TrampolineWrapper oepwrapper;
    DAddingMethods<Registers_x86>::Wrapper handler;
    DAddingMethods<Registers_x86>::Wrapper oepaction;

    handler.static_params = { { "ret", "127" } };
    handler.detect_handler = nullptr;

    oepaction.detect_handler = nullptr;
    oepaction.ret = Registers_x86::EAX;
    oepaction.used_regs = { Registers_x86::EAX, Registers_x86::ECX,
                            Registers_x86::EBX, Registers_x86::EDX,
                            Registers_x86::ESI };

    oepwrapper.detect_handler = &handler;
    oepwrapper.tramp_action = &oepaction;
    oepwrapper.used_regs = { Registers_x86::EDI };

    oepwrapper.static_params = { { "sleep1", "0" },
                               { "sleep2", "5" } };

    oepwrapper.ret = oepaction.ret;

    // ============================================================
    // ============================================================

    QFile code;
    QString binfp, mfp, hfp;
    foreach (QString b, bin) {
        binfp = QString("%1/%2").arg(bin_fld, b);
        QFile f(binfp);
        if(!f.open(QFile::ReadOnly)) {
            qDebug() << "Error opening file: " << binfp;
            return;
        }

        ELF elf(f.readAll());
        if (!elf.is_valid())
            return;

        ELFAddingMethods<Registers_x86> dam(&elf);

        foreach (QString m, meth) {
            mfp = QString("%1/%2").arg(dmeth_fld, m);
            foreach (QString h, hand) {
                hfp = QString("%1/%2").arg(handl_fld, h);

                // ===========================
                // debugger handler generation
                // ===========================
                code.setFileName(hfp);
                if (!code.open(QIODevice::ReadOnly))
                    return;
                handler.code = code.readAll();
                code.close();

                // =============================
                // debugger detection generation
                // =============================
                code.setFileName(mfp);
                if (!code.open(QIODevice::ReadOnly))
                    return;

                oepaction.code = code.readAll();
                code.close();

                // FOR CC code
                QPair<QByteArray, Elf64_Addr> section_data;
                if (!elf.get_section_content(ELF::SectionType::TEXT, section_data))
                    return;

                oepaction.static_params = { { "vsize", QString::number(section_data.first.size()) },
                                            { "vaddr", QString::number(section_data.second) } };

                // ==============================
                // wrapper information generation
                // ==============================
                code.setFileName(mode);
                if (!code.open(QIODevice::ReadOnly))
                    return;

                oepwrapper.code = code.readAll();
                oepwrapper.ret = oepaction.ret;

                code.close();

                inject_desc.cm = DAddingMethods<Registers_x86>::CallingMethod::INIT;
                inject_desc.adding_method = &oepwrapper;
                inject_desc.saved_fname = QString("init_%1_%2_%3").arg(b, m, h);

                if (!dam.secure({&inject_desc}))
                    return;

                system(QString("mv %1 %2").arg(inject_desc.saved_fname, out_fld).toStdString().c_str());
            }
        }
    }

}

void test_init_wrapper_x64(const QString &bin_fld, const QString &dmeth_fld, const QString &out_fld, const QList<QString> &bin,
                           const QList<QString> &meth, const QString &mode, const QString &handl_fld, const QList<QString> &hand) {
    // TODO: INIT STRUCTURE, SHOULD BE DONE USING A JSON MODULE
    // ============================================================
    // ============================================================
    DAddingMethods<Registers_x64>::InjectDescription inject_desc;
    DAddingMethods<Registers_x64>::TrampolineWrapper oepwrapper;
    DAddingMethods<Registers_x64>::Wrapper handler;
    DAddingMethods<Registers_x64>::Wrapper oepaction;

    handler.static_params = { { "ret", "188" } };
    handler.detect_handler = nullptr;

    oepaction.detect_handler = nullptr;
    oepaction.ret = Registers_x64::RAX;
    oepaction.used_regs = { Registers_x64::RAX, Registers_x64::RDI,
                            Registers_x64::RSI, Registers_x64::RDX,
                            Registers_x64::R10 };

    oepwrapper.detect_handler = &handler;
    oepwrapper.tramp_action = &oepaction;
    oepwrapper.used_regs = { Registers_x64::RAX, Registers_x64::RDI,
                             Registers_x64::RSI, Registers_x64::RDX,
                             Registers_x64::R10, Registers_x64::R8 };
    oepwrapper.ret = oepaction.ret;

    // ============================================================
    // ============================================================

    QFile code;
    QString binfp, mfp, hfp;
    foreach (QString b, bin) {
        binfp = QString("%1/%2").arg(bin_fld, b);
        QFile f(binfp);
        if(!f.open(QFile::ReadOnly)) {
            qDebug() << "Error opening file: " << binfp;
            return;
        }

        ELF elf(f.readAll());
        if (!elf.is_valid())
            return;

        ELFAddingMethods<Registers_x64> dam(&elf);

        foreach (QString m, meth) {
            mfp = QString("%1/%2").arg(dmeth_fld, m);
            foreach (QString h, hand) {
                hfp = QString("%1/%2").arg(handl_fld, h);

                // ===========================
                // debugger handler generation
                // ===========================
                code.setFileName(hfp);
                if (!code.open(QIODevice::ReadOnly))
                    return;
                handler.code = code.readAll();
                code.close();

                // =============================
                // debugger detection generation
                // =============================
                code.setFileName(mfp);
                if (!code.open(QIODevice::ReadOnly))
                    return;

                oepaction.code = code.readAll();
                code.close();

                // FOR CC code
                QPair<QByteArray, Elf64_Addr> section_data;
                if (!elf.get_section_content(ELF::SectionType::TEXT, section_data))
                    return;

                oepaction.static_params = { { "vsize", QString::number(section_data.first.size()) },
                                            { "vaddr", QString::number(section_data.second) } };

                // ==============================
                // wrapper information generation
                // ==============================
                code.setFileName(mode);
                if (!code.open(QIODevice::ReadOnly))
                    return;

                oepwrapper.code = code.readAll();
                oepwrapper.ret = oepaction.ret;

                code.close();

                inject_desc.cm = DAddingMethods<Registers_x64>::CallingMethod::INIT;
                inject_desc.adding_method = &oepwrapper;
                inject_desc.saved_fname = QString("init_%1_%2_%3").arg(b, m, h);

                if (!dam.secure({&inject_desc}))
                    return;

                system(QString("mv %1 %2").arg(inject_desc.saved_fname, out_fld).toStdString().c_str());
            }
        }
    }
}

void test_oep_wrapper() {
    QString dmeth_fld = "methods";
    QString bin_fld = "bin";
    QString out_fld = "secured";
    QList<QString> dmeth_x86 = { "cc.asm", "ptrace.asm", "sigtrap.asm" };
    QList<QString> dmeth_x64 = { "cc64.asm", "ptrace64.asm" };
    QList<QString> bin_x86 = { "my32", "derby32" };
    QList<QString> bin_x64 = { "my64", "derby64", "edb", "dDeflect" };
    QString mode_x86 = "single.asm",
            mode_x64 = "single64.asm";

    QList<QString> handlers_x86 = { "exit.asm" };
    QList<QString> handlers_x64 = { "exit64.asm" };
    QString handl_fld = "handlers";


    qDebug() << "xxxxxxxxxxxxxxxxxxxxxxx\n"
             << "Testing OEP Wrapper x86\n"
             << "xxxxxxxxxxxxxxxxxxxxxxx\n";

    test_oep_wrapper_x86(bin_fld, dmeth_fld, out_fld, bin_x86, dmeth_x86, mode_x86, handl_fld, handlers_x86);

    qDebug() << "xxxxxxxxxxxxxxxxxxxxxxxx\n"
             << "Finished OEP Wrapper x86\n"
             << "xxxxxxxxxxxxxxxxxxxxxxxx\n";

    qDebug() << "xxxxxxxxxxxxxxxxxxxxxxx\n"
             << "Testing OEP Wrapper x64\n"
             << "xxxxxxxxxxxxxxxxxxxxxxx\n";

    test_oep_wrapper_x64(bin_fld, dmeth_fld, out_fld, bin_x64, dmeth_x64, mode_x64, handl_fld, handlers_x64);

    qDebug() << "xxxxxxxxxxxxxxxxxxxxxxxx\n"
             << "Finished OEP Wrapper x64\n"
             << "xxxxxxxxxxxxxxxxxxxxxxxx\n";

}

void test_oep_wrapper_x86(const QString &bin_fld, const QString &dmeth_fld, const QString &out_fld, const QList<QString> &bin,
                          const QList<QString> &meth, const QString &mode, const QString &handl_fld, const QList<QString> &hand) {

}

void test_oep_wrapper_x64(const QString &bin_fld, const QString &dmeth_fld, const QString &out_fld, const QList<QString> &bin,
                          const QList<QString> &meth, const QString &mode, const QString &handl_fld, const QList<QString> &hand) {

}

void test_init_array_wrapper() {
    QString dmeth_fld = "methods";
    QString bin_fld = "bin";
    QString out_fld = "secured";
    QList<QString> dmeth_x86 = { "cc.asm", "ptrace.asm", "sigtrap.asm" };
    QList<QString> dmeth_x64 = { "cc64.asm", "ptrace64.asm" };
    QList<QString> bin_x86 = { "my32", "derby32" };
    QList<QString> bin_x64 = { "my64", "derby64", "edb", "dDeflect" };
    QString mode_x86 = "single.asm",
            mode_x64 = "single64.asm";

    QList<QString> handlers_x86 = { "exit.asm" };
    QList<QString> handlers_x64 = { "exit64.asm" };
    QString handl_fld = "handlers";

    qDebug() << "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n"
             << "Testing Init Array Wrapper x86\n"
             << "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n";

    test_init_array_wrapper_x86(bin_fld, dmeth_fld, out_fld, bin_x86, dmeth_x86, mode_x86, handl_fld, handlers_x86);

    qDebug() << "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n"
             << "Finished Init Array Wrapper x86\n"
             << "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n";

    qDebug() << "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n"
             << "Testing Init Array Wrapper x64\n"
             << "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n";
    test_init_array_wrapper_x64(bin_fld, dmeth_fld, out_fld, bin_x64, dmeth_x64, mode_x64, handl_fld, handlers_x64);

    qDebug() << "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n"
             << "Finished Init Array Wrapper x64\n"
             << "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n";

}

void test_init_array_wrapper_x86(const QString &bin_fld, const QString &dmeth_fld, const QString &out_fld, const QList<QString> &bin,
                                 const QList<QString> &meth, const QString &mode, const QString &handl_fld, const QList<QString> &hand) {
}

void test_init_array_wrapper_x64(const QString &bin_fld, const QString &dmeth_fld, const QString &out_fld, const QList<QString> &bin,
                                 const QList<QString> &meth, const QString &mode, const QString &handl_fld, const QList<QString> &hand) {

}

void test_ctors_wrapper() {
    QString dmeth_fld = "methods";
    QString bin_fld = "bin";
    QString out_fld = "secured";
    QList<QString> dmeth_x86 = { "cc.asm", "ptrace.asm", "sigtrap.asm" };
    QList<QString> dmeth_x64 = { "cc64.asm", "ptrace64.asm" };
    QList<QString> bin_x86 = { "my32", "derby32" };
    QList<QString> bin_x64 = { "my64", "derby64", "edb", "dDeflect" };
    QString mode_x86 = "single.asm",
            mode_x64 = "single64.asm";

    QList<QString> handlers_x86 = { "exit.asm" };
    QList<QString> handlers_x64 = { "exit64.asm" };
    QString handl_fld = "handlers";


    qDebug() << "xxxxxxxxxxxxxxxxxxxxxxxxx\n"
             << "Testing Ctors Wrapper x86\n"
             << "xxxxxxxxxxxxxxxxxxxxxxxxx\n";

    test_ctors_wrapper_x86(bin_fld, dmeth_fld, out_fld, bin_x86, dmeth_x86, mode_x86, handl_fld, handlers_x86);

    qDebug() << "xxxxxxxxxxxxxxxxxxxxxxxxxx\n"
             << "Finished Ctors Wrapper x86\n"
             << "xxxxxxxxxxxxxxxxxxxxxxxxxx\n";

    qDebug() << "xxxxxxxxxxxxxxxxxxxxxxxxx\n"
             << "Testing Ctors Wrapper x64\n"
             << "xxxxxxxxxxxxxxxxxxxxxxxxx\n";

    test_ctors_wrapper_x64(bin_fld, dmeth_fld, out_fld, bin_x64, dmeth_x64, mode_x64, handl_fld, handlers_x64);

    qDebug() << "xxxxxxxxxxxxxxxxxxxxxxxxxx\n"
             << "Finished Ctors Wrapper x64\n"
             << "xxxxxxxxxxxxxxxxxxxxxxxxxx\n";
}

void test_ctors_wrapper_x86(const QString &bin_fld, const QString &dmeth_fld, const QString &out_fld, const QList<QString> &bin,
                            const QList<QString> &meth, const QString &mode, const QString &handl_fld, const QList<QString> &hand) {

}

void test_ctors_wrapper_x64(const QString &bin_fld, const QString &dmeth_fld, const QString &out_fld, const QList<QString> &bin,
                            const QList<QString> &meth, const QString &mode, const QString &handl_fld, const QList<QString> &hand) {

}


void test_trampoline_wrapper_x86(const QString &bin_fld, const QString &dmeth_fld, const QString &out_fld, const QList<QString> &bin,
                                 const QList<QString> &meth, const QString &mode, const QString &handl_fld, const QList<QString> &hand) {


    // TODO: INIT STRUCTURE, SHOULD BE DONE USING A JSON MODULE
    // ============================================================
    // ============================================================
    DAddingMethods<Registers_x86>::InjectDescription inject_desc;
    DAddingMethods<Registers_x86>::TrampolineWrapper oepwrapper;
    DAddingMethods<Registers_x86>::Wrapper handler;
    DAddingMethods<Registers_x86>::Wrapper oepaction;

    handler.static_params = { { "ret", "127" } };
    handler.detect_handler = nullptr;

    oepaction.detect_handler = nullptr;
    oepaction.ret = Registers_x86::EAX;
    oepaction.used_regs = { Registers_x86::EAX, Registers_x86::ECX,
                            Registers_x86::EBX, Registers_x86::EDX,
                            Registers_x86::ESI };

    oepwrapper.detect_handler = &handler;
    oepwrapper.tramp_action = &oepaction;
    oepwrapper.used_regs = { Registers_x86::EDI };
    oepwrapper.ret = oepaction.ret;

    // ============================================================
    // ============================================================

    QFile code;
    QString binfp, mfp, hfp;
    foreach (QString b, bin) {
        binfp = QString("%1/%2").arg(bin_fld, b);
        QFile f(binfp);
        if(!f.open(QFile::ReadOnly)) {
            qDebug() << "Error opening file: " << binfp;
            return;
        }

        ELF elf(f.readAll());
        if (!elf.is_valid())
            return;

        ELFAddingMethods<Registers_x86> dam(&elf);

        foreach (QString m, meth) {
            mfp = QString("%1/%2").arg(dmeth_fld, m);
            foreach (QString h, hand) {
                hfp = QString("%1/%2").arg(handl_fld, h);

                // ===========================
                // debugger handler generation
                // ===========================
                code.setFileName(hfp);
                if (!code.open(QIODevice::ReadOnly))
                    return;
                handler.code = code.readAll();
                code.close();

                // =============================
                // debugger detection generation
                // =============================
                code.setFileName(mfp);
                if (!code.open(QIODevice::ReadOnly))
                    return;

                oepaction.code = code.readAll();
                code.close();

                // FOR CC code
                QPair<QByteArray, Elf64_Addr> section_data;
                if (!elf.get_section_content(ELF::SectionType::TEXT, section_data))
                    return;

                oepaction.static_params = { { "vsize", QString::number(section_data.first.size()) },
                                            { "vaddr", QString::number(section_data.second) } };

                // ==============================
                // wrapper information generation
                // ==============================
                code.setFileName(mode);
                if (!code.open(QIODevice::ReadOnly))
                    return;

                oepwrapper.code = code.readAll();
                oepwrapper.ret = oepaction.ret;

                code.close();

                inject_desc.cm = DAddingMethods<Registers_x86>::CallingMethod::Trampoline;
                inject_desc.adding_method = &oepwrapper;
                inject_desc.saved_fname = QString("trampoline_%1_%2_%3").arg(b, m, h);

                if (!dam.secure({&inject_desc}))
                    return;

                system(QString("mv %1 %2").arg(inject_desc.saved_fname, out_fld).toStdString().c_str());
            }
        }
    }
}

void test_trampoline_wrapper_x64(const QString &bin_fld, const QString &dmeth_fld, const QString &out_fld, const QList<QString> &bin,
                                 const QList<QString> &meth, const QString &mode, const QString &handl_fld, const QList<QString> &hand) {

    // TODO: INIT STRUCTURE, SHOULD BE DONE USING A JSON MODULE
    // ============================================================
    // ============================================================
    DAddingMethods<Registers_x64>::InjectDescription inject_desc;
    DAddingMethods<Registers_x64>::TrampolineWrapper oepwrapper;
    DAddingMethods<Registers_x64>::Wrapper handler;
    DAddingMethods<Registers_x64>::Wrapper oepaction;

    handler.static_params = { { "ret", "188" } };
    handler.detect_handler = nullptr;

    oepaction.detect_handler = nullptr;
    oepaction.ret = Registers_x64::RAX;
    oepaction.used_regs = { Registers_x64::RAX, Registers_x64::RDI, Registers_x64::RCX,
                            Registers_x64::RSI, Registers_x64::RDX, Registers_x64::RBX,
                            Registers_x64::RBP, Registers_x64::R10,
                            Registers_x64::R15, Registers_x64::R14 };

    oepwrapper.detect_handler = &handler;
    oepwrapper.tramp_action = &oepaction;
    oepwrapper.used_regs = { Registers_x64::R11, Registers_x64::R12 };
    oepwrapper.ret = oepaction.ret;

    // ============================================================
    // ============================================================

    QFile code;
    QString binfp, mfp, hfp;
    foreach (QString b, bin) {
        binfp = QString("%1/%2").arg(bin_fld, b);
        QFile f(binfp);
        if(!f.open(QFile::ReadOnly)) {
            qDebug() << "Error opening file: " << binfp;
            return;
        }

        ELF elf(f.readAll());
        if (!elf.is_valid())
            return;

        ELFAddingMethods<Registers_x64> dam(&elf);

        foreach (QString m, meth) {
            mfp = QString("%1/%2").arg(dmeth_fld, m);
            foreach (QString h, hand) {
                hfp = QString("%1/%2").arg(handl_fld, h);

                // ===========================
                // debugger handler generation
                // ===========================
                code.setFileName(hfp);
                if (!code.open(QIODevice::ReadOnly))
                    return;
                handler.code = code.readAll();
                code.close();

                // =============================
                // debugger detection generation
                // =============================
                code.setFileName(mfp);
                if (!code.open(QIODevice::ReadOnly))
                    return;

                oepaction.code = code.readAll();
                code.close();

                // FOR CC code
                QPair<QByteArray, Elf64_Addr> section_data;
                if (!elf.get_section_content(ELF::SectionType::TEXT, section_data))
                    return;

                oepaction.static_params = { { "vsize", QString::number(section_data.first.size()) },
                                            { "vaddr", QString::number(section_data.second) } };

                // ==============================
                // wrapper information generation
                // ==============================
                code.setFileName(mode);
                if (!code.open(QIODevice::ReadOnly))
                    return;

                oepwrapper.code = code.readAll();
                oepwrapper.ret = oepaction.ret;

                code.close();

                inject_desc.cm = DAddingMethods<Registers_x64>::CallingMethod::Trampoline;
                inject_desc.adding_method = &oepwrapper;
                inject_desc.saved_fname = QString("trampoline_%1_%2_%3").arg(b, m, h);

                if (!dam.secure({&inject_desc}))
                    return;

                system(QString("mv %1 %2").arg(inject_desc.saved_fname, out_fld).toStdString().c_str());
            }
        }
    }
}

void test_trampoline_wrapper() {
    QString dmeth_fld = "methods";
    QString bin_fld = "bin";
    QString out_fld = "secured";
    QList<QString> dmeth_x86 = { "cc.asm", "sigtrap.asm" };
    QList<QString> dmeth_x64 = { "cc64.asm" };
    QList<QString> bin_x86 = { "my32", "derby32" };
    QList<QString> bin_x64 = { "my64", "derby64", "edb", "dDeflect" };
    QString mode_x86 = "single.asm",
            mode_x64 = "single64.asm";

    QList<QString> handlers_x86 = { "exit.asm" };
    QList<QString> handlers_x64 = { "exit64.asm" };
    QString handl_fld = "handlers";

    qDebug() << "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n"
             << "Testing Trampoline Wrapper x86\n"
             << "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n";

    test_trampoline_wrapper_x86(bin_fld, dmeth_fld, out_fld, bin_x86, dmeth_x86, mode_x86, handl_fld, handlers_x86);

    qDebug() << "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n"
             << "Finished Trampoline Wrapper x64\n"
             << "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n";

    qDebug() << "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n"
             << "Testing Trampoline Wrapper x64\n"
             << "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n";

    test_trampoline_wrapper_x64(bin_fld, dmeth_fld, out_fld, bin_x64, dmeth_x64, mode_x64, handl_fld, handlers_x64);

    qDebug() << "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n"
             << "Finished Trampoline Wrapper x64\n"
             << "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n";
}
