#include "peaddingmethods.h"

PEAddingMethods::PEAddingMethods(PEFile *f) :
    DAddingMethods(f),
    codeCoverage(5),
    gen(std::chrono::system_clock::now().time_since_epoch().count())
{

}

PEAddingMethods::~PEAddingMethods()
{

}

void PEAddingMethods::setCodeCoverage(uint8_t new_coverage)
{
    codeCoverage = new_coverage > 100 ? 100 : new_coverage;
}


template <typename Register>
bool PEAddingMethods::injectCode(QList<InjectDescription<Register> *> descs)
{
    QList<uint64_t> epMethods, tlsMethods, tramMethods;

    codePointers.clear();
    relocations.clear();

    PEFile *pe = dynamic_cast<PEFile*>(file);
    if(!pe)
        return false;

    if(!pe->is_valid())
        return false;

    text_section = pe->getTextSection();
    text_section_offset = pe->getTextSectionOffset();

    foreach(InjectDescription<Register> *desc, descs)
    {
        if(!desc)
            return false;

        switch(desc->cm)
        {
        case CallingMethod::OEP:
            epMethods.append(generateCode<Register>(desc->adding_method));
            if(epMethods.last() == 0)
                return false;
            break;

        case CallingMethod::TLS:
            tlsMethods.append(generateCode<Register>(desc->adding_method, true));
            if(tlsMethods.last() == 0)
                return false;
            break;

        case CallingMethod::Trampoline:
            tramMethods.append(generateCode<Register>(desc->adding_method));
            if(tramMethods.last() == 0)
                return false;
            break;

        default:
            return false;
        }
    }

    if(!tramMethods.empty())
    {
        if(!injectTrampolineCode<Register>(tramMethods))
            return false;
    }

    if(!epMethods.empty())
    {
        if(!injectEpCode<Register>(epMethods))
            return false;
    }

    if(!tlsMethods.empty())
    {
        if(!injectTlsCode<Register>(tlsMethods))
            return false;
    }

    return pe->addRelocations(relocations);
}
template bool PEAddingMethods::injectCode(QList<InjectDescription<Registers_x86> *> descs);
template bool PEAddingMethods::injectCode(QList<InjectDescription<Registers_x64> *> descs);


template <typename Register>
uint64_t PEAddingMethods::generateCode(Wrapper<Register> *w, bool isTlsCallback)
{
    PEFile *pe = dynamic_cast<PEFile*>(file);
    if(!pe)
        return 0;

    if(!w)
        return 0;

    uint64_t action = 0;
    uint64_t thread = 0;

    // Generowanie kodu dla akcji.
    if(w->detect_handler)
    {
        action = generateCode(w->detect_handler);
        if(!action) return 0;
    }

    // Generowanie kodu dla funkcji wątku.
    ThreadWrapper<Register> *tw = dynamic_cast<ThreadWrapper<Register>*>(w);

    if(tw && tw->thread_actions.empty())
        return 0;

    if(tw && !tw->thread_actions.empty())
    {
        thread = generateThreadCode(tw->thread_actions, tw->sleep_time);
        if(!thread) return 0;
    }

    // Generowanie kodu
    BinaryCode<Register> code;

    // Tworzenie ramki stosu
    code.append(CodeDefines<Register>::startFunc);

    std::list<Register> rts = w->used_regs.toStdList();
    bool align = false;

    // Zachowywanie rejestrów
    for(auto it = rts.begin(); it != rts.end(); ++it)
    {
        if(CodeDefines<Register>::externalRegs.contains(*it))
        {
            code.append(CodeDefines<Register>::saveRegister(*it));
            align = !align;
        }
    }

    // Wyrównanie do 16 w przypadku x64
    if(std::is_same<Register, Registers_x64>::value && align)
        code.append(CodeDefines<Register>::reserveStackSpace(CodeDefines<Register>::align16Size));

    // Ładowanie parametrów
    if(!w->dynamic_params.empty())
    {
        // TODO
        Wrapper<Register> *func_wrap;// = Wrapper<Register>::fromFile(Wrapper<Register>::helpersPath + "load_functions.asm");
        if(!func_wrap)
            return 0;

        uint64_t get_functions = generateCode(func_wrap);
        delete func_wrap;

        if(!get_functions)
            return 0;

        if(!generateParametersLoadingCode<Register>(code, get_functions, w->dynamic_params, thread))
            return 0;
    }

    // Doklejanie właściwego kodu
    QByteArray binCode = compileCode(w->code);
    if(binCode.length() == 0)
        return 0;

    code.append(binCode);

    // Handler
    if(action)
    {
        Register cond = w->ret;
        int act_idx = CodeDefines<Register>::internalRegs.indexOf(cond);
        Register act = act_idx == -1 ? CodeDefines<Register>::internalRegs[0] :
                CodeDefines<Register>::internalRegs[(act_idx + 1) % CodeDefines<Register>::internalRegs.length()];

        generateActionConditionCode<Register>(code, action, cond, act);
    }

    // Wyrównanie do 16 w przypadku x64
    if(std::is_same<Register, Registers_x64>::value && align)
        code.append(CodeDefines<Register>::clearStackSpace(CodeDefines<Register>::align16Size));

    // Przywracanie rejestrów
    for(auto it = rts.rbegin(); it != rts.rend(); ++it)
    {
        if(CodeDefines<Register>::externalRegs.contains(*it))
            code.append(CodeDefines<Register>::restoreRegister(*it));
    }

    // Niszczenie ramki stosu i ret
    code.append(CodeDefines<Register>::endFunc);
    if(isTlsCallback && !std::is_same<Register, Registers_x64>::value)
        code.append(CodeDefines<Register>::retN(3 * CodeDefines<Register>::stackCellSize));
    else
        code.append(CodeDefines<Register>::ret);

    return pe->injectUniqueData(code, codePointers, relocations);
}

