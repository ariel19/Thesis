#include <core/file_types/pefile.h>

unsigned int PEFile::getOptHdrFileAlignment()
{
    return _is_x64 ? getOptionalHeader64()->FileAlignment : getOptionalHeader32()->FileAlignment;
}

unsigned int PEFile::getOptHdrSectionAlignment()
{
    return _is_x64 ? getOptionalHeader64()->SectionAlignment : getOptionalHeader32()->SectionAlignment;
}

size_t PEFile::getOptHdrSizeOfCode()
{
    return _is_x64 ? getOptionalHeader64()->SizeOfCode : getOptionalHeader32()->SizeOfCode;
}

size_t PEFile::getOptHdrSizeOfInitializedData()
{
    return _is_x64 ? getOptionalHeader64()->SizeOfInitializedData : getOptionalHeader32()->SizeOfInitializedData;
}

unsigned int PEFile::getOptHdrAddressOfEntryPoint()
{
    return _is_x64 ? getOptionalHeader64()->AddressOfEntryPoint : getOptionalHeader32()->AddressOfEntryPoint;
}

unsigned int PEFile::getNtHdrSignature()
{
    return _is_x64 ? getNtHeaders64()->Signature : getNtHeaders32()->Signature;
}

unsigned int PEFile::getOptHdrNumberOfRvaAndSizes()
{
    return _is_x64 ? getOptionalHeader64()->NumberOfRvaAndSizes : getOptionalHeader32()->NumberOfRvaAndSizes;
}

uint64_t PEFile::getOptHdrImageBase()
{
    return _is_x64 ? getOptionalHeader64()->ImageBase : getOptionalHeader32()->ImageBase;
}

bool PEFile::setOptHdrSizeOfCode(size_t size)
{
    if(_is_x64)
        getOptionalHeader64()->SizeOfCode = size;
    else
        getOptionalHeader32()->SizeOfCode = size;

    return true;
}

bool PEFile::setOptHdrSizeOfInitializedData(size_t size)
{
    if(_is_x64)
        getOptionalHeader64()->SizeOfInitializedData = size;
    else
        getOptionalHeader32()->SizeOfInitializedData = size;

    return true;
}

bool PEFile::setOptHdrSizeOfImage(size_t size)
{
    if(_is_x64)
        getOptionalHeader64()->SizeOfImage = size;
    else
        getOptionalHeader32()->SizeOfImage = size;

    return true;
}

bool PEFile::setOptHdrSizeOfHeaders(size_t size)
{
    if(_is_x64)
        getOptionalHeader64()->SizeOfHeaders = size;
    else
        getOptionalHeader32()->SizeOfHeaders = size;

    return true;
}

bool PEFile::setOptHdrAddressOfEntryPoint(unsigned int ep)
{
    if(_is_x64)
        getOptionalHeader64()->AddressOfEntryPoint = ep;
    else
        getOptionalHeader32()->AddressOfEntryPoint = ep;

    return true;
}

PIMAGE_DOS_HEADER PEFile::getDosHeader()
{
    return reinterpret_cast<PIMAGE_DOS_HEADER>(&(b_data.data()[dosHeaderIdx]));
}

PIMAGE_NT_HEADERS32 PEFile::getNtHeaders32()
{
    if(_is_x64)
        return nullptr;

    return reinterpret_cast<PIMAGE_NT_HEADERS32>(&(b_data.data()[ntHeadersIdx]));
}

PIMAGE_NT_HEADERS64 PEFile::getNtHeaders64()
{
    if(!_is_x64)
        return nullptr;

    return reinterpret_cast<PIMAGE_NT_HEADERS64>(&(b_data.data()[ntHeadersIdx]));
}

PIMAGE_FILE_HEADER PEFile::getFileHeader()
{
    return reinterpret_cast<PIMAGE_FILE_HEADER>(&(b_data.data()[fileHeaderIdx]));
}

PIMAGE_OPTIONAL_HEADER32 PEFile::getOptionalHeader32()
{
    if(_is_x64)
        return nullptr;

    return reinterpret_cast<PIMAGE_OPTIONAL_HEADER32>(&(b_data.data()[optionalHeaderIdx]));
}

