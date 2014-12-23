#include <core/file_types/pefile.h>

#ifdef _WIN32

unsigned int PEFile::getOptHdrFileAlignment()
{
    return is_x64 ? getOptionalHeader64()->FileAlignment : getOptionalHeader32()->FileAlignment;
}

unsigned int PEFile::getOptHdrSectionAlignment()
{
    return is_x64 ? getOptionalHeader64()->SectionAlignment : getOptionalHeader32()->SectionAlignment;
}

size_t PEFile::getOptHdrSizeOfCode()
{
    return is_x64 ? getOptionalHeader64()->SizeOfCode : getOptionalHeader32()->SizeOfCode;
}

size_t PEFile::getOptHdrSizeOfInitializedData()
{
    return is_x64 ? getOptionalHeader64()->SizeOfInitializedData : getOptionalHeader32()->SizeOfInitializedData;
}

unsigned int PEFile::getOptHdrAddressOfEntryPoint()
{
    return is_x64 ? getOptionalHeader64()->AddressOfEntryPoint : getOptionalHeader32()->AddressOfEntryPoint;
}

unsigned int PEFile::getNtHdrSignature()
{
    return is_x64 ? getNtHeaders64()->Signature : getNtHeaders32()->Signature;
}

unsigned int PEFile::getOptHdrNumberOfRvaAndSizes()
{
    return is_x64 ? getOptionalHeader64()->NumberOfRvaAndSizes : getOptionalHeader32()->NumberOfRvaAndSizes;
}

uint64_t PEFile::getOptHdrImageBase()
{
    return is_x64 ? getOptionalHeader64()->ImageBase : getOptionalHeader32()->ImageBase;
}

void PEFile::setOptHdrSizeOfCode(size_t size)
{
    if(is_x64)
        getOptionalHeader64()->SizeOfCode = size;
    else
        getOptionalHeader32()->SizeOfCode = size;
}

void PEFile::setOptHdrSizeOfInitializedData(size_t size)
{
    if(is_x64)
        getOptionalHeader64()->SizeOfInitializedData = size;
    else
        getOptionalHeader32()->SizeOfInitializedData = size;
}

void PEFile::setOptHdrSizeOfImage(size_t size)
{
    if(is_x64)
        getOptionalHeader64()->SizeOfImage = size;
    else
        getOptionalHeader32()->SizeOfImage = size;
}

void PEFile::setOptHdrSizeOfHeaders(size_t size)
{
    if(is_x64)
        getOptionalHeader64()->SizeOfHeaders = size;
    else
        getOptionalHeader32()->SizeOfHeaders = size;
}

void PEFile::setOptHdrAddressOfEntryPoint(unsigned int ep)
{
    if(is_x64)
        getOptionalHeader64()->AddressOfEntryPoint = ep;
    else
        getOptionalHeader32()->AddressOfEntryPoint = ep;
}

PIMAGE_DOS_HEADER PEFile::getDosHeader()
{
    return reinterpret_cast<PIMAGE_DOS_HEADER>(&(b_data.data()[dosHeaderIdx]));
}

PIMAGE_NT_HEADERS32 PEFile::getNtHeaders32()
{
    return reinterpret_cast<PIMAGE_NT_HEADERS32>(&(b_data.data()[ntHeadersIdx]));
}

PIMAGE_NT_HEADERS64 PEFile::getNtHeaders64()
{
    return reinterpret_cast<PIMAGE_NT_HEADERS64>(&(b_data.data()[ntHeadersIdx]));
}

PIMAGE_FILE_HEADER PEFile::getFileHeader()
{
    return reinterpret_cast<PIMAGE_FILE_HEADER>(&(b_data.data()[fileHeaderIdx]));
}

PIMAGE_OPTIONAL_HEADER32 PEFile::getOptionalHeader32()
{
    return reinterpret_cast<PIMAGE_OPTIONAL_HEADER32>(&(b_data.data()[optionalHeaderIdx]));
}

PIMAGE_OPTIONAL_HEADER64 PEFile::getOptionalHeader64()
{
    return reinterpret_cast<PIMAGE_OPTIONAL_HEADER64>(&(b_data.data()[optionalHeaderIdx]));
}

PIMAGE_SECTION_HEADER PEFile::getSectionHeader(unsigned int n)
{
    return n >= numberOfSections ?
                NULL : reinterpret_cast<PIMAGE_SECTION_HEADER>(&(b_data.data()[sectionHeadersIdx[n]]));
}

PIMAGE_DATA_DIRECTORY PEFile::getDataDirectory(unsigned int n)
{
    return n >= numberOfDataDirectories ?
                NULL : reinterpret_cast<PIMAGE_DATA_DIRECTORY>(&(b_data.data()[dataDirectoriesIdx[n]]));
}

PEFile::PEFile(QByteArray d) :
    parsed(false),
    is_x64(false),
    sectionHeadersIdx(NULL),
    dataDirectoriesIdx(NULL)
{
    b_data = d;
    parsed = parse();
}