template <>
bool PEAddingMethods::injectEpCode<Registers_x86>(QList<uint64_t> &epMethods)
{
    typedef Registers_x86 Register;

    PEFile *pe = dynamic_cast<PEFile*>(file);
    if(!pe)
        return false;

    BinaryCode<Register> code;

    // Wywołanie każdej z metod
    foreach(uint64_t offset, epMethods)
    {
        code.append(CodeDefines<Register>::movValueToReg(offset, Register::EAX), true);
        code.append(CodeDefines<Register>::callReg(Register::EAX));
    }

    // Skok do Entry Point
    code.append(CodeDefines<Register>::movValueToReg(pe->getEntryPoint() + pe->getImageBase(), Register::EAX), true);
    code.append(CodeDefines<Register>::jmpReg(Register::EAX));

    uint64_t new_ep = pe->injectUniqueData(code, codePointers, relocations);

    if(new_ep == 0)
        return false;

    new_ep -= pe->getImageBase();

    if(!pe->setNewEntryPoint(new_ep))
        return false;

    return true;
}

template <>
bool PEAddingMethods::injectEpCode<Registers_x64>(QList<uint64_t> &epMethods)
{
    typedef Registers_x64 Register;

    PEFile *pe = dynamic_cast<PEFile*>(file);
    if(!pe)
        return false;

    BinaryCode<Register> code;

    // Alokacja Shadow Space
    code.append(CodeDefines<Register>::reserveStackSpace(CodeDefines<Register>::shadowSize));

    // Wywołanie każdej z metod
    foreach(uint64_t offset, epMethods)
    {
        code.append(CodeDefines<Register>::movValueToReg(offset, Register::RAX), true);
        code.append(CodeDefines<Register>::callReg(Register::RAX));
    }

    // Usunięcie Shadow Space
    code.append(CodeDefines<Register>::clearStackSpace(CodeDefines<Register>::shadowSize));

    // Skok do Entry Point
    code.append(CodeDefines<Register>::movValueToReg(pe->getEntryPoint() + pe->getImageBase(), Register::RAX), true);
    code.append(CodeDefines<Register>::jmpReg(Register::RAX));

    uint64_t new_ep = pe->injectUniqueData(code, codePointers, relocations);

    if(new_ep == 0)
        return false;

    new_ep -= pe->getImageBase();

    if(!pe->setNewEntryPoint(new_ep))
        return false;

    return true;
}

