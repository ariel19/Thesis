#include "peaddingmethods.h"

PEAddingMethods::PEAddingMethods() :
    gen(std::chrono::system_clock::now().time_since_epoch().count())
{

}

PEAddingMethods::~PEAddingMethods()
{

}


template <typename Register>
bool PEAddingMethods::injectCode(PEFile &pe, QList<InjectDescription<Register> *> descs, uint8_t codeCoverage)
{
    QMap<QByteArray, uint64_t> codePointers;
    QList<uint64_t> relocations;
    QList<uint64_t> epMethods, tlsMethods, tramMethods;

    if(!pe.isValid())
        return false;

    QByteArray text_section = pe.getTextSection();
    uint32_t text_section_offset = pe.getTextSectionOffset();

    foreach(InjectDescription<Register> *desc, descs)
    {
        if(!desc)
            return false;

        switch(desc->cm)
        {
        case CallingMethod::OEP:
            epMethods.append(generateCode<Register>(pe, desc->adding_method, codePointers, relocations));
            if(epMethods.last() == 0)
                return false;
            break;

        case CallingMethod::TLS:
            tlsMethods.append(generateCode<Register>(pe, desc->adding_method, codePointers, relocations, true));
            if(tlsMethods.last() == 0)
                return false;
            break;

        case CallingMethod::Trampoline:
            tramMethods.append(generateCode<Register>(pe, desc->adding_method, codePointers, relocations));
            if(tramMethods.last() == 0)
                return false;
            break;

        default:
            return false;
        }
    }

    if(!tramMethods.empty())
    {
        if(!injectTrampolineCode<Register>(pe, tramMethods, codePointers, relocations, text_section, text_section_offset, codeCoverage))
            return false;
    }

    if(!epMethods.empty())
    {
        if(!injectEpCode<Register>(pe, epMethods, codePointers, relocations))
            return false;
    }

    if(!tlsMethods.empty())
    {
        if(!injectTlsCode<Register>(pe, tlsMethods, codePointers, relocations))
            return false;
    }

    return pe.addRelocations(relocations);
}
template bool PEAddingMethods::injectCode(PEFile &pe, QList<InjectDescription<Registers_x86> *> descs, uint8_t codeCoverage);
template bool PEAddingMethods::injectCode(PEFile &pe, QList<InjectDescription<Registers_x64> *> descs, uint8_t codeCoverage);


