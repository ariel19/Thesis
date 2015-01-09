#include "peaddingmethods.h"

template <>
const QString PEAddingMethods<Registers_x86>::windowsApiLoadingFunction = "win_x86_helper_load_functions.json";

template <>
const QString PEAddingMethods<Registers_x64>::windowsApiLoadingFunction = "win_x64_helper_load_functions.json";

template <typename Register>
const QMap<typename PEAddingMethods<Register>::ErrorCode, QString> PEAddingMethods<Register>::errorDescriptions =
{
    { PEAddingMethods<Register>::ErrorCode::BinaryFileNoPe, "Podany plik nie jest plikiem PE!" },
    { PEAddingMethods<Register>::ErrorCode::CannotCreateTempDir, "Nie można utworzyć tymczasoweg katalogu." },
    { PEAddingMethods<Register>::ErrorCode::CannotCreateTempFile, "Nie można utworzyć tymczasowego pliku." },
    { PEAddingMethods<Register>::ErrorCode::CannotOpenCompiledFile, "Błąd podczas otwoerania skompilowanego pliku." },
    { PEAddingMethods<Register>::ErrorCode::ErrorLoadingFunctions, "Nie można załadować pomocniczego kodu ładującego funkcji z pliku .json." },
    { PEAddingMethods<Register>::ErrorCode::InvalidInjectDescription, "Opis metody jest niepoprawny." },
    { PEAddingMethods<Register>::ErrorCode::InvalidPeFile, "Plik PE nie jest poprawny!" },
    { PEAddingMethods<Register>::ErrorCode::NasmFailed, "Wywołanie programu nasm nie powiodło się." },
    { PEAddingMethods<Register>::ErrorCode::NdisasmFailed, "Wywołanie programu ndisasm nie powiodło się." },
    { PEAddingMethods<Register>::ErrorCode::NoThreadAction, "Brak zdefiniowenych metod dla wywołania wątku." },
    { PEAddingMethods<Register>::ErrorCode::NullInjectDescription, "Opis metody nie został poprawnie utworzony" },
    { PEAddingMethods<Register>::ErrorCode::NullWrapper, "Metoda nie została poprawnie wczytana." },
    { PEAddingMethods<Register>::ErrorCode::PeOperationFailed, "Operacja na pliku PE nie powiodła się." },
    { PEAddingMethods<Register>::ErrorCode::ToManyBytesForRelativeJump, "Wybran zbyt dużą liczbę metod w wątku." }
};

template <typename Register>
PEAddingMethods<Register>::PEAddingMethods(PEFile *f) :
    DAddingMethods<Register>(f),
    codeCoverage(5),
    gen(std::chrono::system_clock::now().time_since_epoch().count())
{

}
template PEAddingMethods<Registers_x86>::PEAddingMethods(PEFile *f);
template PEAddingMethods<Registers_x64>::PEAddingMethods(PEFile *f);

template <typename Register>
PEAddingMethods<Register>::~PEAddingMethods()
{

}
template PEAddingMethods<Registers_x86>::~PEAddingMethods();
template PEAddingMethods<Registers_x64>::~PEAddingMethods();

template <typename Register>
void PEAddingMethods<Register>::setCodeCoverage(uint8_t new_coverage)
{
    codeCoverage = new_coverage > 100 ? 100 : new_coverage;
}
template void PEAddingMethods<Registers_x86>::setCodeCoverage(uint8_t new_coverage);
template void PEAddingMethods<Registers_x64>::setCodeCoverage(uint8_t new_coverage);