template <typename Register>
bool PEAddingMethods::injectTlsCode(QList<uint64_t> &tlsMethods)
{
    PEFile *pe = dynamic_cast<PEFile*>(file);
    if(!pe)
        return false;

    // Tworzenie tablicy TLS jeśli nie istnieje
    if(!pe->hasTls())
    {
        QByteArray new_tls(pe->getImageTlsDirectorySize(), '\x00');
        uint64_t addr = pe->injectUniqueData(new_tls, codePointers);

        if(addr == 0)
            return false;

        addr -= pe->getImageBase();

        pe->setTlsDirectoryAddress(addr);
    }

    QByteArray tlsCallbacks;

    // Zapisanie istniejących callbacków
    if(pe->getTlsAddressOfCallBacks() != 0)
    {
        QList<uint64_t> clbks = pe->getTlsCallbacks();
        foreach(uint64_t value, clbks)
        {
            tlsCallbacks.append(reinterpret_cast<const char*>(&value), CodeDefines<Register>::stackCellSize);
        }
    }
    else
    {
        // Adres tablicy z callbackami nie istniał. Trzeba dodać do relokacji.
        relocations.append(pe->getTlsDirectoryAddress() + pe->getImageBase() +
                           3 * CodeDefines<Register>::stackCellSize);
    }

    // Dodanie nowych callbacków
    foreach(uint64_t cbk, tlsMethods)
        tlsCallbacks.append(reinterpret_cast<const char*>(&cbk), CodeDefines<Register>::stackCellSize);

    // Dodanie NULL-byte i pola do zapisu TLSIndex
    tlsCallbacks.append(QByteArray(CodeDefines<Register>::stackCellSize * 2, '\x00'));

    uint64_t cb_pointer = pe->injectUniqueData(tlsCallbacks, codePointers);

    if(cb_pointer == 0)
        return false;

    for(uint64_t i = 0; i < static_cast<uint64_t>(tlsCallbacks.length() - CodeDefines<Register>::stackCellSize * 2);
        i += CodeDefines<Register>::stackCellSize)
    {
        relocations.append(i + cb_pointer);
    }

    pe->setTlsAddressOfCallBacks(cb_pointer);
    if(pe->getTlsAddressOfIndex() == 0)
    {
        relocations.append(pe->getTlsDirectoryAddress() + pe->getImageBase() +
                           2 * CodeDefines<Register>::stackCellSize);
        pe->setTlsAddressOfIndex(cb_pointer + tlsCallbacks.length() - CodeDefines<Register>::stackCellSize);
    }

    return true;
}
template bool PEAddingMethods::injectTlsCode<Registers_x86>(QList<uint64_t> &tlsMethods);
template bool PEAddingMethods::injectTlsCode<Registers_x64>(QList<uint64_t> &tlsMethods);


template <typename Register>
bool PEAddingMethods::injectTrampolineCode(QList<uint64_t> &tramMethods)
{
    PEFile *pe = dynamic_cast<PEFile*>(file);
    if(!pe)
        return false;

    QTemporaryFile temp_file;
    if(!temp_file.open())
        return false;

    temp_file.write(text_section);
    temp_file.flush();

    QProcess ndisasm;

    ndisasm.setProcessChannelMode(QProcess::MergedChannels);
    // TODO
    //ndisasm.start(Wrapper<Register>::ndisasmPath, {"-a", "-b", pe.is_x64() ? "64" : "32", QFileInfo(temp_file).absoluteFilePath()});

    if(!ndisasm.waitForStarted())
        return false;

    QByteArray assembly;

    while(ndisasm.waitForReadyRead(-1))
        assembly.append(ndisasm.readAll());

    QStringList asm_lines = QString(assembly).split(CodeDefines<Register>::newLineRegExp, QString::SkipEmptyParts);
    QStringList call_lines = asm_lines.filter(CodeDefines<Register>::callRegExp);
    QStringList jmp_lines = asm_lines.filter(CodeDefines<Register>::jmpRegExp);

    QList<uint32_t> fileOffsets;
    getFileOffsetsFromOpcodes(call_lines, fileOffsets, text_section_offset);
    getFileOffsetsFromOpcodes(jmp_lines, fileOffsets, text_section_offset);

    std::uniform_int_distribution<int> prob(0, 99);

    int method_idx = 0;

    foreach(uint32_t offset, fileOffsets)
    {
        if(prob(gen) >= codeCoverage)
            continue;

        BinaryCode<Register> code = generateTrampolineCode<Register>(pe->getAddressAtCallInstructionOffset(offset), tramMethods[method_idx]);

        uint64_t addr = pe->injectUniqueData(code, codePointers, relocations);
        if(addr == 0)
            return false;

        pe->setAddressAtCallInstructionOffset(offset, addr);

        method_idx = (method_idx + 1) % tramMethods.length();
    }

    return true;
}