template <typename Register>
uint64_t PEAddingMethods::generateCode
(PEFile &pe, Wrapper<Register> *w, QMap<QByteArray, uint64_t> &ptrs, QList<uint64_t> &relocations, bool isTlsCallback)
{
    if(!w)
        return 0;

    uint64_t action = 0;
    uint64_t thread = 0;

    // Generowanie kodu dla akcji.
    if(w->detect_handler)
    {
        action = generateCode(pe, w->detect_handler, ptrs, relocations);
        if(!action) return 0;
    }

    // Generowanie kodu dla funkcji wątku.
    ThreadWrapper<Register> *tw = dynamic_cast<ThreadWrapper<Register>*>(w);

    if(tw && tw->thread_actions.empty())
        return 0;

    if(tw && !tw->thread_actions.empty())
    {
        thread = generateThreadCode(pe, tw->thread_actions, ptrs, tw->sleep_time, relocations);
        if(!thread) return 0;
    }

    // Generowanie kodu
    BinaryCode<Register> code;

    // Tworzenie ramki stosu
    code.append(PECodeDefines<Register>::startFunc);

    std::list<Register> rts = w->used_regs.toStdList();
    bool align = false;

    // Zachowywanie rejestrów
    for(auto it = rts.begin(); it != rts.end(); ++it)
    {
        if(PECodeDefines<Register>::externalRegs.contains(*it))
        {
            code.append(PECodeDefines<Register>::saveRegister(*it));
            align = !align;
        }
    }

    // Wyrównanie do 16 w przypadku x64
    if(std::is_same<Register, Registers_x64>::value && align)
        code.append(PECodeDefines<Register>::reserveStackSpace(PECodeDefines<Register>::align16Size));

    // Ładowanie parametrów
    if(!w->dynamic_params.empty())
    {
        // TODO
        Wrapper<Register> *func_wrap;// = Wrapper<Register>::fromFile(Wrapper<Register>::helpersPath + "load_functions.asm");
        if(!func_wrap)
            return 0;

        uint64_t get_functions = generateCode(pe, func_wrap, ptrs, relocations);
        delete func_wrap;

        if(!get_functions)
            return 0;

        if(!generateParametersLoadingCode<Register>(pe, code, get_functions, w->dynamic_params, ptrs, thread))
            return 0;
    }

    // Doklejanie właściwego kodu
    code.append(w->code);

    // Handler
    if(action)
    {
        Register cond = w->ret;
        int act_idx = PECodeDefines<Register>::internalRegs.indexOf(cond);
        Register act = act_idx == -1 ? PECodeDefines<Register>::internalRegs[0] :
                PECodeDefines<Register>::internalRegs[(act_idx + 1) % PECodeDefines<Register>::internalRegs.length()];

        generateActionConditionCode<Register>(pe, code, action, cond, act);
    }

    // Wyrównanie do 16 w przypadku x64
    if(std::is_same<Register, Registers_x64>::value && align)
        code.append(PECodeDefines<Register>::clearStackSpace(PECodeDefines<Register>::align16Size));

    // Przywracanie rejestrów
    for(auto it = rts.rbegin(); it != rts.rend(); ++it)
    {
        if(PECodeDefines<Register>::externalRegs.contains(*it))
            code.append(PECodeDefines<Register>::restoreRegister(*it));
    }

    // Niszczenie ramki stosu i ret
    code.append(PECodeDefines<Register>::endFunc);
    if(isTlsCallback && !std::is_same<Register, Registers_x64>::value)
        code.append(PECodeDefines<Register>::retN(3 * PECodeDefines<Register>::stackCellSize));
    else
        code.append(PECodeDefines<Register>::ret);

    return pe.injectUniqueData(code, ptrs, relocations);
}

template <>
bool PEAddingMethods::injectEpCode<Registers_x86>
(PEFile &pe, QList<uint64_t> &epMethods, QMap<QByteArray, uint64_t> &codePointers, QList<uint64_t> &relocations)
{
    typedef Registers_x86 Register;

    BinaryCode<Register> code;

    // Wywołanie każdej z metod
    foreach(uint64_t offset, epMethods)
    {
        code.append(PECodeDefines<Register>::movValueToReg(offset, Register::EAX), true);
        code.append(PECodeDefines<Register>::callReg(Register::EAX));
    }

    // Skok do Entry Point
    code.append(PECodeDefines<Register>::movValueToReg(pe.getEntryPoint() + pe.getImageBase(), Register::EAX), true);
    code.append(PECodeDefines<Register>::jmpReg(Register::EAX));

    uint64_t new_ep = pe.injectUniqueData(code, codePointers, relocations);

    if(new_ep == 0)
        return false;

    new_ep -= pe.getImageBase();

    if(!pe.setNewEntryPoint(new_ep))
        return false;

    return true;
}

template <>
bool PEAddingMethods::injectEpCode<Registers_x64>
(PEFile &pe, QList<uint64_t> &epMethods, QMap<QByteArray, uint64_t> &codePointers, QList<uint64_t> &relocations)
{
    typedef Registers_x64 Register;

    BinaryCode<Register> code;

    // Alokacja Shadow Space
    code.append(PECodeDefines<Register>::reserveStackSpace(PECodeDefines<Register>::shadowSize));

    // Wywołanie każdej z metod
    foreach(uint64_t offset, epMethods)
    {
        code.append(PECodeDefines<Register>::movValueToReg(offset, Register::RAX), true);
        code.append(PECodeDefines<Register>::callReg(Register::RAX));
    }

    // Usunięcie Shadow Space
    code.append(PECodeDefines<Register>::clearStackSpace(PECodeDefines<Register>::shadowSize));

    // Skok do Entry Point
    code.append(PECodeDefines<Register>::movValueToReg(pe.getEntryPoint() + pe.getImageBase(), Register::RAX), true);
    code.append(PECodeDefines<Register>::jmpReg(Register::RAX));

    uint64_t new_ep = pe.injectUniqueData(code, codePointers, relocations);

    if(new_ep == 0)
        return false;

    new_ep -= pe.getImageBase();

    if(!pe.setNewEntryPoint(new_ep))
        return false;

    return true;
}