PEFile::~PEFile()
{
    if(sectionHeadersIdx)
        delete [] sectionHeadersIdx;

    if(dataDirectoriesIdx)
        delete [] dataDirectoriesIdx;
}

bool PEFile::injectCode(QList<InjectDescription> descs)
{
    QMap<QByteArray, uint64_t> codePointers;
    QList<uint64_t> epMethods, tlsMethods, tramMethods;

    if(!parsed)
        return false;

    foreach(InjectDescription desc, descs)
    {
        switch(desc.getCallingMethod())
        {
        case CallingMethod::EntryPoint:
            epMethods.append(generateCode(desc.getWrapper(), codePointers));
            if(epMethods.last() == 0)
                return false;
            break;

        case CallingMethod::TLS:
            // TODO: TLS
            break;

        case CallingMethod::Trampoline:
            // TODO: trampolina
            break;
        }
    }

    if(!epMethods.empty())
    {
        QByteArray code;

        foreach(uint64_t offset, epMethods)
        {
            code.append(PECodeDefines::movDWordToReg(offset, Register::EAX));
            code.append(PECodeDefines::callReg(Register::EAX));
        }

        code.append(PECodeDefines::movDWordToReg(getEntryPoint(), Register::EAX));
        code.append(PECodeDefines::jmpReg(Register::EAX));

        uint64_t new_ep = injectUniqueData(code, codePointers);

        if(new_ep == 0)
            return false;

        new_ep -= getImageBase();

        if(!setNewEntryPoint(new_ep))
            return false;
    }

    return true;
}

uint64_t PEFile::generateCode(Wrapper *w, QMap<QByteArray, uint64_t> &ptrs)
{
    if(!parsed)
        return 0;

    if(!w)
        return 0;

    uint64_t action = 0;
    uint64_t thread = 0;

    // Generowanie kodu dla akcji.
    if(w->getAction())
    {
        action = generateCode(w->getAction(), ptrs);
        if(!action) return 0;
    }

    // Generowanie kodu dla funkcji wątku.
    ThreadWrapper *tw = dynamic_cast<ThreadWrapper*>(w);

    if(tw && tw->getThreadWrappers().empty())
        return 0;

    if(tw && !tw->getThreadWrappers().empty())
    {
        thread = generateThreadCode(tw->getThreadWrappers(), ptrs, tw->getSleepTime());
        if(!thread) return 0;
    }

    // Generowanie kodu
    QByteArray code;

    // Tworzenie ramki stosu
    code.append(PECodeDefines::startFunc);

    std::list<Register> rts = w->getRegistersToSave().toStdList();

    // Zachowywanie rejestrów
    for(auto it = rts.begin(); it != rts.end(); ++it)
    {
        if(PECodeDefines::externalRegs.contains(*it))
            code.append(PECodeDefines::saveRegister(*it));
    }

    // Ładowanie parametrów
    if(!w->getParameters().empty())
    {
        Wrapper *func_wrap = Wrapper::fromFile(Wrapper::methodsPath + "load_functions.asm");
        if(!func_wrap)
            return 0;

        uint64_t get_functions = generateCode(func_wrap, ptrs);
        delete func_wrap;

        code.append(PECodeDefines::reserveStackSpace(3));
        code.append(PECodeDefines::movDWordToReg(get_functions, Register::EAX));
        code.append(PECodeDefines::callReg(Register::EAX));

        QList<Register> keys = w->getParameters().keys();
        foreach (Register r, keys)
        {
            QList<QString> func_name = w->getParameters()[r].split('!');
            if(func_name.length() != 2)
                return 0;

            if(thread && func_name[0] == "THREAD" && func_name[1] == "THREAD")
            {
                code.append(PECodeDefines::movDWordToReg(thread, r));
                continue;
            }

            uint64_t lib_name_addr = generateString(func_name[0], ptrs);
            uint64_t func_name_addr = generateString(func_name[1], ptrs);

            code.append(PECodeDefines::saveAllInternal());

            code.append(PECodeDefines::storeDWord(lib_name_addr));
            code.append(PECodeDefines::readFromEspMemToReg(Register::EAX, 20));
            code.append(PECodeDefines::callReg(Register::EAX));

            code.append(PECodeDefines::storeDWord(func_name_addr));
            code.append(PECodeDefines::saveRegister(Register::EAX));
            code.append(PECodeDefines::readFromEspMemToReg(Register::EAX, 20));
            code.append(PECodeDefines::callReg(Register::EAX));

            code.append(PECodeDefines::readFromRegToEspMem(Register::EAX, 20));

            code.append(PECodeDefines::restoreAllInternal());

            code.append(PECodeDefines::readFromEspMemToReg(r, 8));
        }

        code.append(PECodeDefines::clearStackSpace(3));
    }

    // Doklejanie właściwego kodu
    code.append(w->getCode());

    // Handler
    if(action)
    {
        Register cond = w->getReturns();
        int act_idx = PECodeDefines::internalRegs.indexOf(cond);
        Register act = act_idx == -1 ? PECodeDefines::internalRegs[0] :
                PECodeDefines::internalRegs[(act_idx + 1) % PECodeDefines::internalRegs.length()];

        // mov act, &action()
        // test cond, cond
        // jz xxx
        // push internal
        // call act
        // pop internal
        // xxx:
        code.append(PECodeDefines::movDWordToReg(action, act));
        code.append(PECodeDefines::testReg(cond));

        QByteArray call_code;
        call_code.append(PECodeDefines::saveAllInternal());
        call_code.append(PECodeDefines::callReg(act));
        call_code.append(PECodeDefines::restoreAllInternal());

        code.append(PECodeDefines::jzRelative(call_code.length()));
        code.append(call_code);
    }

    // Przywracanie rejestrów
    for(auto it = rts.rbegin(); it != rts.rend(); ++it)
    {
        if(PECodeDefines::externalRegs.contains(*it))
            code.append(PECodeDefines::saveRegister(*it));
    }

    // Niszczenie ramki stosu
    code.append(PECodeDefines::endFunc);

    return injectUniqueData(code, ptrs);
}