template <>
uint64_t PEAddingMethods::generateThreadCode<Registers_x86>(QList<Wrapper<Registers_x86>*> wrappers, uint16_t sleepTime)
{
    typedef Registers_x86 Register;

    PEFile *pe = dynamic_cast<PEFile*>(file);
    if(!pe)
        return false;

    BinaryCode<Register> code;

    code.append(CodeDefines<Register>::startFunc);
    int jmp_offset = 0;

    if(sleepTime)
    {
        // TODO: ścieżka z config!
        Wrapper<Register> *func_wrap;// = Wrapper<Register>::fromFile(Wrapper<Register>::helpersPath + "load_functions.asm");
        if(!func_wrap)
            return 0;

        uint32_t get_functions = generateCode(func_wrap);
        delete func_wrap;
        if(get_functions == 0)
            return 0;

        uint32_t lib_name_addr = pe->generateString("kernel32", codePointers);
        uint32_t func_name_addr = pe->generateString("Sleep", codePointers);
        if(!lib_name_addr || !func_name_addr)
            return 0;

        code.append(CodeDefines<Register>::reserveStackSpace(2));
        code.append(CodeDefines<Register>::movValueToReg(get_functions, Register::EAX), true);
        code.append(CodeDefines<Register>::callReg(Register::EAX));

        code.append(CodeDefines<Register>::restoreRegister(Register::EAX));
        code.append(CodeDefines<Register>::restoreRegister(Register::EDX));
        code.append(CodeDefines<Register>::saveRegister(Register::EAX));

        code.append(CodeDefines<Register>::storeValue(lib_name_addr), true);
        code.append(CodeDefines<Register>::callReg(Register::EDX));

        code.append(CodeDefines<Register>::restoreRegister(Register::EDX));

        code.append(CodeDefines<Register>::storeValue(func_name_addr), true);
        code.append(CodeDefines<Register>::saveRegister(Register::EAX));
        code.append(CodeDefines<Register>::callReg(Register::EDX));
        code.append(CodeDefines<Register>::saveRegister(Register::EAX));

        jmp_offset = code.length();

        // Pętla
        code.append(CodeDefines<Register>::readFromEspMemToReg(Register::EAX, 0));
        code.append(CodeDefines<Register>::storeValue(static_cast<uint32_t>(sleepTime * 1000)));
        code.append(CodeDefines<Register>::callReg(Register::EAX));
    }

    foreach(Wrapper<Register> *w, wrappers)
    {
        if(!w)
            return 0;

        uint32_t fnc = generateCode(w);
        code.append(CodeDefines<Register>::movValueToReg(fnc, Register::EAX), true);
        code.append(CodeDefines<Register>::callReg(Register::EAX));
    }

    if(sleepTime)
    {
        jmp_offset = code.length() + 2 - jmp_offset;
        if(jmp_offset > 126)
            return 0;

        code.append(CodeDefines<Register>::jmpRelative(-jmp_offset));
        code.append(CodeDefines<Register>::clearStackSpace(1));
    }

    code.append(CodeDefines<Register>::movValueToReg(0U, Register::EAX));
    code.append(CodeDefines<Register>::endFunc);
    code.append(CodeDefines<Register>::retN(4));

    return pe->injectUniqueData(code, codePointers, relocations);
}