template <typename Register>
bool PEAddingMethods::injectTlsCode(PEFile &pe, QList<uint64_t> &tlsMethods, QMap<QByteArray, uint64_t> &codePointers, QList<uint64_t> &relocations)
{
    // Tworzenie tablicy TLS jeśli nie istnieje
    if(!pe.hasTls())
    {
        QByteArray new_tls(pe.getImageTlsDirectorySize(), '\x00');
        uint64_t addr = pe.injectUniqueData(new_tls, codePointers);

        if(addr == 0)
            return false;

        addr -= pe.getImageBase();

        pe.setTlsDirectoryAddress(addr);
    }

    QByteArray tlsCallbacks;

    // Zapisanie istniejących callbacków
    if(pe.getTlsAddressOfCallBacks() != 0)
    {
        QList<uint64_t> clbks = pe.getTlsCallbacks();
        foreach(uint64_t value, clbks)
        {
            tlsCallbacks.append(reinterpret_cast<const char*>(&value), PECodeDefines<Register>::stackCellSize);
        }
    }
    else
    {
        // Adres tablicy z callbackami nie istniał. Trzeba dodać do relokacji.
        relocations.append(pe.getTlsDirectoryAddress() + pe.getImageBase() +
                           3 * PECodeDefines<Register>::stackCellSize);
    }

    // Dodanie nowych callbacków
    foreach(uint64_t cbk, tlsMethods)
        tlsCallbacks.append(reinterpret_cast<const char*>(&cbk), PECodeDefines<Register>::stackCellSize);

    // Dodanie NULL-byte i pola do zapisu TLSIndex
    tlsCallbacks.append(QByteArray(PECodeDefines<Register>::stackCellSize * 2, '\x00'));

    uint64_t cb_pointer = pe.injectUniqueData(tlsCallbacks, codePointers);

    if(cb_pointer == 0)
        return false;

    for(uint64_t i = 0; i < static_cast<uint64_t>(tlsCallbacks.length() - PECodeDefines<Register>::stackCellSize * 2);
        i += PECodeDefines<Register>::stackCellSize)
    {
        relocations.append(i + cb_pointer);
    }

    pe.setTlsAddressOfCallBacks(cb_pointer);
    if(pe.getTlsAddressOfIndex() == 0)
    {
        relocations.append(pe.getTlsDirectoryAddress() + pe.getImageBase() +
                           2 * PECodeDefines<Register>::stackCellSize);
        pe.setTlsAddressOfIndex(cb_pointer + tlsCallbacks.length() - PECodeDefines<Register>::stackCellSize);
    }

    return true;
}
template bool PEAddingMethods::injectTlsCode<Registers_x86>(PEFile &pe, QList<uint64_t> &tlsMethods, QMap<QByteArray, uint64_t> &codePointers, QList<uint64_t> &relocations);
template bool PEAddingMethods::injectTlsCode<Registers_x64>(PEFile &pe, QList<uint64_t> &tlsMethods, QMap<QByteArray, uint64_t> &codePointers, QList<uint64_t> &relocations);