uint64_t PEFile::generateThreadCode(QList<Wrapper *> wrappers, QMap<QByteArray, uint64_t> &ptrs, uint16_t sleepTime)
{
    QByteArray code;

    code.append(PECodeDefines::startFunc);
    int jmp_offset = 0;

    if(sleepTime)
    {
        Wrapper *func_wrap = Wrapper::fromFile(Wrapper::methodsPath + "load_functions.asm");
        if(!func_wrap)
            return 0;

        uint64_t get_functions = generateCode(func_wrap, ptrs);
        delete func_wrap;
        if(get_functions == 0)
            return 0;

        uint64_t lib_name_addr = generateString("kernel32", ptrs);
        uint64_t func_name_addr = generateString("Sleep", ptrs);
        if(!lib_name_addr || !func_name_addr)
            return 0;

        code.append(PECodeDefines::reserveStackSpace(2));
        code.append(PECodeDefines::movDWordToReg(get_functions, Register::EAX));
        code.append(PECodeDefines::callReg(Register::EAX));

        code.append(PECodeDefines::restoreRegister(Register::EAX));
        code.append(PECodeDefines::restoreRegister(Register::EDX));
        code.append(PECodeDefines::saveRegister(Register::EAX));

        code.append(PECodeDefines::storeDWord(lib_name_addr));
        code.append(PECodeDefines::callReg(Register::EDX));

        code.append(PECodeDefines::restoreRegister(Register::EDX));

        code.append(PECodeDefines::storeDWord(func_name_addr));
        code.append(PECodeDefines::saveRegister(Register::EAX));
        code.append(PECodeDefines::callReg(Register::EDX));
        code.append(PECodeDefines::saveRegister(Register::EAX));

        jmp_offset = code.length();

        // Pętla
        code.append(PECodeDefines::readFromEspMemToReg(Register::EAX, 0));
        code.append(PECodeDefines::storeDWord(sleepTime * 1000));
        code.append(PECodeDefines::callReg(Register::EAX));
    }

    foreach(Wrapper *w, wrappers)
    {
        if(!w)
            return 0;

        uint32_t fnc = generateCode(w, ptrs);
        code.append(PECodeDefines::movDWordToReg(fnc, Register::EAX));
        code.append(PECodeDefines::callReg(Register::EAX));
    }

    if(sleepTime)
    {
        jmp_offset = code.length() + 2 - jmp_offset;
        if(jmp_offset > 126)
            return 0;

        code.append(PECodeDefines::jmpRelative(-jmp_offset));
        code.append(PECodeDefines::clearStackSpace(1));
    }

    code.append(PECodeDefines::movDWordToReg(0, Register::EAX));
    code.append(PECodeDefines::endFunc);
    code.append(PECodeDefines::retN(4));

    return injectUniqueData(code, ptrs);
}

uint64_t PEFile::generateString(QString str, QMap<QByteArray, uint64_t> &ptrs)
{
    return injectUniqueData(QByteArray(str.toStdString().c_str(), str.length() + 1), ptrs);
}