template <>
uint64_t PEAddingMethods::generateThreadCode<Registers_x64>(QList<Wrapper<Registers_x64>*> wrappers, uint16_t sleepTime)
{
    typedef Registers_x64 Register;

    PEFile *pe = dynamic_cast<PEFile*>(file);
    if(!pe)
        return false;

    BinaryCode<Register> code;

    code.append(CodeDefines<Register>::startFunc);

    int jmp_offset = 0;

    if(sleepTime)
    {
        // TODO
        Wrapper<Register> *func_wrap;// = Wrapper<Register>::fromFile(Wrapper<Register>::helpersPath + "load_functions.asm");
        if(!func_wrap)
            return 0;

        uint64_t get_functions = generateCode(func_wrap);
        delete func_wrap;
        if(get_functions == 0)
            return 0;

        uint64_t lib_name_addr = pe->generateString("kernel32", codePointers);
        uint64_t func_name_addr = pe->generateString("Sleep", codePointers);
        if(!lib_name_addr || !func_name_addr)
            return 0;

        // Alokacja Shadow Space. W pierwszych 2 komórkach znajdą się adresy LoadLibrary i GetProcAddr
        code.append(CodeDefines<Register>::reserveStackSpace(CodeDefines<Register>::shadowSize));

        // Pobieranie adresów
        code.append(CodeDefines<Register>::movValueToReg(get_functions, Register::RAX), true);
        code.append(CodeDefines<Register>::callReg(Register::RAX));

        // Shadow Space dla LoadLibrary i GetProcAddr
        code.append(CodeDefines<Register>::reserveStackSpace(CodeDefines<Register>::shadowSize));

        // Wczytywanie adresu LoadLibrary
        code.append(CodeDefines<Register>::readFromEspMemToReg(Register::RDX, (CodeDefines<Register>::shadowSize + 1) * CodeDefines<Register>::stackCellSize));

        // Wywoływanie LoadLibrary
        code.append(CodeDefines<Register>::movValueToReg(lib_name_addr, Register::RCX), true);
        code.append(CodeDefines<Register>::callReg(Register::RDX));

        // Wczytywanie adresu GetProcAddr
        code.append(CodeDefines<Register>::readFromEspMemToReg(Register::R8, (CodeDefines<Register>::shadowSize) * CodeDefines<Register>::stackCellSize));

        // Wywołanie GetProcAddr
        code.append(CodeDefines<Register>::saveRegister(Register::RAX));
        code.append(CodeDefines<Register>::restoreRegister(Register::RCX));
        code.append(CodeDefines<Register>::movValueToReg(func_name_addr, Register::RDX), true);
        code.append(CodeDefines<Register>::callReg(Register::R8));

        // Usunięcie Shadow Space dla LoadLibrary i GetProcAddr
        code.append(CodeDefines<Register>::clearStackSpace(CodeDefines<Register>::shadowSize));

        // Odłożenie adresu Sleep w Shadow Space
        code.append(CodeDefines<Register>::readFromRegToEspMem(Register::RAX, 0));

        jmp_offset = code.length();

        // Pętla
        code.append(CodeDefines<Register>::readFromEspMemToReg(Register::RAX, 0));
        code.append(CodeDefines<Register>::movValueToReg(static_cast<uint64_t>(sleepTime * 1000), Register::RCX));

        // Sleep
        code.append(CodeDefines<Register>::reserveStackSpace(CodeDefines<Register>::shadowSize));
        code.append(CodeDefines<Register>::callReg(Register::RAX));
        code.append(CodeDefines<Register>::clearStackSpace(CodeDefines<Register>::shadowSize));
    }

    code.append(CodeDefines<Register>::reserveStackSpace(CodeDefines<Register>::shadowSize));

    foreach(Wrapper<Register> *w, wrappers)
    {
        if(!w)
            return 0;

        uint64_t fnc = generateCode(w);
        code.append(CodeDefines<Register>::movValueToReg(fnc, Register::RAX), true);
        code.append(CodeDefines<Register>::callReg(Register::RAX));
    }

    code.append(CodeDefines<Register>::clearStackSpace(CodeDefines<Register>::shadowSize));

    if(sleepTime)
    {
        jmp_offset = code.length() + 2 - jmp_offset;
        if(jmp_offset > 126)
            return 0;

        code.append(CodeDefines<Register>::jmpRelative(-jmp_offset));
        code.append(CodeDefines<Register>::clearStackSpace(CodeDefines<Register>::shadowSize));
    }

    code.append(CodeDefines<Register>::movValueToReg(static_cast<uint64_t>(0), Register::RAX));
    code.append(CodeDefines<Register>::endFunc);
    code.append(CodeDefines<Register>::ret);

    return pe->injectUniqueData(code, codePointers, relocations);
}