template <typename Register>
bool PEAddingMethods::injectTrampolineCode(PEFile &pe, QList<uint64_t> &tramMethods, QMap<QByteArray, uint64_t> &codePointers,
                                  QList<uint64_t> &relocations, QByteArray text_section,
                                  uint32_t text_section_offset, uint8_t codeCoverage)
{
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

    QStringList asm_lines = QString(assembly).split(PECodeDefines<Register>::newLineRegExp, QString::SkipEmptyParts);
    QStringList call_lines = asm_lines.filter(PECodeDefines<Register>::callRegExp);
    QStringList jmp_lines = asm_lines.filter(PECodeDefines<Register>::jmpRegExp);

    QList<uint32_t> fileOffsets;
    getFileOffsetsFromOpcodes(call_lines, fileOffsets, text_section_offset);
    getFileOffsetsFromOpcodes(jmp_lines, fileOffsets, text_section_offset);

    std::uniform_int_distribution<int> prob(0, 99);

    int method_idx = 0;

    foreach(uint32_t offset, fileOffsets)
    {
        if(prob(gen) >= codeCoverage)
            continue;

        BinaryCode<Register> code = generateTrampolineCode<Register>(pe.getAddressAtCallInstructionOffset(offset), tramMethods[method_idx]);

        uint64_t addr = pe.injectUniqueData(code, codePointers, relocations);
        if(addr == 0)
            return false;

        pe.setAddressAtCallInstructionOffset(offset, addr);

        method_idx = (method_idx + 1) % tramMethods.length();
    }

    return true;
}

template <>
uint64_t PEAddingMethods::generateThreadCode<Registers_x86>
(PEFile &pe, QList<Wrapper<Registers_x86>*> wrappers, QMap<QByteArray, uint64_t> &ptrs, uint16_t sleepTime, QList<uint64_t> &relocations)
{
    typedef Registers_x86 Register;

    BinaryCode<Register> code;

    code.append(PECodeDefines<Register>::startFunc);
    int jmp_offset = 0;

    if(sleepTime)
    {
        // TODO: ścieżka z config!
        Wrapper<Register> *func_wrap;// = Wrapper<Register>::fromFile(Wrapper<Register>::helpersPath + "load_functions.asm");
        if(!func_wrap)
            return 0;

        uint32_t get_functions = generateCode(pe, func_wrap, ptrs, relocations);
        delete func_wrap;
        if(get_functions == 0)
            return 0;

        uint32_t lib_name_addr = pe.generateString("kernel32", ptrs);
        uint32_t func_name_addr = pe.generateString("Sleep", ptrs);
        if(!lib_name_addr || !func_name_addr)
            return 0;

        code.append(PECodeDefines<Register>::reserveStackSpace(2));
        code.append(PECodeDefines<Register>::movValueToReg(get_functions, Register::EAX), true);
        code.append(PECodeDefines<Register>::callReg(Register::EAX));

        code.append(PECodeDefines<Register>::restoreRegister(Register::EAX));
        code.append(PECodeDefines<Register>::restoreRegister(Register::EDX));
        code.append(PECodeDefines<Register>::saveRegister(Register::EAX));

        code.append(PECodeDefines<Register>::storeValue(lib_name_addr), true);
        code.append(PECodeDefines<Register>::callReg(Register::EDX));

        code.append(PECodeDefines<Register>::restoreRegister(Register::EDX));

        code.append(PECodeDefines<Register>::storeValue(func_name_addr), true);
        code.append(PECodeDefines<Register>::saveRegister(Register::EAX));
        code.append(PECodeDefines<Register>::callReg(Register::EDX));
        code.append(PECodeDefines<Register>::saveRegister(Register::EAX));

        jmp_offset = code.length();

        // Pętla
        code.append(PECodeDefines<Register>::readFromEspMemToReg(Register::EAX, 0));
        code.append(PECodeDefines<Register>::storeValue(static_cast<uint32_t>(sleepTime * 1000)));
        code.append(PECodeDefines<Register>::callReg(Register::EAX));
    }

    foreach(Wrapper<Register> *w, wrappers)
    {
        if(!w)
            return 0;

        uint32_t fnc = generateCode(pe, w, ptrs, relocations);
        code.append(PECodeDefines<Register>::movValueToReg(fnc, Register::EAX), true);
        code.append(PECodeDefines<Register>::callReg(Register::EAX));
    }

    if(sleepTime)
    {
        jmp_offset = code.length() + 2 - jmp_offset;
        if(jmp_offset > 126)
            return 0;

        code.append(PECodeDefines<Register>::jmpRelative(-jmp_offset));
        code.append(PECodeDefines<Register>::clearStackSpace(1));
    }

    code.append(PECodeDefines<Register>::movValueToReg(0U, Register::EAX));
    code.append(PECodeDefines<Register>::endFunc);
    code.append(PECodeDefines<Register>::retN(4));

    return pe.injectUniqueData(code, ptrs, relocations);
}