uint64_t PEFile::injectUniqueData(QByteArray data, QMap<QByteArray, uint64_t> &ptrs)
{
    QByteArray hash = QCryptographicHash::hash(data, QCryptographicHash::Sha3_512);

    if(ptrs.contains(hash))
        return ptrs[hash];

    unsigned int fileOffset = 0;
    unsigned int memOffset = 0;
    bool is_added = false;

    // Próba dodania kodu do każdej z sekcji
    for(unsigned int i = 0; !is_added && i < numberOfSections; ++i)
        is_added = addDataToSection(i, data, fileOffset, memOffset);

    // Próba dodania kodu do każdej z sekcji z rozszerzeniem
    for(unsigned int i = 0; !is_added && i < numberOfSections - 1; ++i)
        is_added = addDataToSection(i, data, fileOffset, memOffset);

    // Tworzenie nowej sekcji, lub rozszerzanie ostatniej
    std::default_random_engine gen;
    std::uniform_int_distribution<int> prob(0, 9);
    if(prob(gen) == 0)
    {
        if(!is_added)
            is_added = addNewSection(getRandomSectionName(), data, fileOffset, memOffset);

        if(!is_added)
            is_added = resizeLastSection(data, fileOffset, memOffset);
    }
    else
    {
        if(!is_added)
            is_added = resizeLastSection(data, fileOffset, memOffset);

        if(!is_added)
            is_added = addNewSection(getRandomSectionName(), data, fileOffset, memOffset);
    }

    if(!is_added)
        return 0;

    uint64_t offset = memOffset + getImageBase();
    ptrs.insert(hash, offset);

    return offset;
}

QString PEFile::getRandomSectionName()
{
    QString name;
    std::default_random_engine generator;
    std::uniform_int_distribution<int> no_letters(1, 7);
    std::uniform_int_distribution<int> char_gen('a', 'z');

    name += '.';
    int n = no_letters(generator);
    for(int i = 0; i < n; ++i)
        name += static_cast<char>(char_gen(generator));

    return name;
}

bool PEFile::isValid()
{
    return parsed;
}

bool PEFile::parse()
{
    char *data = b_data.data();
    size_t length = b_data.length();

    PIMAGE_DOS_HEADER dosHeader = reinterpret_cast<PIMAGE_DOS_HEADER>(data);
    dosHeaderIdx = 0;

    if(length < sizeof(IMAGE_DOS_HEADER) || dosHeader->e_magic != IMAGE_DOS_SIGNATURE)
        return false;

    is_x64 = isPE_64(dosHeader->e_lfanew);

    PIMAGE_NT_HEADERS32 ntHeaders32 = NULL;
    PIMAGE_NT_HEADERS64 ntHeaders64 = NULL;

    if(is_x64)
    {
        ntHeaders64 = reinterpret_cast<PIMAGE_NT_HEADERS64>(reinterpret_cast<char*>(dosHeader) +
                                                            dosHeader->e_lfanew);
        ntHeadersIdx = reinterpret_cast<char*>(ntHeaders64) - reinterpret_cast<char*>(dosHeader);

        if(length < dosHeader->e_lfanew + sizeof(IMAGE_NT_HEADERS64::Signature)
                || getNtHdrSignature() != IMAGE_NT_SIGNATURE)
            return false;
    }
    else
    {
        ntHeaders32 = reinterpret_cast<PIMAGE_NT_HEADERS32>(reinterpret_cast<char*>(dosHeader) +
                                                            dosHeader->e_lfanew);
        ntHeadersIdx = reinterpret_cast<char*>(ntHeaders32) - reinterpret_cast<char*>(dosHeader);

        if(length < dosHeader->e_lfanew + sizeof(IMAGE_NT_HEADERS32::Signature)
                || getNtHdrSignature() != IMAGE_NT_SIGNATURE)
            return false;
    }

    PIMAGE_FILE_HEADER fileHeader = is_x64 ?
                reinterpret_cast<PIMAGE_FILE_HEADER>(&ntHeaders64->FileHeader) :
                reinterpret_cast<PIMAGE_FILE_HEADER>(&ntHeaders32->FileHeader);

    fileHeaderIdx = reinterpret_cast<char*>(fileHeader) - reinterpret_cast<char*>(dosHeader);

    if(length < (fileHeaderIdx + sizeof(IMAGE_FILE_HEADER)))
        return false;

    PIMAGE_OPTIONAL_HEADER32 optionalHeader32 = NULL;
    PIMAGE_OPTIONAL_HEADER64 optionalHeader64 = NULL;

    if(is_x64)
        optionalHeader64 = reinterpret_cast<PIMAGE_OPTIONAL_HEADER64>(&ntHeaders64->OptionalHeader);
    else
        optionalHeader32 = reinterpret_cast<PIMAGE_OPTIONAL_HEADER32>(&ntHeaders32->OptionalHeader);

    optionalHeaderSize = fileHeader->SizeOfOptionalHeader;
    optionalHeaderIdx = is_x64 ?
                (reinterpret_cast<char*>(optionalHeader64) - reinterpret_cast<char*>(dosHeader)) :
                (reinterpret_cast<char*>(optionalHeader32) - reinterpret_cast<char*>(dosHeader));

    if(length < optionalHeaderIdx + optionalHeaderSize)
        return false;

    if(is_x64 && optionalHeader64->Magic != IMAGE_NT_OPTIONAL_HDR64_MAGIC)
        return false;

    if(!is_x64 && optionalHeader32->Magic != IMAGE_NT_OPTIONAL_HDR32_MAGIC)
        return false;

    // Data Directories
    numberOfDataDirectories = getOptHdrNumberOfRvaAndSizes();

    if(dataDirectoriesIdx)
        delete [] dataDirectoriesIdx;

    try
    {
        dataDirectoriesIdx = new unsigned int[numberOfDataDirectories];
    }
    catch(std::bad_alloc &)
    {
        dataDirectoriesIdx = NULL;
        return false;
    }

    unsigned int firstDataDirIdx = is_x64 ?
                reinterpret_cast<char*>(&(optionalHeader64->DataDirectory[0])) - reinterpret_cast<char*>(dosHeader) :
        reinterpret_cast<char*>(&(optionalHeader32->DataDirectory[0])) - reinterpret_cast<char*>(dosHeader);

    for(unsigned int i = 0; i < numberOfDataDirectories; ++i)
        dataDirectoriesIdx[i] = firstDataDirIdx + i * sizeof(IMAGE_DATA_DIRECTORY);

    // Section Headers
    numberOfSections = fileHeader->NumberOfSections;

    if(sectionHeadersIdx)
        delete [] sectionHeadersIdx;

    try
    {
        sectionHeadersIdx = new unsigned int[numberOfSections];
    }
    catch(std::bad_alloc &)
    {
        sectionHeadersIdx = NULL;
        return false;
    }

    unsigned int firstSectionHeaderIdx = optionalHeaderIdx + optionalHeaderSize;

    for(unsigned int i = 0; i < numberOfSections; ++i)
        sectionHeadersIdx[i] = firstSectionHeaderIdx + i * sizeof(IMAGE_SECTION_HEADER);

    return true;
}