PIMAGE_OPTIONAL_HEADER64 PEFile::getOptionalHeader64()
{
    if(!_is_x64)
        return nullptr;

    return reinterpret_cast<PIMAGE_OPTIONAL_HEADER64>(&(b_data.data()[optionalHeaderIdx]));
}

PIMAGE_SECTION_HEADER PEFile::getSectionHeader(unsigned int n)
{
    return n >= numberOfSections ?
                nullptr : reinterpret_cast<PIMAGE_SECTION_HEADER>(&(b_data.data()[sectionHeadersIdx[n]]));
}

PIMAGE_DATA_DIRECTORY PEFile::getDataDirectory(unsigned int n)
{
    return n >= numberOfDataDirectories ?
                nullptr : reinterpret_cast<PIMAGE_DATA_DIRECTORY>(&(b_data.data()[dataDirectoriesIdx[n]]));
}

PIMAGE_TLS_DIRECTORY32 PEFile::getTlsDirectory32()
{
    if(_is_x64)
        return nullptr;

    uint64_t tls_offset = getTlsDirectoryFileOffset();
    return tls_offset == 0 ? nullptr : reinterpret_cast<PIMAGE_TLS_DIRECTORY32>(&(b_data.data()[tls_offset]));
}

PIMAGE_TLS_DIRECTORY64 PEFile::getTlsDirectory64()
{
    if(!_is_x64)
        return nullptr;

    uint64_t tls_offset = getTlsDirectoryFileOffset();
    return tls_offset == 0 ? nullptr : reinterpret_cast<PIMAGE_TLS_DIRECTORY64>(&(b_data.data()[tls_offset]));
}

uint64_t PEFile::getTlsDirectoryFileOffset()
{
    uint32_t va = getDataDirectory(IMAGE_DIRECTORY_ENTRY_TLS)->VirtualAddress;

    if(va == 0)
        return 0;

    PIMAGE_SECTION_HEADER hdr = getSectionHeader(getSectionByVirtualAddress(va));

    if(!hdr)
        return 0;

    return hdr->PointerToRawData + (va - hdr->VirtualAddress);
}

size_t PEFile::getImageTlsDirectorySize() const
{
    return _is_x64 ? sizeof(IMAGE_TLS_DIRECTORY64) : sizeof(IMAGE_TLS_DIRECTORY32);
}

uint64_t PEFile::getTlsAddressOfCallBacks()
{
    if(!parsed)
        return 0;

    return _is_x64 ?
                (getTlsDirectory64() ? getTlsDirectory64()->AddressOfCallBacks : 0) :
                (getTlsDirectory32() ? getTlsDirectory32()->AddressOfCallBacks : 0);
}

bool PEFile::setTlsAddressOfCallBacks(uint64_t addr)
{
    if(!parsed)
        return false;

    if(_is_x64)
    {
        if(getTlsDirectory64())
            getTlsDirectory64()->AddressOfCallBacks = addr;
        else
        {
            LOG_ERROR("Setting new TLS AddressOfCallbacks failed");
            return false;
        }
    }
    else
    {
        if(getTlsDirectory32())
            getTlsDirectory32()->AddressOfCallBacks = addr;
        else
        {
            LOG_ERROR("Setting new TLS AddressOfCallbacks failed");
            return false;
        }
    }

    return true;
}

uint64_t PEFile::getTlsAddressOfIndex()
{
    if(!parsed)
        return 0;

    return _is_x64 ?
                (getTlsDirectory64() ? getTlsDirectory64()->AddressOfIndex : 0) :
                (getTlsDirectory32() ? getTlsDirectory32()->AddressOfIndex: 0);
}

bool PEFile::setTlsAddressOfIndex(uint64_t addr)
{
    if(!parsed)
        return false;

    if(_is_x64)
    {
        if(getTlsDirectory64())
            getTlsDirectory64()->AddressOfIndex = addr;
        else
        {
            LOG_ERROR("Setting new TLS AddressOfIndex failed");
            return false;
        }
    }
    else
    {
        if(getTlsDirectory32())
            getTlsDirectory32()->AddressOfIndex = addr;
        else
        {
            LOG_ERROR("Setting new TLS AddressOfIndex failed");
            return false;
        }
    }

    return true;
}

uint8_t PEFile::getRelocationType()
{
    return _is_x64 ? IMAGE_REL_BASED_DIR64 : IMAGE_REL_BASED_HIGHLOW;
}