template <>
uint64_t PEAddingMethods::generateThreadCode<Registers_x64>
(PEFile &pe, QList<Wrapper<Registers_x64>*> wrappers, QMap<QByteArray, uint64_t> &ptrs, uint16_t sleepTime, QList<uint64_t> &relocations)
{
    typedef Registers_x64 Register;

    BinaryCode<Register> code;

    code.append(PECodeDefines<Register>::startFunc);

    int jmp_offset = 0;

    if(sleepTime)
    {
        // TODO
        Wrapper<Register> *func_wrap;// = Wrapper<Register>::fromFile(Wrapper<Register>::helpersPath + "load_functions.asm");
        if(!func_wrap)
            return 0;

        uint64_t get_functions = generateCode(pe, func_wrap, ptrs, relocations);
        delete func_wrap;
        if(get_functions == 0)
            return 0;

        uint64_t lib_name_addr = pe.generateString("kernel32", ptrs);
        uint64_t func_name_addr = pe.generateString("Sleep", ptrs);
        if(!lib_name_addr || !func_name_addr)
            return 0;

        // Alokacja Shadow Space. W pierwszych 2 komórkach znajdą się adresy LoadLibrary i GetProcAddr
        code.append(PECodeDefines<Register>::reserveStackSpace(PECodeDefines<Register>::shadowSize));

        // Pobieranie adresów
        code.append(PECodeDefines<Register>::movValueToReg(get_functions, Register::RAX), true);
        code.append(PECodeDefines<Register>::callReg(Register::RAX));

        // Shadow Space dla LoadLibrary i GetProcAddr
        code.append(PECodeDefines<Register>::reserveStackSpace(PECodeDefines<Register>::shadowSize));

        // Wczytywanie adresu LoadLibrary
        code.append(PECodeDefines<Register>::readFromEspMemToReg(Register::RDX, (PECodeDefines<Register>::shadowSize + 1) * PECodeDefines<Register>::stackCellSize));

        // Wywoływanie LoadLibrary
        code.append(PECodeDefines<Register>::movValueToReg(lib_name_addr, Register::RCX), true);
        code.append(PECodeDefines<Register>::callReg(Register::RDX));

        // Wczytywanie adresu GetProcAddr
        code.append(PECodeDefines<Register>::readFromEspMemToReg(Register::R8, (PECodeDefines<Register>::shadowSize) * PECodeDefines<Register>::stackCellSize));

        // Wywołanie GetProcAddr
        code.append(PECodeDefines<Register>::saveRegister(Register::RAX));
        code.append(PECodeDefines<Register>::restoreRegister(Register::RCX));
        code.append(PECodeDefines<Register>::movValueToReg(func_name_addr, Register::RDX), true);
        code.append(PECodeDefines<Register>::callReg(Register::R8));

        // Usunięcie Shadow Space dla LoadLibrary i GetProcAddr
        code.append(PECodeDefines<Register>::clearStackSpace(PECodeDefines<Register>::shadowSize));

        // Odłożenie adresu Sleep w Shadow Space
        code.append(PECodeDefines<Register>::readFromRegToEspMem(Register::RAX, 0));

        jmp_offset = code.length();

        // Pętla
        code.append(PECodeDefines<Register>::readFromEspMemToReg(Register::RAX, 0));
        code.append(PECodeDefines<Register>::movValueToReg(static_cast<uint64_t>(sleepTime * 1000), Register::RCX));

        // Sleep
        code.append(PECodeDefines<Register>::reserveStackSpace(PECodeDefines<Register>::shadowSize));
        code.append(PECodeDefines<Register>::callReg(Register::RAX));
        code.append(PECodeDefines<Register>::clearStackSpace(PECodeDefines<Register>::shadowSize));
    }

    code.append(PECodeDefines<Register>::reserveStackSpace(PECodeDefines<Register>::shadowSize));

    foreach(Wrapper<Register> *w, wrappers)
    {
        if(!w)
            return 0;

        uint64_t fnc = generateCode(pe, w, ptrs, relocations);
        code.append(PECodeDefines<Register>::movValueToReg(fnc, Register::RAX), true);
        code.append(PECodeDefines<Register>::callReg(Register::RAX));
    }

    code.append(PECodeDefines<Register>::clearStackSpace(PECodeDefines<Register>::shadowSize));

    if(sleepTime)
    {
        jmp_offset = code.length() + 2 - jmp_offset;
        if(jmp_offset > 126)
            return 0;

        code.append(PECodeDefines<Register>::jmpRelative(-jmp_offset));
        code.append(PECodeDefines<Register>::clearStackSpace(PECodeDefines<Register>::shadowSize));
    }

    code.append(PECodeDefines<Register>::movValueToReg(static_cast<uint64_t>(0), Register::RAX));
    code.append(PECodeDefines<Register>::endFunc);
    code.append(PECodeDefines<Register>::ret);

    return pe.injectUniqueData(code, ptrs, relocations);
}