bool PEFile::isPE_64(unsigned int pe_offset)
{
    pe_offset += sizeof(DWORD);

    return *reinterpret_cast<WORD*>(&b_data.data()[pe_offset]) != IMAGE_FILE_MACHINE_I386;
}

size_t PEFile::getFreeSpaceBeforeNextSectionMem(unsigned int section)
{
    unsigned int secEnd = getSectionHeader(section)->VirtualAddress + getSectionHeader(section)->SizeOfRawData;

    int next = -1;
    unsigned int nextRVA = 0;

    for(unsigned int i = 0; i < numberOfSections; ++i)
    {
        unsigned int va = getSectionHeader(i)->VirtualAddress;
        if(va >= secEnd && va > nextRVA)
        {
            next = i;
            nextRVA = va;
        }
    }

    if(next == -1)
        return 0;

    return nextRVA - secEnd;
}

size_t PEFile::getFreeSpaceBeforeFirstSectionFile()
{
    int first = -1;
    unsigned int offset = ~0;

    for(unsigned int i = 0; i < numberOfSections; ++i)
    {
        if(getSectionHeader(i)->SizeOfRawData > 0 && getSectionHeader(i)->PointerToRawData < offset)
        {
            first = i;
            offset = getSectionHeader(i)->PointerToRawData;
        }
    }

    if(first == -1)
        return 0;

    unsigned int nextHeader = sectionHeadersIdx[getNumberOfSections() - 1] + sizeof(IMAGE_SECTION_HEADER);

    if(offset <= nextHeader)
        return 0;

    return offset - nextHeader;
}

unsigned int PEFile::alignNumber(unsigned int number, unsigned int alignment)
{
    if(!alignment)
        return number;

    if(number % alignment == 0)
        return number;

    return ((number / alignment) + 1) * alignment;
}

bool PEFile::addDataToSectionExVirtual(unsigned int section, QByteArray data, unsigned int &fileOffset, unsigned int &memOffset)
{
    // TODO: BUG: kompilator zakłada że w takiej sekcji będą zera, jeżeli wypełniamy danymi możemy spowodować crash
    PIMAGE_SECTION_HEADER header = getSectionHeader(section);

    size_t newSizeOfRawData = alignNumber(qMax<unsigned int>(data.length(), header->Misc.VirtualSize), getOptHdrFileAlignment());

    if(getLastSectionNumberMem() != section && newSizeOfRawData > getFreeSpaceBeforeNextSectionMem(section))
        return false;

    header->Characteristics |= IMAGE_SCN_CNT_INITIALIZED_DATA;
    header->Characteristics &= ~IMAGE_SCN_CNT_UNINITIALIZED_DATA;

    if(!isSectionExecutable(section))
        makeSectionExecutable(section);

    header->SizeOfRawData = newSizeOfRawData;
    header->Misc.VirtualSize = qMax<unsigned int>(newSizeOfRawData, header->Misc.VirtualSize);
    header->PointerToRawData =
            getSectionHeader(getLastSectionNumberRaw())->PointerToRawData +
            getSectionHeader(getLastSectionNumberRaw())->SizeOfRawData;

    setOptHdrSizeOfCode(getOptHdrSizeOfCode() + newSizeOfRawData);
    setOptHdrSizeOfInitializedData(getOptHdrSizeOfInitializedData() + newSizeOfRawData);
    setOptHdrSizeOfImage(alignNumber(getSectionHeader(getLastSectionNumberMem())->VirtualAddress +
                                     getSectionHeader(getLastSectionNumberMem())->Misc.VirtualSize,
                                     getOptHdrSectionAlignment()));

    if(newSizeOfRawData > static_cast<size_t>(data.length()))
        data.append(QByteArray(newSizeOfRawData - data.length(), 0x00));

    fileOffset = header->PointerToRawData;
    memOffset = header->VirtualAddress;

    if(static_cast<size_t>(b_data.length()) < header->PointerToRawData + newSizeOfRawData)
        b_data.resize(header->PointerToRawData + newSizeOfRawData);
    b_data.replace(fileOffset, newSizeOfRawData, data);

    return parse();
}