PEFile::PEFile(QByteArray d) :
    BinaryFile(d),
    _is_x64(false),
    gen(std::chrono::system_clock::now().time_since_epoch().count()),
    sectionHeadersIdx(NULL),
    dataDirectoriesIdx(NULL)
{
    parsed = parse();
}

PEFile::~PEFile()
{
    if(sectionHeadersIdx)
        delete [] sectionHeadersIdx;

    if(dataDirectoriesIdx)
        delete [] dataDirectoriesIdx;
}


uint64_t PEFile::generateString(QString str, QMap<QByteArray, uint64_t> &ptrs)
{
    return injectUniqueData(QByteArray(str.toStdString().c_str(), str.length() + 1), ptrs);
}

uint64_t PEFile::injectUniqueData(QByteArray data, QMap<QByteArray, uint64_t> &ptrs, bool *inserted)
{
    QByteArray hash = QCryptographicHash::hash(data, QCryptographicHash::Sha3_512);

    if(inserted)
        *inserted = false;

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
    {
        LOG_ERROR("No more bytes can be added to the file.");
        return 0;
    }

    uint64_t offset = memOffset + getImageBase();
    ptrs.insert(hash, offset);
    if(inserted)
        *inserted = true;

    return offset;
}

template <typename Register>
uint64_t PEFile::injectUniqueData(BinaryCode<Register> data, QMap<QByteArray, uint64_t> &ptrs, QList<uint64_t> &relocations)
{
    bool inserted;
    uint64_t offset = injectUniqueData(data.getBytes(), ptrs, &inserted);

    if(inserted)
        relocations.append(data.getRelocations(offset));

    return offset;
}
template uint64_t PEFile::injectUniqueData(BinaryCode<Registers_x86> data, QMap<QByteArray, uint64_t> &ptrs, QList<uint64_t> &relocations);
template uint64_t PEFile::injectUniqueData(BinaryCode<Registers_x64> data, QMap<QByteArray, uint64_t> &ptrs, QList<uint64_t> &relocations);

QString PEFile::getRandomSectionName()
{
    QString name;
    std::uniform_int_distribution<int> no_letters(1, 7);
    std::uniform_int_distribution<int> char_gen('a', 'z');

    name += '.';
    int n = no_letters(gen);
    for(int i = 0; i < n; ++i)
        name += static_cast<char>(char_gen(gen));

    return name;
}

bool PEFile::addRelocations(QList<uint64_t> relocations)
{
    if(!parsed)
        return false;

    if(getRelocationsSize() == 0)
        return true;

    QList<RelocationTable> reloc_table;
    if(!getRelocations(reloc_table))
        return false;

    // Dodawanie do tablicy
    foreach(uint64_t abs_addr, relocations)
    {
        uint32_t rel_addr = abs_addr - getImageBase();
        uint32_t va = rel_addr & 0xFFFFF000;
        uint32_t offset = rel_addr & 0x00000FFF;

        QList<RelocationTable> new_reloc_table;
        bool added = false;

        for(QList<RelocationTable>::iterator it = reloc_table.begin(); it != reloc_table.end(); ++it)
        {
            if(it->VirtualAddress < va || (added && it->VirtualAddress > va))
            {
                new_reloc_table.append(*it);
                continue;
            }

            if(it->VirtualAddress == va)
            {
                RelocationTable r = *it;
                r.addOffset(offset, getRelocationType());
                new_reloc_table.append(r);

                added = true;
            }

            if(!added && it->VirtualAddress > va)
            {
                RelocationTable r;
                r.VirtualAddress = va;
                r.SizeOfBlock = IMAGE_SIZEOF_BASE_RELOCATION;
                r.addOffset(offset, getRelocationType());
                new_reloc_table.append(r);

                added = true;
                new_reloc_table.append(*it);
            }
        }

        if(!added)
        {
            RelocationTable r;
            r.VirtualAddress = va;
            r.SizeOfBlock = IMAGE_SIZEOF_BASE_RELOCATION;
            r.addOffset(offset, getRelocationType());
            new_reloc_table.append(r);
        }

        reloc_table = new_reloc_table;
    }

    QByteArray raw_table;
    foreach(RelocationTable rt, reloc_table)
        raw_table.append(rt.toBytes());

    unsigned int file_offset, mem_offset;
    if(!addNewSection(getRandomSectionName(), raw_table, file_offset, mem_offset))
        return false;

    getDataDirectory(IMAGE_DIRECTORY_ENTRY_BASERELOC)->Size = raw_table.length();
    getDataDirectory(IMAGE_DIRECTORY_ENTRY_BASERELOC)->VirtualAddress = mem_offset;

    return true;
}