template <>
bool PEAddingMethods::generateParametersLoadingCode<Registers_x86, uint32_t>
(PEFile &pe, BinaryCode<Registers_x86> &code, uint32_t getFunctionsCodeAddr, QMap<Registers_x86, QString> params,
 QMap<QByteArray, uint64_t> &ptrs, uint32_t threadCodePtr)
{
    typedef Registers_x86 Reg;

    // Rezerwowanie miejsca na GetProcAddr, LoadLibrary i tmp
    code.append(PECodeDefines<Reg>::reserveStackSpace(3));

    // Wczytywanie adresów GetProcAddr i LoadLibrary
    code.append(PECodeDefines<Reg>::movValueToReg(getFunctionsCodeAddr, Reg::EAX), true);
    code.append(PECodeDefines<Reg>::callReg(Reg::EAX));

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
            code.append(PECodeDefines<Reg>::movValueToReg(threadCodePtr, r), true);
            continue;
        }

        // Generowanie nazw biblioteki i funkcji
        uint32_t lib_name_addr = pe.generateString(func_name[0], ptrs);
        uint32_t func_name_addr = pe.generateString(func_name[1], ptrs);

        // Zachowywanie wypełnionych już wcześniej rejestrów
        code.append(PECodeDefines<Reg>::saveAllInternal());

        int internalSize = PECodeDefines<Reg>::internalRegs.length();

        // Wywołanie LoadLibrary
        code.append(PECodeDefines<Reg>::storeValue(lib_name_addr), true);
        code.append(PECodeDefines<Reg>::readFromEspMemToReg(Reg::EAX, (2 + internalSize) * PECodeDefines<Reg>::stackCellSize));
        code.append(PECodeDefines<Reg>::callReg(Reg::EAX));

        // Wywołanie GetProcAddr
        code.append(PECodeDefines<Reg>::storeValue(func_name_addr), true);
        code.append(PECodeDefines<Reg>::saveRegister(Reg::EAX));
        code.append(PECodeDefines<Reg>::readFromEspMemToReg(Reg::EAX, (2 + internalSize) * PECodeDefines<Reg>::stackCellSize));
        code.append(PECodeDefines<Reg>::callReg(Reg::EAX));

        // Zapisanie adresu szukanej funkcji do tmp
        code.append(PECodeDefines<Reg>::readFromRegToEspMem(Reg::EAX, (2 + internalSize) * PECodeDefines<Reg>::stackCellSize));

        // Odtworzenie wypełnionych wcześniej rejestrów
        code.append(PECodeDefines<Reg>::restoreAllInternal());

        // Przypisanie znalezionego adresu szukanej funkcji
        code.append(PECodeDefines<Reg>::readFromEspMemToReg(r, 2 * PECodeDefines<Reg>::stackCellSize));
    }

    code.append(PECodeDefines<Reg>::clearStackSpace(3));

    return true;
}