bool PEFile::makeSectionExecutable(unsigned int section)
{
    if(!parsed)
        return false;

    if(section >= numberOfSections)
        return false;

    getSectionHeader(section)->Characteristics |=
            (IMAGE_SCN_CNT_CODE | IMAGE_SCN_MEM_EXECUTE | IMAGE_SCN_MEM_WRITE);
    getSectionHeader(section)->Characteristics &= (~IMAGE_SCN_MEM_DISCARDABLE);

    return true;
}

QByteArray PEFile::getData()
{
    return b_data;
}

unsigned int PEFile::getLastSectionNumberMem()
{
    if(!parsed)
        return 0;

    unsigned int last = 0;
    unsigned int lastRVA = 0;

    for(unsigned int i = 0; i < numberOfSections; ++i)
    {
        unsigned int rva = getSectionHeader(i)->VirtualAddress;
        if(rva > lastRVA)
        {
            lastRVA = rva;
            last = i;
        }
    }

    return last;
}

unsigned int PEFile::getLastSectionNumberRaw()
{
    if(!parsed)
        return 0;

    unsigned int last = 0;
    unsigned int lastRD = 0;

    for(unsigned int i = 0; i < numberOfSections; ++i)
    {
        unsigned int rd = getSectionHeader(i)->PointerToRawData;
        if(rd > lastRD && getSectionHeader(i)->SizeOfRawData != 0)
        {
            lastRD = rd;
            last = i;
        }
    }

    return last;
}

unsigned int PEFile::getNumberOfSections() const
{
    return parsed ? numberOfSections : 0;
}

size_t PEFile::getSectionFreeSpace(unsigned int section)
{
    if(!parsed || section >= numberOfSections)
        return 0;

    size_t rd = getSectionHeader(section)->SizeOfRawData;
    size_t vs = getSectionHeader(section)->Misc.VirtualSize;

    return rd > vs ? rd - vs : 0;
}

bool PEFile::isSectionRawDataEmpty(unsigned int section)
{
    if(!parsed || section >= numberOfSections)
        return false;

    return getSectionHeader(section)->SizeOfRawData == 0;
}

bool PEFile::resizeLastSection(QByteArray data, unsigned int &fileOffset, unsigned int &memOffset)
{
    if(!parsed)
        return false;

    unsigned int last = 0;
    bool isVirtual = false;

    // Ostatnia sekcja w pamięci i pliku.
    unsigned int lastRaw = getLastSectionNumberRaw();
    unsigned int lastMem = getLastSectionNumberMem();

    last = lastMem;

    // Jeżeli sekcje są różne.
    if(lastRaw != lastMem)
    {
        // Jeżeli ostatnia sekcja w pamięci jest sekcją wirtualną można zmienić jej położenie w pliku.
        // TODO: BUG: kompilator zakłada że w takiej sekcji będą zera, jeżeli wypełniamy danymi możemy spowodować crash
        if(isSectionRawDataEmpty(lastMem))
            isVirtual = true;
        else
        {
            // Jeżeli ostatnia sekcja w pliku ma za sobą wystarczające wolne miejsce dla danych.
            // Może to uszkodzić dane poza sekcjami!
            if(getFreeSpaceBeforeNextSectionMem(lastRaw) >= static_cast<size_t>(data.length()))
                last = lastRaw;
            else
                return false;
        }
    }

    PIMAGE_SECTION_HEADER header = getSectionHeader(last);

    // Liczba bajtów do dodania do PE.
    size_t numBytesToAdd = alignNumber(qMax<unsigned int>(data.length(), isVirtual ? header->Misc.VirtualSize : 0),
                                       getOptHdrFileAlignment());

    // Dopełnienie zerami nowych danych.
    size_t numOfZeros = numBytesToAdd - data.length();

    // Wielkość nowych danych bez zer.
    size_t actualDataLen = data.length();

    // Offset dla nowych danych.
    unsigned int newDataOffset = isVirtual ?
                alignNumber(b_data.length(), getOptHdrFileAlignment()) :
                header->PointerToRawData + header->SizeOfRawData;

    // Dodanie zer do nowych danych.
    if(numOfZeros)
        data.append(QByteArray(numOfZeros, 0x00));

    //    getOptionalHeader()->SizeOfImage +=
    //            alignNumber(qMax<int>(header->SizeOfRawData - header->Misc.VirtualSize, 0) + actualDataLen, getOptionalHeader()->SectionAlignment);

    header->Misc.VirtualSize = header->SizeOfRawData + actualDataLen;
    header->SizeOfRawData += numBytesToAdd;

    setOptHdrSizeOfImage(alignNumber(header->VirtualAddress + header->Misc.VirtualSize,
                                     getOptHdrSectionAlignment()));

    if(!isSectionExecutable(last))
    {
        // Wyrównany wcześniej
        setOptHdrSizeOfCode(getOptHdrSizeOfCode() + header->SizeOfRawData);
        makeSectionExecutable(last);
    }
    else
        setOptHdrSizeOfCode(getOptHdrSizeOfCode() + numBytesToAdd);

    if(isVirtual)
    {
        header->PointerToRawData = newDataOffset;
        header->Characteristics |= IMAGE_SCN_CNT_INITIALIZED_DATA;
        header->Characteristics &= ~IMAGE_SCN_CNT_UNINITIALIZED_DATA;
    }
    setOptHdrSizeOfInitializedData(getOptHdrSizeOfInitializedData() + numBytesToAdd);

    // TODO: size of uninitialized data?
    // TODO: virtual alignment?

    memOffset = header->VirtualAddress + header->SizeOfRawData - numBytesToAdd;

    if(static_cast<size_t>(b_data.length()) < newDataOffset + numBytesToAdd)
        b_data.resize(newDataOffset + numBytesToAdd);
    b_data.replace(newDataOffset, numBytesToAdd, data);

    fileOffset = newDataOffset;

    return parse();
}