template <typename Register>
typename PEAddingMethods<Register>::ErrorCode PEAddingMethods<Register>::safe_secure(
        const QList<typename DAddingMethods<Register>::InjectDescription*> &descs)
{
    QList<uint64_t> epMethods, tlsMethods, tramMethods;
    ErrorCode ec;

    codePointers.clear();
    relocations.clear();

    PEFile *pe = dynamic_cast<PEFile*>(DAddingMethods<Register>::file);
    if(!pe)
        return ErrorCode::BinaryFileNoPe;

    if(!pe->is_valid())
        return ErrorCode::InvalidPeFile;

    text_section = pe->getTextSection();
    text_section_offset = pe->getTextSectionOffset();

    foreach(typename DAddingMethods<Register>::InjectDescription *desc, descs)
    {
        if(!desc)
            return ErrorCode::NullInjectDescription;

        uint64_t addr = 0;

        switch(desc->cm)
        {
        case DAddingMethods<Register>::CallingMethod::OEP:
            ec = generateCode(desc->adding_method, addr);
            if(ec != ErrorCode::Success)
                return ec;
            epMethods.append(addr);
            break;

        case DAddingMethods<Register>::CallingMethod::TLS:
            ec = generateCode(desc->adding_method, addr, true);
            if(ec != ErrorCode::Success)
                return ec;
            tlsMethods.append(addr);
            break;

        case DAddingMethods<Register>::CallingMethod::Trampoline:
            ec = generateCode(desc->adding_method, addr);
            if(ec != ErrorCode::Success)
                return ec;
            tramMethods.append(addr);
            break;

        default:
            return ErrorCode::InvalidInjectDescription;
        }
    }

    if(!tramMethods.empty())
    {
        ec = injectTrampolineCode(tramMethods);
        if(ec != ErrorCode::Success)
            return ec;
    }

    if(!epMethods.empty())
    {
        ec = injectEpCode(epMethods);
        if(ec != ErrorCode::Success)
            return ec;
    }

    if(!tlsMethods.empty())
    {
        ec = injectTlsCode(tlsMethods);
        if(ec != ErrorCode::Success)
            return ec;
    }

    if(!pe->addRelocations(relocations))
        return ErrorCode::PeOperationFailed;

    return ErrorCode::Success;
}
template PEAddingMethods<Registers_x86>::ErrorCode PEAddingMethods<Registers_x86>::safe_secure(const QList<typename DAddingMethods<Registers_x86>::InjectDescription*> &descs);
template PEAddingMethods<Registers_x64>::ErrorCode PEAddingMethods<Registers_x64>::safe_secure(const QList<typename DAddingMethods<Registers_x64>::InjectDescription*> &descs);


template <typename Register>
bool PEAddingMethods<Register>::secure(const QList<typename DAddingMethods<Register>::InjectDescription *> &descs)
{
    ErrorCode err = safe_secure(descs);

    if(err != ErrorCode::Success)
        LOG_ERROR(errorDescriptions[err]);

    return err == ErrorCode::Success;
}
template bool PEAddingMethods<Registers_x86>::secure(const QList<DAddingMethods<Registers_x86>::InjectDescription *> &descs);
template bool PEAddingMethods<Registers_x64>::secure(const QList<DAddingMethods<Registers_x64>::InjectDescription *> &descs);