template <>
bool PEAddingMethods::generateParametersLoadingCode<Registers_x64, uint64_t>
(PEFile &pe, BinaryCode<Registers_x64> &code, uint64_t getFunctionsCodeAddr, QMap<Registers_x64, QString> params,
 QMap<QByteArray, uint64_t> &ptrs, uint64_t threadCodePtr)
{
    typedef Registers_x64 Reg;

    // Rezerwowanie miejsca na GetProcAddr, LoadLibrary i tmp z wyrównaniem do 16
    code.append(PECodeDefines<Reg>::reserveStackSpace(4));

    // Wczytywanie adresów GetProcAddr i LoadLibrary
    code.append(PECodeDefines<Reg>::movValueToReg(getFunctionsCodeAddr, Reg::RAX), true);
    code.append(PECodeDefines<Reg>::callReg(Reg::RAX));

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
            code.append(PECodeDefines<Reg>::movValueToReg(threadCodePtr, r), true);
            continue;
        }

        // Generowanie nazw biblioteki i funkcji
        uint64_t lib_name_addr = pe.generateString(func_name[0], ptrs);
        uint64_t func_name_addr = pe.generateString(func_name[1], ptrs);

        // Zapisanie wszystkich wypełnionych wcześniej rejestrów + wyrównanie do 16
        code.append(PECodeDefines<Reg>::saveAllInternal());
        int internalSize = PECodeDefines<Reg>::internalRegs.length();
        if(PECodeDefines<Reg>::internalRegs.length() % 2 != 0)
        {
            code.append(PECodeDefines<Reg>::reserveStackSpace(PECodeDefines<Reg>::align16Size));
            internalSize++;
        }

        code.append(PECodeDefines<Reg>::reserveStackSpace(PECodeDefines<Reg>::shadowSize));
        internalSize += PECodeDefines<Reg>::shadowSize;

        // Wywołanie LoadLibrary
        code.append(PECodeDefines<Reg>::movValueToReg(lib_name_addr, Reg::RCX), true);
        code.append(PECodeDefines<Reg>::readFromEspMemToReg(Reg::RAX, (1 + internalSize) * PECodeDefines<Reg>::stackCellSize));
        code.append(PECodeDefines<Reg>::callReg(Reg::RAX));

        // Wywołanie GetProcAddr
        code.append(PECodeDefines<Reg>::saveRegister(Reg::RAX));
        code.append(PECodeDefines<Reg>::restoreRegister(Reg::RCX));
        code.append(PECodeDefines<Reg>::movValueToReg(func_name_addr, Reg::RDX), true);
        code.append(PECodeDefines<Reg>::readFromEspMemToReg(Reg::RAX, (internalSize) * PECodeDefines<Reg>::stackCellSize));
        code.append(PECodeDefines<Reg>::callReg(Reg::RAX));

        // Zapisanie znalezionej wartości jako tmp
        code.append(PECodeDefines<Reg>::readFromRegToEspMem(Reg::RAX, (2 + internalSize) * PECodeDefines<Reg>::stackCellSize));

        code.append(PECodeDefines<Reg>::clearStackSpace(PECodeDefines<Reg>::shadowSize));
        if(PECodeDefines<Reg>::internalRegs.length() % 2 != 0)
            code.append(PECodeDefines<Reg>::clearStackSpace(PECodeDefines<Reg>::align16Size));

        code.append(PECodeDefines<Reg>::restoreAllInternal());

        // Odtworzenie zapisanej wartości adresu szukanej funkcji i przypisanie do rejestru
        code.append(PECodeDefines<Reg>::readFromEspMemToReg(r, 2 * PECodeDefines<Reg>::stackCellSize));
    }

    code.append(PECodeDefines<Reg>::clearStackSpace(4));

    return true;
}