bool PEFile::addDataToSection(unsigned int section, QByteArray data,
                              unsigned int &fileOffset, unsigned int &memOffset)
{
    if(!parsed || (section >= numberOfSections))
        return false;

    if(getSectionFreeSpace(section) < static_cast<size_t>(data.length()))
        return false;

    if(isSectionRawDataEmpty(section))
        return false;

    PIMAGE_SECTION_HEADER header = getSectionHeader(section);

    unsigned int newDataOffset = header->PointerToRawData + header->Misc.VirtualSize;

    memOffset = header->VirtualAddress + header->Misc.VirtualSize;
    header->Misc.VirtualSize += data.length();

    if(!isSectionExecutable(section))
        makeSectionExecutable(section);

    b_data.replace(newDataOffset, data.length(), data);

    fileOffset = newDataOffset;

    return parse();
}

bool PEFile::addDataToSectionEx(unsigned int section, QByteArray data, unsigned int &fileOffset, unsigned int &memOffset, bool changeVirtual)
{
    if(!parsed || (section >= numberOfSections))
        return false;

    if(getSectionFreeSpace(section) >= static_cast<size_t>(data.length()))
        return addDataToSection(section, data, fileOffset, memOffset);

    if(changeVirtual && isSectionRawDataEmpty(section))
        return addDataToSectionExVirtual(section, data, fileOffset, memOffset);

    PIMAGE_SECTION_HEADER header = getSectionHeader(section);

    // Za sekcją w pliku znajduje się inna sekcja i nie można jej rozszerzyć.
    if(section != getLastSectionNumberRaw())
        return false;

    size_t newSectionRawSize =
            alignNumber(header->Misc.VirtualSize + data.length(), getOptHdrFileAlignment());

    size_t newSectionVirtualSize = header->Misc.VirtualSize + data.length();

    // Liczba bajtów do fizycznego dodania do pliku.
    size_t numBytesToAdd = newSectionRawSize - header->SizeOfRawData;

    // Liczba bajtów którą należy wpisać do pliku.
    size_t numBytesToPaste = newSectionRawSize - header->Misc.VirtualSize;
    size_t numZeros = numBytesToPaste - data.length();

    unsigned int newDataOffset = header->PointerToRawData + header->Misc.VirtualSize;

    // Za sekcją w pamięci znajduje się inna sekcja, czy dane się mieszczą?
    if(section != getLastSectionNumberMem() && getFreeSpaceBeforeNextSectionMem(section) < numBytesToAdd)
        return false;

    memOffset = header->VirtualAddress + header->Misc.VirtualSize;

    if(numZeros)
        data.append(QByteArray(numZeros, 0x00));

    header->Misc.VirtualSize = newSectionVirtualSize;
    header->SizeOfRawData = newSectionRawSize;

    setOptHdrSizeOfImage(alignNumber(getSectionHeader(getLastSectionNumberMem())->VirtualAddress +
                                     getSectionHeader(getLastSectionNumberMem())->Misc.VirtualSize,
                                     getOptHdrSectionAlignment()));

    setOptHdrSizeOfInitializedData(getOptHdrSizeOfInitializedData() + numBytesToAdd);

    if(!isSectionExecutable(section))
    {
        setOptHdrSizeOfCode(getOptHdrSizeOfCode() + header->SizeOfRawData);
        makeSectionExecutable(section);
    }
    else
        setOptHdrSizeOfCode(getOptHdrSizeOfCode() + numBytesToAdd);

    if(static_cast<size_t>(b_data.length()) < newDataOffset + numBytesToPaste)
        b_data.resize(newDataOffset + numBytesToPaste);
    b_data.replace(newDataOffset, numBytesToPaste, data);

    fileOffset = newDataOffset;

    return parse();
}