bool PEFile::hasTls()
{
    if(!parsed)
        return false;

    return getDataDirectory(IMAGE_DIRECTORY_ENTRY_TLS)->VirtualAddress == 0;
}

bool PEFile::setTlsDirectoryAddress(uint64_t addr)
{
    if(!parsed)
        return false;

    getDataDirectory(IMAGE_DIRECTORY_ENTRY_TLS)->VirtualAddress = addr;
    getDataDirectory(IMAGE_DIRECTORY_ENTRY_TLS)->Size = getImageTlsDirectorySize();

    return true;
}

uint64_t PEFile::getTlsDirectoryAddress()
{
    if(!parsed)
        return 0;

    return getDataDirectory(IMAGE_DIRECTORY_ENTRY_TLS)->VirtualAddress;
}

QList<uint64_t> PEFile::getTlsCallbacks()
{
    QList<uint64_t> tlsCallbacks;

    uint32_t va = getTlsAddressOfCallBacks() - getImageBase();
    PIMAGE_SECTION_HEADER hdr = getSectionHeader(getSectionByVirtualAddress(va));

    if(!hdr)
        return tlsCallbacks;

    uint32_t fileptr = hdr->PointerToRawData + (va - hdr->VirtualAddress);
    uint64_t value = _is_x64 ? *reinterpret_cast<uint64_t*>(&b_data.data()[fileptr]) : *reinterpret_cast<uint32_t*>(&b_data.data()[fileptr]);

    while(value != 0)
    {
        tlsCallbacks.append(value);
        fileptr += (_is_x64 ? CodeDefines<Registers_x64>::stackCellSize : CodeDefines<Registers_x86>::stackCellSize);
        value = _is_x64 ? *reinterpret_cast<uint64_t*>(&b_data.data()[fileptr]) : *reinterpret_cast<uint32_t*>(&b_data.data()[fileptr]);
    }

    return tlsCallbacks;
}

bool PEFile::is_x64() const
{
    return parsed && _is_x64;
}

bool PEFile::is_x86() const
{
    return parsed && !_is_x64;
}

uint64_t PEFile::getAddressAtCallInstructionOffset(uint32_t offset)
{
    if(!parsed)
        return 0;

    int32_t call_off = *reinterpret_cast<int32_t*>(&b_data.data()[offset]);
    uint64_t call_addr = getImageBase() + fileOffsetToRVA(offset + 4) + call_off;

    return call_addr;
}

bool PEFile::setAddressAtCallInstructionOffset(uint32_t offset, uint64_t address)
{
    if(!parsed)
        return false;

    uint32_t new_call_off = (address - getImageBase()) - fileOffsetToRVA(offset + 4);
    *reinterpret_cast<int32_t*>(&b_data.data()[offset]) = new_call_off;

    return true;
}

bool PEFile::getRelocations(QList<RelocationTable> &rt)
{
    if(getRelocationsSize() == 0)
        return true;

    PIMAGE_SECTION_HEADER hdr = getSectionHeader(getSectionByVirtualAddress(getRelocationsVirtualAddress()));
    if(!hdr)
        return false;

    uint32_t shift = getRelocationsVirtualAddress() - hdr->VirtualAddress;
    uint32_t relocBase = hdr->PointerToRawData + shift;
    char *raw = b_data.data();
    uint32_t i = 0;

    while(i < getRelocationsSize())
    {
        IMAGE_BASE_RELOCATION reloc_tab = *reinterpret_cast<IMAGE_BASE_RELOCATION*>(&raw[relocBase + i]);

        RelocationTable t;
        t.SizeOfBlock = reloc_tab.SizeOfBlock;
        t.VirtualAddress = reloc_tab.VirtualAddress;

        printf("%x: %d\n", t.VirtualAddress, t.SizeOfBlock);

        uint32_t j = IMAGE_SIZEOF_BASE_RELOCATION;

        while(j < t.SizeOfBlock)
        {
            RelocationTable::TypeOffset typeOffset;

            typeOffset.Type = ((*reinterpret_cast<uint16_t*>(&raw[relocBase + i + j])) & 0xF000) >> 12;
            typeOffset.Offset = (*reinterpret_cast<uint16_t*>(&raw[relocBase + i + j])) & 0x0FFF;

            if(typeOffset.Type == IMAGE_REL_BASED_ABSOLUTE)
                t.SizeOfBlock -= 2;
            else
                t.TypeOffsets.append(typeOffset);
            j += 2;
        }

        rt.append(t);
        i += reloc_tab.SizeOfBlock;
    }

    return true;
}