template <>
bool PEAddingMethods::generateParametersLoadingCode<Registers_x86, uint64_t>
(PEFile &pe, BinaryCode<Registers_x86> &code, uint64_t getFunctionsCodeAddr, QMap<Registers_x86, QString> params,
 QMap<QByteArray, uint64_t> &ptrs, uint64_t threadCodePtr)
{
    return generateParametersLoadingCode<Registers_x86, uint32_t>
            (pe, code, static_cast<uint32_t>(getFunctionsCodeAddr), params, ptrs, static_cast<uint32_t>(threadCodePtr));
}

template <typename Register>
bool PEAddingMethods::generateActionConditionCode(PEFile &pe, BinaryCode<Register> &code, uint64_t action, Register cond, Register act)
{
    code.append(PECodeDefines<Register>::movValueToReg(action, act), true);
    code.append(PECodeDefines<Register>::testReg(cond));

    QByteArray call_code;
    call_code.append(PECodeDefines<Register>::saveAllInternal());

    if(pe.is_x64() && PECodeDefines<Register>::internalRegs.length() % 2 != 0)
        call_code.append(PECodeDefines<Register>::reserveStackSpace(PECodeDefines<Register>::align16Size));

    call_code.append(PECodeDefines<Register>::callReg(act));

    if(pe.is_x64() && PECodeDefines<Register>::internalRegs.length() % 2 != 0)
        call_code.append(PECodeDefines<Register>::clearStackSpace(PECodeDefines<Register>::align16Size));

    call_code.append(PECodeDefines<Register>::restoreAllInternal());


    code.append(PECodeDefines<Register>::jzRelative(call_code.length()));
    code.append(call_code);

    return true;
}
template bool PEAddingMethods::generateActionConditionCode(PEFile &pe, BinaryCode<Registers_x86> &code, uint64_t action, Registers_x86 cond, Registers_x86 act);
template bool PEAddingMethods::generateActionConditionCode(PEFile &pe, BinaryCode<Registers_x64> &code, uint64_t action, Registers_x64 cond, Registers_x64 act);

void PEAddingMethods::getFileOffsetsFromOpcodes(QStringList &opcodes, QList<uint32_t> &fileOffsets, uint32_t baseOffset)
{
    foreach(QString op, opcodes)
        fileOffsets.append(op.mid(0, 8).toUInt(NULL, 16) + baseOffset + 1);
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

    code.append(PECodeDefines<Register>::storeValue(static_cast<uint32_t>(realAddr)), true);

    code.append(PECodeDefines<Register>::saveAll());
    Register r = static_cast<Register>(getRandomRegister<Register>());
    code.append(PECodeDefines<Register>::movValueToReg(wrapperAddr, r), true);
    code.append(PECodeDefines<Register>::callReg(r));
    code.append(PECodeDefines<Register>::restoreAll());

    code.append(PECodeDefines<Register>::ret);

    return code;
}

template <>
BinaryCode<Registers_x64> PEAddingMethods::generateTrampolineCode<Registers_x64>(uint64_t realAddr, uint64_t wrapperAddr)
{
    typedef Registers_x64 Register;

    BinaryCode<Register> code;

    Register r = static_cast<Register>(getRandomRegister<Register>());
    code.append(PECodeDefines<Register>::reserveStackSpace(PECodeDefines<Register>::align16Size));
    code.append(PECodeDefines<Register>::saveRegister(r));
    code.append(PECodeDefines<Register>::movValueToReg(realAddr, r), true);
    code.append(PECodeDefines<Register>::readFromRegToEspMem(r, PECodeDefines<Register>::stackCellSize));
    code.append(PECodeDefines<Register>::restoreRegister(r));

    code.append(PECodeDefines<Register>::saveAll());
    code.append(PECodeDefines<Register>::reserveStackSpace(PECodeDefines<Register>::shadowSize));
    r = static_cast<Register>(getRandomRegister<Register>());
    code.append(PECodeDefines<Register>::movValueToReg(wrapperAddr, r), true);
    code.append(PECodeDefines<Register>::callReg(r));
    code.append(PECodeDefines<Register>::clearStackSpace(PECodeDefines<Register>::shadowSize));
    code.append(PECodeDefines<Register>::restoreAll());

    code.append(PECodeDefines<Register>::ret);

    return code;
}