bool PEFile::isSectionExecutable(unsigned int section)
{
    if(!parsed || section >= numberOfSections)
        return false;

    return ((getSectionHeader(section)->Characteristics & IMAGE_SCN_MEM_EXECUTE) |
            (getSectionHeader(section)->Characteristics & IMAGE_SCN_CNT_CODE)) != 0;
}

bool PEFile::setNewEntryPoint(unsigned int newEP)
{
    if(!parsed)
        return false;

    if(newEP > getSectionHeader(getLastSectionNumberMem())->VirtualAddress +
            getSectionHeader(getLastSectionNumberMem())->Misc.VirtualSize)
        return false;

    setOptHdrAddressOfEntryPoint(newEP);

    return true;
}

unsigned int PEFile::getEntryPoint()
{
    if(!parsed)
        return 0;

    return getOptHdrAddressOfEntryPoint();
}

uint64_t PEFile::getImageBase()
{
    if(!parsed)
        return 0;

    return getOptHdrImageBase();
}

bool PEFile::addNewSection(QString name, QByteArray data, unsigned int &fileOffset, unsigned int &memOffset)
{
    if(!parsed)
        return false;

    unsigned int newHeaderOffset = sectionHeadersIdx[getNumberOfSections() - 1] + sizeof(IMAGE_SECTION_HEADER);
    unsigned int newFileOffset = alignNumber(b_data.length(), getOptHdrFileAlignment());

    // Header się nie zmieści.
    if(getFreeSpaceBeforeFirstSectionFile() < sizeof(IMAGE_SECTION_HEADER))
        return false;

    QByteArray d_header(sizeof(IMAGE_SECTION_HEADER), 0x00);
    PIMAGE_SECTION_HEADER header = reinterpret_cast<PIMAGE_SECTION_HEADER>(d_header.data());

    strncpy(reinterpret_cast<char*>(header->Name), name.toStdString().c_str(), IMAGE_SIZEOF_SHORT_NAME);
    header->Misc.VirtualSize = data.length();
    header->VirtualAddress =
            alignNumber(getSectionHeader(getLastSectionNumberMem())->VirtualAddress +
                        getSectionHeader(getLastSectionNumberMem())->Misc.VirtualSize,
                        getOptHdrSectionAlignment());
    header->SizeOfRawData = alignNumber(data.length(), getOptHdrFileAlignment());
    header->PointerToRawData = newFileOffset;
    header->Characteristics = IMAGE_SCN_CNT_CODE | IMAGE_SCN_MEM_EXECUTE |
            IMAGE_SCN_MEM_READ | IMAGE_SCN_CNT_INITIALIZED_DATA;

    setOptHdrSizeOfCode(getOptHdrSizeOfCode() + header->SizeOfRawData);
    setOptHdrSizeOfInitializedData(getOptHdrSizeOfInitializedData() + header->SizeOfRawData);
    setOptHdrSizeOfImage(alignNumber(header->VirtualAddress + header->Misc.VirtualSize,
                                     getOptHdrSectionAlignment()));
    if(is_x64)
        setOptHdrSizeOfHeaders(alignNumber(sizeof(IMAGE_DOS_HEADER::e_lfanew) + sizeof(IMAGE_NT_HEADERS64::Signature) +
                                           sizeof(IMAGE_FILE_HEADER) + sizeof(IMAGE_OPTIONAL_HEADER64) +
                                           sizeof(IMAGE_SECTION_HEADER) * (getNumberOfSections() + 1),
                                           getOptHdrFileAlignment()));
    else
        setOptHdrSizeOfHeaders(alignNumber(sizeof(IMAGE_DOS_HEADER::e_lfanew) + sizeof(IMAGE_NT_HEADERS32::Signature) +
                                           sizeof(IMAGE_FILE_HEADER) + sizeof(IMAGE_OPTIONAL_HEADER32) +
                                           sizeof(IMAGE_SECTION_HEADER) * (getNumberOfSections() + 1),
                                           getOptHdrFileAlignment()));

    getFileHeader()->NumberOfSections += 1;

    fileOffset = header->PointerToRawData;
    memOffset = header->VirtualAddress;

    unsigned int sizeOfNewData = header->SizeOfRawData;

    if(static_cast<size_t>(data.length()) < sizeOfNewData)
        data.append(QByteArray(sizeOfNewData - data.length(), 0x00));

    if(static_cast<size_t>(b_data.length()) < newFileOffset + sizeOfNewData)
        b_data.resize(newFileOffset + sizeOfNewData);

    b_data.replace(newHeaderOffset, sizeof(IMAGE_SECTION_HEADER), d_header);
    b_data.replace(newFileOffset, sizeOfNewData, data);

    return parse();
}

#endif