template <>
bool PEAddingMethods::generateParametersLoadingCode<Registers_x86, uint32_t>
(BinaryCode<Registers_x86> &code, uint32_t getFunctionsCodeAddr, QMap<Registers_x86, QString> params, uint32_t threadCodePtr)
{
    typedef Registers_x86 Reg;

    PEFile *pe = dynamic_cast<PEFile*>(file);
    if(!pe)
        return false;

    // Rezerwowanie miejsca na GetProcAddr, LoadLibrary i tmp
    code.append(CodeDefines<Reg>::reserveStackSpace(3));

    // Wczytywanie adresów GetProcAddr i LoadLibrary
    code.append(CodeDefines<Reg>::movValueToReg(getFunctionsCodeAddr, Reg::EAX), true);
    code.append(CodeDefines<Reg>::callReg(Reg::EAX));

    // Wczytywanie wymaganych adresów do rejestrów
    QList<Reg> keys = params.keys();
    foreach (Reg r, keys)
    {
        QList<QString> func_name = params[r].split('!');
        if(func_name.length() != 2)
            return false;

        // Jeżeli szukana wartość jest adresem funkcji wątku to przypisujemy
        if(threadCodePtr && func_name[0] == "THREAD" && func_name[1] == "THREAD")
        {
            code.append(CodeDefines<Reg>::movValueToReg(threadCodePtr, r), true);
            continue;
        }

        // Generowanie nazw biblioteki i funkcji
        uint32_t lib_name_addr = pe->generateString(func_name[0], codePointers);
        uint32_t func_name_addr = pe->generateString(func_name[1], codePointers);

        // Zachowywanie wypełnionych już wcześniej rejestrów
        code.append(CodeDefines<Reg>::saveAllInternal());

        int internalSize = CodeDefines<Reg>::internalRegs.length();

        // Wywołanie LoadLibrary
        code.append(CodeDefines<Reg>::storeValue(lib_name_addr), true);
        code.append(CodeDefines<Reg>::readFromEspMemToReg(Reg::EAX, (2 + internalSize) * CodeDefines<Reg>::stackCellSize));
        code.append(CodeDefines<Reg>::callReg(Reg::EAX));

        // Wywołanie GetProcAddr
        code.append(CodeDefines<Reg>::storeValue(func_name_addr), true);
        code.append(CodeDefines<Reg>::saveRegister(Reg::EAX));
        code.append(CodeDefines<Reg>::readFromEspMemToReg(Reg::EAX, (2 + internalSize) * CodeDefines<Reg>::stackCellSize));
        code.append(CodeDefines<Reg>::callReg(Reg::EAX));

        // Zapisanie adresu szukanej funkcji do tmp
        code.append(CodeDefines<Reg>::readFromRegToEspMem(Reg::EAX, (2 + internalSize) * CodeDefines<Reg>::stackCellSize));

        // Odtworzenie wypełnionych wcześniej rejestrów
        code.append(CodeDefines<Reg>::restoreAllInternal());

        // Przypisanie znalezionego adresu szukanej funkcji
        code.append(CodeDefines<Reg>::readFromEspMemToReg(r, 2 * CodeDefines<Reg>::stackCellSize));
    }

    code.append(CodeDefines<Reg>::clearStackSpace(3));

    return true;
}