uint32_t PEFile::getSectionByVirtualAddress(uint32_t va)
{
    for(unsigned int i = 0; i < getNumberOfSections(); ++i)
    {
        uint32_t sva = getSectionHeader(i)->VirtualAddress;
        uint32_t svs = getSectionHeader(i)->Misc.VirtualSize;

        if(sva <= va && sva + svs > va)
            return i;
    }

    return 0;
}

uint32_t PEFile::getRelocationsSize()
{
    PIMAGE_DATA_DIRECTORY relocDir = getDataDirectory(IMAGE_DIRECTORY_ENTRY_BASERELOC);

    return relocDir->Size;
}

uint32_t PEFile::getRelocationsVirtualAddress()
{
    PIMAGE_DATA_DIRECTORY relocDir = getDataDirectory(IMAGE_DIRECTORY_ENTRY_BASERELOC);

    return relocDir->VirtualAddress;
}

uint32_t PEFile::fileOffsetToRVA(uint32_t fileOffset)
{
    for(unsigned int i = 0; i < numberOfSections; ++i)
    {
        PIMAGE_SECTION_HEADER hdr = getSectionHeader(i);
        if(fileOffset >= hdr->PointerToRawData)
        {
            if(fileOffset < hdr->PointerToRawData + hdr->SizeOfRawData)
            {
                return fileOffset - hdr->PointerToRawData + hdr->VirtualAddress;
            }
        }
    }

    return 0;
}

bool PEFile::is_valid() const
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

    _is_x64 = isPE_64(dosHeader->e_lfanew);

    PIMAGE_NT_HEADERS32 ntHeaders32 = NULL;
    PIMAGE_NT_HEADERS64 ntHeaders64 = NULL;

    if(_is_x64)
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

    PIMAGE_FILE_HEADER fileHeader = _is_x64 ?
                reinterpret_cast<PIMAGE_FILE_HEADER>(&ntHeaders64->FileHeader) :
                reinterpret_cast<PIMAGE_FILE_HEADER>(&ntHeaders32->FileHeader);

    fileHeaderIdx = reinterpret_cast<char*>(fileHeader) - reinterpret_cast<char*>(dosHeader);

    if(length < (fileHeaderIdx + sizeof(IMAGE_FILE_HEADER)))
        return false;

    PIMAGE_OPTIONAL_HEADER32 optionalHeader32 = NULL;
    PIMAGE_OPTIONAL_HEADER64 optionalHeader64 = NULL;

    if(_is_x64)
        optionalHeader64 = reinterpret_cast<PIMAGE_OPTIONAL_HEADER64>(&ntHeaders64->OptionalHeader);
    else
        optionalHeader32 = reinterpret_cast<PIMAGE_OPTIONAL_HEADER32>(&ntHeaders32->OptionalHeader);

    optionalHeaderSize = fileHeader->SizeOfOptionalHeader;
    optionalHeaderIdx = _is_x64 ?
                (reinterpret_cast<char*>(optionalHeader64) - reinterpret_cast<char*>(dosHeader)) :
                (reinterpret_cast<char*>(optionalHeader32) - reinterpret_cast<char*>(dosHeader));

    if(length < optionalHeaderIdx + optionalHeaderSize)
        return false;

    if(_is_x64 && optionalHeader64->Magic != IMAGE_NT_OPTIONAL_HDR64_MAGIC)
        return false;

    if(!_is_x64 && optionalHeader32->Magic != IMAGE_NT_OPTIONAL_HDR32_MAGIC)
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

    unsigned int firstDataDirIdx = _is_x64 ?
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