template <typename Register>
typename PEAddingMethods<Register>::ErrorCode PEAddingMethods<Register>::generateCode
(typename DAddingMethods<Register>::Wrapper *w, uint64_t &codePtr, bool isTlsCallback)
{
    ErrorCode ec;
    PEFile *pe = dynamic_cast<PEFile*>(DAddingMethods<Register>::file);
    if(!pe)
        return ErrorCode::BinaryFileNoPe;

    if(!w)
        return ErrorCode::NullWrapper;

    uint64_t action = 0;
    uint64_t thread = 0;

    // Generowanie kodu dla akcji.
    if(w->detect_handler)
    {
        ec = generateCode(w->detect_handler, action);
        if(ec != ErrorCode::Success)
            return ec;
    }

    // Generowanie kodu dla funkcji wątku.
    typename DAddingMethods<Register>::ThreadWrapper *tw = dynamic_cast<typename DAddingMethods<Register>::ThreadWrapper*>(w);

    if(tw && tw->thread_actions.empty())
        return ErrorCode::NoThreadAction;

    if(tw && !tw->thread_actions.empty())
    {
        ec = generateThreadCode(tw->thread_actions, thread, tw->sleep_time);
        if(ec != ErrorCode::Success) return ec;
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
        DJsonParser parser(DSettings::getSettings().getDescriptionsPath<Register>());
        typename DAddingMethods<Register>::Wrapper *func_wrap =
                parser.loadInjectDescription<Register>(windowsApiLoadingFunction);
        if(!func_wrap)
            return ErrorCode::ErrorLoadingFunctions;

        uint64_t get_functions = 0;
        ec = generateCode(func_wrap, get_functions);
        delete func_wrap;

        if(ec != ErrorCode::Success)
            return ec;

        ec = generateParametersLoadingCode(code, get_functions, w->dynamic_params, thread);
        if(ec != ErrorCode::Success)
            return ec;
    }

    // Doklejanie właściwego kodu
    QByteArray binCode;
    ec = compileCode(w->code, binCode);
    if(ec != ErrorCode::Success)
        return ec;

    code.append(binCode);

    // Handler
    if(action)
    {
        Register cond = w->ret;
        int act_idx = CodeDefines<Register>::internalRegs.indexOf(cond);
        Register act = act_idx == -1 ? CodeDefines<Register>::internalRegs[0] :
                CodeDefines<Register>::internalRegs[(act_idx + 1) % CodeDefines<Register>::internalRegs.length()];

        generateActionConditionCode(code, action, cond, act);
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

    codePtr = pe->injectUniqueData(code, codePointers, relocations);

    return codePtr == 0 ? ErrorCode::PeOperationFailed : ErrorCode::Success;
}

template <>
PEAddingMethods<Registers_x86>::ErrorCode PEAddingMethods<Registers_x86>::injectEpCode(QList<uint64_t> &epMethods)
{
    typedef Registers_x86 Register;

    PEFile *pe = dynamic_cast<PEFile*>(file);
    if(!pe)
        return ErrorCode::BinaryFileNoPe;

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
        return ErrorCode::PeOperationFailed;

    new_ep -= pe->getImageBase();

    if(!pe->setNewEntryPoint(new_ep))
        return ErrorCode::PeOperationFailed;

    return ErrorCode::Success;
}

template <>
PEAddingMethods<Registers_x64>::ErrorCode PEAddingMethods<Registers_x64>::injectEpCode(QList<uint64_t> &epMethods)
{
    typedef Registers_x64 Register;

    PEFile *pe = dynamic_cast<PEFile*>(file);
    if(!pe)
        return ErrorCode::BinaryFileNoPe;

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
        return ErrorCode::PeOperationFailed;

    new_ep -= pe->getImageBase();

    if(!pe->setNewEntryPoint(new_ep))
        return ErrorCode::PeOperationFailed;

    return ErrorCode::Success;
}

template <typename Register>
typename PEAddingMethods<Register>::ErrorCode PEAddingMethods<Register>::injectTlsCode(QList<uint64_t> &tlsMethods)
{
    PEFile *pe = dynamic_cast<PEFile*>(DAddingMethods<Register>::file);
    if(!pe)
        return ErrorCode::BinaryFileNoPe;

    // Tworzenie tablicy TLS jeśli nie istnieje
    if(!pe->hasTls())
    {
        QByteArray new_tls(pe->getImageTlsDirectorySize(), '\x00');
        uint64_t addr = pe->injectUniqueData(new_tls, codePointers);

        if(addr == 0)
            return ErrorCode::PeOperationFailed;

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
        return ErrorCode::PeOperationFailed;

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

    return ErrorCode::Success;
}
template PEAddingMethods<Registers_x86>::ErrorCode PEAddingMethods<Registers_x86>::injectTlsCode(QList<uint64_t> &tlsMethods);
template PEAddingMethods<Registers_x64>::ErrorCode PEAddingMethods<Registers_x64>::injectTlsCode(QList<uint64_t> &tlsMethods);


template <typename Register>
typename PEAddingMethods<Register>::ErrorCode PEAddingMethods<Register>::injectTrampolineCode(QList<uint64_t> &tramMethods)
{
    PEFile *pe = dynamic_cast<PEFile*>(DAddingMethods<Register>::file);
    if(!pe)
        return ErrorCode::BinaryFileNoPe;

    QTemporaryFile temp_file;
    if(!temp_file.open())
        return ErrorCode::CannotCreateTempFile;

    temp_file.write(text_section);
    temp_file.flush();

    QProcess ndisasm;

    ndisasm.setProcessChannelMode(QProcess::MergedChannels);
    QString ndisasm_path = DSettings::getSettings().getNdisasmPath();
    ndisasm.start(ndisasm_path, {"-a", "-b", pe->is_x64() ? "64" : "32", QFileInfo(temp_file).absoluteFilePath()});

    if(!ndisasm.waitForStarted())
        return ErrorCode::NdisasmFailed;

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

        BinaryCode<Register> code = generateTrampolineCode(pe->getAddressAtCallInstructionOffset(offset), tramMethods[method_idx]);

        uint64_t addr = pe->injectUniqueData(code, codePointers, relocations);
        if(addr == 0)
            return ErrorCode::PeOperationFailed;

        pe->setAddressAtCallInstructionOffset(offset, addr);

        method_idx = (method_idx + 1) % tramMethods.length();
    }

    return ErrorCode::Success;
}

template <>
PEAddingMethods<Registers_x86>::ErrorCode PEAddingMethods<Registers_x86>::generateThreadCode
(QList<DAddingMethods<Registers_x86>::Wrapper*> wrappers, uint64_t &codePtr, uint16_t sleepTime)
{
    typedef Registers_x86 Register;

    ErrorCode ec;

    PEFile *pe = dynamic_cast<PEFile*>(file);
    if(!pe)
        return ErrorCode::BinaryFileNoPe;

    BinaryCode<Register> code;

    code.append(CodeDefines<Register>::startFunc);
    int jmp_offset = 0;

    if(sleepTime)
    {
        DJsonParser parser(DSettings::getSettings().getDescriptionsPath<Register>());
        typename DAddingMethods<Register>::Wrapper *func_wrap =
                parser.loadInjectDescription<Register>(windowsApiLoadingFunction);
        if(!func_wrap)
            return ErrorCode::ErrorLoadingFunctions;

        uint64_t get_functions = 0;
        ec = generateCode(func_wrap, get_functions);
        delete func_wrap;
        if(ec != ErrorCode::Success)
            return ec;

        uint32_t lib_name_addr = pe->generateString("kernel32", codePointers);
        uint32_t func_name_addr = pe->generateString("Sleep", codePointers);
        if(!lib_name_addr || !func_name_addr)
            return ErrorCode::PeOperationFailed;

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

    foreach(DAddingMethods<Register>::Wrapper *w, wrappers)
    {
        if(!w)
            return ErrorCode::NullWrapper;

        uint64_t fnc = 0;
        ec = generateCode(w, fnc);
        if(ec != ErrorCode::Success)
            return ec;

        code.append(CodeDefines<Register>::movValueToReg(fnc, Register::EAX), true);
        code.append(CodeDefines<Register>::callReg(Register::EAX));
    }

    if(sleepTime)
    {
        jmp_offset = code.length() + 2 - jmp_offset;
        if(jmp_offset > 126)
            return ErrorCode::ToManyBytesForRelativeJump;

        code.append(CodeDefines<Register>::jmpRelative(-jmp_offset));
        code.append(CodeDefines<Register>::clearStackSpace(1));
    }

    code.append(CodeDefines<Register>::movValueToReg(0U, Register::EAX));
    code.append(CodeDefines<Register>::endFunc);
    code.append(CodeDefines<Register>::retN(4));

    codePtr = pe->injectUniqueData(code, codePointers, relocations);

    return codePtr == 0 ? ErrorCode::PeOperationFailed : ErrorCode::Success;
}

template <>
PEAddingMethods<Registers_x64>::ErrorCode PEAddingMethods<Registers_x64>::generateThreadCode
(QList<DAddingMethods<Registers_x64>::Wrapper*> wrappers, uint64_t &codePtr, uint16_t sleepTime)
{
    typedef Registers_x64 Register;

    ErrorCode ec;

    PEFile *pe = dynamic_cast<PEFile*>(file);
    if(!pe)
        return ErrorCode::BinaryFileNoPe;

    BinaryCode<Register> code;

    code.append(CodeDefines<Register>::startFunc);

    int jmp_offset = 0;

    if(sleepTime)
    {
        DJsonParser parser(DSettings::getSettings().getDescriptionsPath<Register>());
        typename DAddingMethods<Register>::Wrapper *func_wrap =
                parser.loadInjectDescription<Register>(windowsApiLoadingFunction);
        if(!func_wrap)
            return ErrorCode::ErrorLoadingFunctions;

        uint64_t get_functions = 0;
        ec = generateCode(func_wrap, get_functions);
        delete func_wrap;
        if(ec != ErrorCode::Success)
            return ec;

        uint64_t lib_name_addr = pe->generateString("kernel32", codePointers);
        uint64_t func_name_addr = pe->generateString("Sleep", codePointers);
        if(!lib_name_addr || !func_name_addr)
            return ErrorCode::PeOperationFailed;

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

    foreach(DAddingMethods<Register>::Wrapper *w, wrappers)
    {
        if(!w)
            return ErrorCode::NullWrapper;

        uint64_t fnc = 0;
        ec = generateCode(w, fnc);
        if(ec != ErrorCode::Success)
            return ec;
        code.append(CodeDefines<Register>::movValueToReg(fnc, Register::RAX), true);
        code.append(CodeDefines<Register>::callReg(Register::RAX));
    }

    code.append(CodeDefines<Register>::clearStackSpace(CodeDefines<Register>::shadowSize));

    if(sleepTime)
    {
        jmp_offset = code.length() + 2 - jmp_offset;
        if(jmp_offset > 126)
            return ErrorCode::ToManyBytesForRelativeJump;

        code.append(CodeDefines<Register>::jmpRelative(-jmp_offset));
        code.append(CodeDefines<Register>::clearStackSpace(CodeDefines<Register>::shadowSize));
    }

    code.append(CodeDefines<Register>::movValueToReg(static_cast<uint64_t>(0), Register::RAX));
    code.append(CodeDefines<Register>::endFunc);
    code.append(CodeDefines<Register>::ret);

    codePtr = pe->injectUniqueData(code, codePointers, relocations);

    return codePtr == 0 ? ErrorCode::PeOperationFailed : ErrorCode::Success;
}

template <>
template <>
PEAddingMethods<Registers_x86>::ErrorCode PEAddingMethods<Registers_x86>::generateParametersLoadingCode<uint32_t>
(BinaryCode<Registers_x86> &code, uint32_t getFunctionsCodeAddr, QMap<Registers_x86, QString> params, uint32_t threadCodePtr)
{
    typedef Registers_x86 Reg;

    PEFile *pe = dynamic_cast<PEFile*>(file);
    if(!pe)
        return ErrorCode::BinaryFileNoPe;

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
            return ErrorCode::InvalidParametersFormat;

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

    return ErrorCode::Success;
}

template <>
template <>
PEAddingMethods<Registers_x64>::ErrorCode PEAddingMethods<Registers_x64>::generateParametersLoadingCode<uint64_t>
(BinaryCode<Registers_x64> &code, uint64_t getFunctionsCodeAddr, QMap<Registers_x64, QString> params, uint64_t threadCodePtr)
{
    typedef Registers_x64 Reg;

    PEFile *pe = dynamic_cast<PEFile*>(file);
    if(!pe)
        return ErrorCode::BinaryFileNoPe;

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
            return ErrorCode::InvalidParametersFormat;

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

    return ErrorCode::Success;
}

template <>
template <>
PEAddingMethods<Registers_x86>::ErrorCode PEAddingMethods<Registers_x86>::generateParametersLoadingCode<uint64_t>
(BinaryCode<Registers_x86> &code, uint64_t getFunctionsCodeAddr, QMap<Registers_x86, QString> params, uint64_t threadCodePtr)
{
    return generateParametersLoadingCode<uint32_t>
            (code, static_cast<uint32_t>(getFunctionsCodeAddr), params, static_cast<uint32_t>(threadCodePtr));
}

template <typename Register>
typename PEAddingMethods<Register>::ErrorCode PEAddingMethods<Register>::generateActionConditionCode
(BinaryCode<Register> &code, uint64_t action, Register cond, Register act)
{
    PEFile *pe = dynamic_cast<PEFile*>(DAddingMethods<Register>::file);
    if(!pe)
        return ErrorCode::BinaryFileNoPe;

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

    return ErrorCode::Success;
}
template PEAddingMethods<Registers_x86>::ErrorCode PEAddingMethods<Registers_x86>::generateActionConditionCode(BinaryCode<Registers_x86> &code, uint64_t action, Registers_x86 cond, Registers_x86 act);
template PEAddingMethods<Registers_x64>::ErrorCode PEAddingMethods<Registers_x64>::generateActionConditionCode(BinaryCode<Registers_x64> &code, uint64_t action, Registers_x64 cond, Registers_x64 act);

template <typename Register>
void PEAddingMethods<Register>::getFileOffsetsFromOpcodes(QStringList &opcodes, QList<uint32_t> &fileOffsets, uint32_t baseOffset)
{
    foreach(QString op, opcodes)
        fileOffsets.append(op.mid(0, 8).toUInt(NULL, 16) + baseOffset + 1);
}

template <typename Register>
typename PEAddingMethods<Register>::ErrorCode PEAddingMethods<Register>::compileCode(QByteArray code, QByteArray &compiled)
{
    QByteArray bin;

    QTemporaryFile temp_file;
    if(!temp_file.open())
        return ErrorCode::CannotCreateTempFile;

    temp_file.write(code);
    temp_file.flush();

    QTemporaryDir compile_dir;
    if(!compile_dir.isValid())
        return ErrorCode::CannotCreateTempDir;

    QString data_file = QFileInfo(compile_dir.path(), "data.bin").absoluteFilePath();

    QProcess nasm;

    nasm.setProcessChannelMode(QProcess::MergedChannels);
    QString nasmPath = DSettings::getSettings().getNasmPath();
    nasm.start(nasmPath, {"-f", "bin", "-o", data_file, QFileInfo(temp_file).absoluteFilePath()});

    if(!nasm.waitForFinished())
        return ErrorCode::NasmFailed;

    temp_file.close();

    QFile f(data_file);

    if(!f.open(QFile::ReadOnly))
        return ErrorCode::CannotOpenCompiledFile;

    bin = f.readAll();

    f.close();

    compiled = bin;
    return ErrorCode::Success;
}

template <>
Registers_x86 PEAddingMethods<Registers_x86>::getRandomRegister()
{
    typedef Registers_x86 Reg;
    QList<Reg> r = {Reg::EAX, Reg::ECX, Reg::EDX, Reg::EBX, Reg::ESI, Reg::EDI};

    std::uniform_int_distribution<int> idx(0, r.length() - 1);

    return r[idx(gen)];
}

template <>
Registers_x64 PEAddingMethods<Registers_x64>::getRandomRegister()
{
    typedef Registers_x64 Reg;
    QList<Reg> r = {Reg::RAX, Reg::RDX, Reg::RCX, Reg::RBX, Reg::RSI, Reg::RDI,
                    Reg::R8, Reg::R9, Reg::R10, Reg::R11, Reg::R12, Reg::R13, Reg::R14, Reg::R15};

    std::uniform_int_distribution<int> idx(0, r.length() - 1);

    return r[idx(gen)];
}

template <>
BinaryCode<Registers_x86> PEAddingMethods<Registers_x86>::generateTrampolineCode(uint64_t realAddr, uint64_t wrapperAddr)
{
    typedef Registers_x86 Register;

    BinaryCode<Register> code;

    code.append(CodeDefines<Register>::storeValue(static_cast<uint32_t>(realAddr)), true);

    code.append(CodeDefines<Register>::saveAll());
    Register r = getRandomRegister();
    code.append(CodeDefines<Register>::movValueToReg(wrapperAddr, r), true);
    code.append(CodeDefines<Register>::callReg(r));
    code.append(CodeDefines<Register>::restoreAll());

    code.append(CodeDefines<Register>::ret);

    return code;
}

template <>
BinaryCode<Registers_x64> PEAddingMethods<Registers_x64>::generateTrampolineCode(uint64_t realAddr, uint64_t wrapperAddr)
{
    typedef Registers_x64 Register;

    BinaryCode<Register> code;

    Register r = getRandomRegister();
    code.append(CodeDefines<Register>::reserveStackSpace(CodeDefines<Register>::align16Size));
    code.append(CodeDefines<Register>::saveRegister(r));
    code.append(CodeDefines<Register>::movValueToReg(realAddr, r), true);
    code.append(CodeDefines<Register>::readFromRegToEspMem(r, CodeDefines<Register>::stackCellSize));
    code.append(CodeDefines<Register>::restoreRegister(r));

    code.append(CodeDefines<Register>::saveAll());
    code.append(CodeDefines<Register>::reserveStackSpace(CodeDefines<Register>::shadowSize));
    r = getRandomRegister();
    code.append(CodeDefines<Register>::movValueToReg(wrapperAddr, r), true);
    code.append(CodeDefines<Register>::callReg(r));
    code.append(CodeDefines<Register>::clearStackSpace(CodeDefines<Register>::shadowSize));
    code.append(CodeDefines<Register>::restoreAll());

    code.append(CodeDefines<Register>::ret);

    return code;
}