template <>
bool PEAddingMethods::generateParametersLoadingCode<Registers_x64, uint64_t>
(BinaryCode<Registers_x64> &code, uint64_t getFunctionsCodeAddr, QMap<Registers_x64, QString> params, uint64_t threadCodePtr)
{
    typedef Registers_x64 Reg;

    PEFile *pe = dynamic_cast<PEFile*>(file);
    if(!pe)
        return false;

    // Rezerwowanie miejsca na GetProcAddr, LoadLibrary i tmp z wyrównaniem do 16
    code.append(CodeDefines<Reg>::reserveStackSpace(4));

    // Wczytywanie adresów GetProcAddr i LoadLibrary
    code.append(CodeDefines<Reg>::movValueToReg(getFunctionsCodeAddr, Reg::RAX), true);
    code.append(CodeDefines<Reg>::callReg(Reg::RAX));

    // Wczytywanie wymaganych adresów do rejestrów
    QList<Reg> keys = params.keys();
    foreach (Reg r, keys)
    {
        QList<QString> func_name = params[r].split('!');
        if(func_name.length() != 2)
            return false;

        // Jeżeli szukana wartość jest adresem funkcji wątku to przypisujemy
        if(threadCodePtr && func_name[0] == "THREAD" && func_name[1] == "THREAD")
        {
            code.append(CodeDefines<Reg>::movValueToReg(threadCodePtr, r), true);
            continue;
        }

        // Generowanie nazw biblioteki i funkcji
        uint64_t lib_name_addr = pe->generateString(func_name[0], codePointers);
        uint64_t func_name_addr = pe->generateString(func_name[1], codePointers);

        // Zapisanie wszystkich wypełnionych wcześniej rejestrów + wyrównanie do 16
        code.append(CodeDefines<Reg>::saveAllInternal());
        int internalSize = CodeDefines<Reg>::internalRegs.length();
        if(CodeDefines<Reg>::internalRegs.length() % 2 != 0)
        {
            code.append(CodeDefines<Reg>::reserveStackSpace(CodeDefines<Reg>::align16Size));
            internalSize++;
        }

        code.append(CodeDefines<Reg>::reserveStackSpace(CodeDefines<Reg>::shadowSize));
        internalSize += CodeDefines<Reg>::shadowSize;

        // Wywołanie LoadLibrary
        code.append(CodeDefines<Reg>::movValueToReg(lib_name_addr, Reg::RCX), true);
        code.append(CodeDefines<Reg>::readFromEspMemToReg(Reg::RAX, (1 + internalSize) * CodeDefines<Reg>::stackCellSize));
        code.append(CodeDefines<Reg>::callReg(Reg::RAX));

        // Wywołanie GetProcAddr
        code.append(CodeDefines<Reg>::saveRegister(Reg::RAX));
        code.append(CodeDefines<Reg>::restoreRegister(Reg::RCX));
        code.append(CodeDefines<Reg>::movValueToReg(func_name_addr, Reg::RDX), true);
        code.append(CodeDefines<Reg>::readFromEspMemToReg(Reg::RAX, (internalSize) * CodeDefines<Reg>::stackCellSize));
        code.append(CodeDefines<Reg>::callReg(Reg::RAX));

        // Zapisanie znalezionej wartości jako tmp
        code.append(CodeDefines<Reg>::readFromRegToEspMem(Reg::RAX, (2 + internalSize) * CodeDefines<Reg>::stackCellSize));

        code.append(CodeDefines<Reg>::clearStackSpace(CodeDefines<Reg>::shadowSize));
        if(CodeDefines<Reg>::internalRegs.length() % 2 != 0)
            code.append(CodeDefines<Reg>::clearStackSpace(CodeDefines<Reg>::align16Size));

        code.append(CodeDefines<Reg>::restoreAllInternal());

        // Odtworzenie zapisanej wartości adresu szukanej funkcji i przypisanie do rejestru
        code.append(CodeDefines<Reg>::readFromEspMemToReg(r, 2 * CodeDefines<Reg>::stackCellSize));
    }

    code.append(CodeDefines<Reg>::clearStackSpace(4));

    return true;
}

template <>
bool PEAddingMethods::generateParametersLoadingCode<Registers_x86, uint64_t>
(BinaryCode<Registers_x86> &code, uint64_t getFunctionsCodeAddr, QMap<Registers_x86, QString> params, uint64_t threadCodePtr)
{
    return generateParametersLoadingCode<Registers_x86, uint32_t>
            (code, static_cast<uint32_t>(getFunctionsCodeAddr), params, static_cast<uint32_t>(threadCodePtr));
}

template <typename Register>
bool PEAddingMethods::generateActionConditionCode(BinaryCode<Register> &code, uint64_t action, Register cond, Register act)
{
    PEFile *pe = dynamic_cast<PEFile*>(file);
    if(!pe)
        return false;

    code.append(CodeDefines<Register>::movValueToReg(action, act), true);
    code.append(CodeDefines<Register>::testReg(cond));

    QByteArray call_code;
    call_code.append(CodeDefines<Register>::saveAllInternal());

    if(pe->is_x64() && CodeDefines<Register>::internalRegs.length() % 2 != 0)
        call_code.append(CodeDefines<Register>::reserveStackSpace(CodeDefines<Register>::align16Size));

    call_code.append(CodeDefines<Register>::callReg(act));

    if(pe->is_x64() && CodeDefines<Register>::internalRegs.length() % 2 != 0)
        call_code.append(CodeDefines<Register>::clearStackSpace(CodeDefines<Register>::align16Size));

    call_code.append(CodeDefines<Register>::restoreAllInternal());


    code.append(CodeDefines<Register>::jzRelative(call_code.length()));
    code.append(call_code);

    return true;
}
template bool PEAddingMethods::generateActionConditionCode(BinaryCode<Registers_x86> &code, uint64_t action, Registers_x86 cond, Registers_x86 act);
template bool PEAddingMethods::generateActionConditionCode(BinaryCode<Registers_x64> &code, uint64_t action, Registers_x64 cond, Registers_x64 act);