bool PEFile::isPE_64(unsigned int pe_offset) const
{
    pe_offset += sizeof(DWORD);

    return *reinterpret_cast<const WORD*>(&b_data.data()[pe_offset]) != IMAGE_FILE_MACHINE_I386;
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
    if(section >= numberOfSections)
        return false;

    getSectionHeader(section)->Characteristics |=
            (IMAGE_SCN_CNT_CODE | IMAGE_SCN_MEM_EXECUTE | IMAGE_SCN_MEM_WRITE);
    getSectionHeader(section)->Characteristics &= (~IMAGE_SCN_MEM_DISCARDABLE);

    return true;
}

unsigned int PEFile::getLastSectionNumberMem()
{
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

    uint32_t flags = IMAGE_SCN_MEM_EXECUTE | IMAGE_SCN_MEM_WRITE | IMAGE_SCN_CNT_CODE;

    return (getSectionHeader(section)->Characteristics & flags) == flags;
}

bool PEFile::setNewEntryPoint(unsigned int newEP)
{
    if(!parsed)
        return false;

    if(newEP > getSectionHeader(getLastSectionNumberMem())->VirtualAddress +
            getSectionHeader(getLastSectionNumberMem())->Misc.VirtualSize)
    {
        LOG_ERROR("New EntryPoint outside data!");
        return false;
    }

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

bool PEFile::addNewSection(QString name, QByteArray data, unsigned int &fileOffset, unsigned int &memOffset, bool useReserved)
{
    if(!parsed)
        return false;

    unsigned int newHeaderOffset = sectionHeadersIdx[getNumberOfSections() - 1] + sizeof(IMAGE_SECTION_HEADER);
    unsigned int newFileOffset = alignNumber(b_data.length(), getOptHdrFileAlignment());

    // Header się nie zmieści.
    if(getFreeSpaceBeforeFirstSectionFile() < sizeof(IMAGE_SECTION_HEADER) * (useReserved ? 1 : 2))
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
    if(_is_x64)
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

QByteArray PEFile::getTextSection()
{
    if(!parsed)
        return QByteArray();

    PIMAGE_SECTION_HEADER text_hdr = getSectionHeader(getSectionByVirtualAddress(getEntryPoint() + getImageBase()));
    if(!text_hdr)
        return QByteArray();

    return QByteArray(&b_data.data()[text_hdr->PointerToRawData], text_hdr->Misc.VirtualSize);
}

uint32_t PEFile::getTextSectionOffset()
{
    if(!parsed)
        return 0;

    PIMAGE_SECTION_HEADER text_hdr = getSectionHeader(getSectionByVirtualAddress(getEntryPoint() + getImageBase()));
    if(!text_hdr)
        return 0;

    return text_hdr->PointerToRawData;
}

bool PEFile::RelocationTable::addOffset(uint16_t offset, uint8_t type)
{
    bool added = false, ok = false;
    QList<TypeOffset> new_to;

    TypeOffset to;
    to.Type = type;
    to.Offset = offset;

    for(QList<TypeOffset>::iterator it = TypeOffsets.begin(); it != TypeOffsets.end(); ++it)
    {
        if(it->Offset == offset)
            added = true;

        if(it->Offset > offset && !added)
        {
            added = ok = true;
            new_to.append(to);
        }

        new_to.append(*it);
    }

    if(!added)
    {
        new_to.append(to);
        ok = true;
    }

    TypeOffsets = new_to;

    if(ok)
        SizeOfBlock += sizeof(uint16_t);

    return ok;
}

QByteArray PEFile::RelocationTable::toBytes()
{
    QByteArray bytes;

    if(SizeOfBlock % 4 != 0)
    {
        TypeOffset align;
        align.Type = 0;
        align.Offset = 0;

        SizeOfBlock += sizeof(uint16_t);
        TypeOffsets.append(align);
    }

    bytes.append(reinterpret_cast<const char*>(&VirtualAddress), sizeof(uint32_t));
    bytes.append(reinterpret_cast<const char*>(&SizeOfBlock), sizeof(uint32_t));

    foreach(TypeOffset to, TypeOffsets)
    {
        uint16_t bin_to = (to.Type << 12) | to.Offset;
        bytes.append(reinterpret_cast<const char*>(&bin_to), sizeof(uint16_t));
    }

    return bytes;
}