void PEAddingMethods::getFileOffsetsFromOpcodes(QStringList &opcodes, QList<uint32_t> &fileOffsets, uint32_t baseOffset)
{
    foreach(QString op, opcodes)
        fileOffsets.append(op.mid(0, 8).toUInt(NULL, 16) + baseOffset + 1);
}

QByteArray PEAddingMethods::compileCode(QByteArray code)
{
    QByteArray bin;

    QTemporaryFile temp_file;
    if(!temp_file.open())
        return bin;

    temp_file.write(code);
    temp_file.flush();

    QProcess nasm;

    nasm.setProcessChannelMode(QProcess::MergedChannels);
    // TODO
    //nasm.start(Wrapper<Register>::nasmPath, {"-f", "bin", "-o", "~tmpfile.bin", QFileInfo(temp_file).absoluteFilePath()});

    if(!nasm.waitForFinished())
        return bin;

    temp_file.close();

    QFile f("~tmpfile.bin");

    if(!f.open(QFile::ReadOnly))
        return bin;

    bin = f.readAll();

    f.close();

    return bin;
}

template <>
uint8_t PEAddingMethods::getRandomRegister<Registers_x86>()
{
    typedef Registers_x86 Reg;
    QList<Reg> r = {Reg::EAX, Reg::ECX, Reg::EDX, Reg::EBX, Reg::ESI, Reg::EDI};

    std::uniform_int_distribution<int> idx(0, r.length() - 1);

    return static_cast<uint8_t>(r[idx(gen)]);
}

template <>
uint8_t PEAddingMethods::getRandomRegister<Registers_x64>()
{
    typedef Registers_x64 Reg;
    QList<Reg> r = {Reg::RAX, Reg::RDX, Reg::RCX, Reg::RBX, Reg::RSI, Reg::RDI,
                    Reg::R8, Reg::R9, Reg::R10, Reg::R11, Reg::R12, Reg::R13, Reg::R14, Reg::R15};

    std::uniform_int_distribution<int> idx(0, r.length() - 1);

    return static_cast<uint8_t>(r[idx(gen)]);
}

template <>
BinaryCode<Registers_x86> PEAddingMethods::generateTrampolineCode<Registers_x86>(uint64_t realAddr, uint64_t wrapperAddr)
{
    typedef Registers_x86 Register;

    BinaryCode<Register> code;

    code.append(CodeDefines<Register>::storeValue(static_cast<uint32_t>(realAddr)), true);

    code.append(CodeDefines<Register>::saveAll());
    Register r = static_cast<Register>(getRandomRegister<Register>());
    code.append(CodeDefines<Register>::movValueToReg(wrapperAddr, r), true);
    code.append(CodeDefines<Register>::callReg(r));
    code.append(CodeDefines<Register>::restoreAll());

    code.append(CodeDefines<Register>::ret);

    return code;
}

template <>
BinaryCode<Registers_x64> PEAddingMethods::generateTrampolineCode<Registers_x64>(uint64_t realAddr, uint64_t wrapperAddr)
{
    typedef Registers_x64 Register;

    BinaryCode<Register> code;

    Register r = static_cast<Register>(getRandomRegister<Register>());
    code.append(CodeDefines<Register>::reserveStackSpace(CodeDefines<Register>::align16Size));
    code.append(CodeDefines<Register>::saveRegister(r));
    code.append(CodeDefines<Register>::movValueToReg(realAddr, r), true);
    code.append(CodeDefines<Register>::readFromRegToEspMem(r, CodeDefines<Register>::stackCellSize));
    code.append(CodeDefines<Register>::restoreRegister(r));

    code.append(CodeDefines<Register>::saveAll());
    code.append(CodeDefines<Register>::reserveStackSpace(CodeDefines<Register>::shadowSize));
    r = static_cast<Register>(getRandomRegister<Register>());
    code.append(CodeDefines<Register>::movValueToReg(wrapperAddr, r), true);
    code.append(CodeDefines<Register>::callReg(r));
    code.append(CodeDefines<Register>::clearStackSpace(CodeDefines<Register>::shadowSize));
    code.append(CodeDefines<Register>::restoreAll());

    code.append(CodeDefines<Register>::ret);

    return code;
}
