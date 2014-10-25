#include "pefile.h"

PIMAGE_DOS_HEADER PEFile::getDosHeader()
{
    return reinterpret_cast<PIMAGE_DOS_HEADER>(&(b_data.data()[dosHeaderIdx]));
}

PIMAGE_NT_HEADERS PEFile::getNtHeaders()
{
    return reinterpret_cast<PIMAGE_NT_HEADERS>(&(b_data.data()[ntHeadersIdx]));
}

PIMAGE_FILE_HEADER PEFile::getFileHeader()
{
    return reinterpret_cast<PIMAGE_FILE_HEADER>(&(b_data.data()[fileHeaderIdx]));
}

PIMAGE_OPTIONAL_HEADER PEFile::getOptionalHeader()
{
    return reinterpret_cast<PIMAGE_OPTIONAL_HEADER>(&(b_data.data()[optionalHeaderIdx]));
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

    PIMAGE_NT_HEADERS ntHeaders =
            reinterpret_cast<PIMAGE_NT_HEADERS>(reinterpret_cast<char*>(dosHeader) + dosHeader->e_lfanew);
    ntHeadersIdx = reinterpret_cast<char*>(ntHeaders) - reinterpret_cast<char*>(dosHeader);

    if(length < dosHeader->e_lfanew + sizeof(IMAGE_NT_HEADERS::Signature)
            || ntHeaders->Signature != IMAGE_NT_SIGNATURE)
        return false;

    PIMAGE_FILE_HEADER fileHeader = reinterpret_cast<PIMAGE_FILE_HEADER>(&ntHeaders->FileHeader);
    fileHeaderIdx = reinterpret_cast<char*>(fileHeader) - reinterpret_cast<char*>(dosHeader);

    if(length < (fileHeaderIdx + sizeof(IMAGE_FILE_HEADER)))
        return false;

    PIMAGE_OPTIONAL_HEADER optionalHeader = reinterpret_cast<PIMAGE_OPTIONAL_HEADER>(&ntHeaders->OptionalHeader);
    optionalHeaderSize = fileHeader->SizeOfOptionalHeader;
    optionalHeaderIdx = reinterpret_cast<char*>(optionalHeader) - reinterpret_cast<char*>(dosHeader);

    if(length < optionalHeaderIdx + optionalHeaderSize)
        return false;

    if(optionalHeader->Magic != IMAGE_NT_OPTIONAL_HDR_MAGIC)
        return false;

    // Data Directories
    numberOfDataDirectories = optionalHeader->NumberOfRvaAndSizes;

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

    unsigned int firstDataDirIdx =
            reinterpret_cast<char*>(&(optionalHeader->DataDirectory[0])) - reinterpret_cast<char*>(dosHeader);

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

    size_t newSizeOfRawData = alignNumber(qMax<unsigned int>(data.length(), header->Misc.VirtualSize), getOptionalHeader()->FileAlignment);

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

    getOptionalHeader()->SizeOfCode += newSizeOfRawData;
    getOptionalHeader()->SizeOfInitializedData += newSizeOfRawData;
    getOptionalHeader()->SizeOfImage =
            alignNumber(getSectionHeader(getLastSectionNumberMem())->VirtualAddress +
                        getSectionHeader(getLastSectionNumberMem())->Misc.VirtualSize,
                        getOptionalHeader()->SectionAlignment);

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
                                       getOptionalHeader()->FileAlignment);

    // Dopełnienie zerami nowych danych.
    size_t numOfZeros = numBytesToAdd - data.length();

    // Wielkość nowych danych bez zer.
    size_t actualDataLen = data.length();

    // Offset dla nowych danych.
    unsigned int newDataOffset = isVirtual ?
                alignNumber(b_data.length(), getOptionalHeader()->FileAlignment) :
                header->PointerToRawData + header->SizeOfRawData;

    // Dodanie zer do nowych danych.
    if(numOfZeros)
        data.append(QByteArray(numOfZeros, 0x00));

    //    getOptionalHeader()->SizeOfImage +=
    //            alignNumber(qMax<int>(header->SizeOfRawData - header->Misc.VirtualSize, 0) + actualDataLen, getOptionalHeader()->SectionAlignment);

    header->Misc.VirtualSize = header->SizeOfRawData + actualDataLen;
    header->SizeOfRawData += numBytesToAdd;

    getOptionalHeader()->SizeOfImage =
            alignNumber(header->VirtualAddress + header->Misc.VirtualSize, getOptionalHeader()->SectionAlignment);

    if(!isSectionExecutable(last))
    {
        // Wyrównany wcześniej
        getOptionalHeader()->SizeOfCode += header->SizeOfRawData;
        makeSectionExecutable(last);
    }
    else
        getOptionalHeader()->SizeOfCode += numBytesToAdd;

    if(isVirtual)
    {
        header->PointerToRawData = newDataOffset;
        header->Characteristics |= IMAGE_SCN_CNT_INITIALIZED_DATA;
        header->Characteristics &= ~IMAGE_SCN_CNT_UNINITIALIZED_DATA;
    }
    getOptionalHeader()->SizeOfInitializedData += numBytesToAdd;

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

bool PEFile::addDataToSectionEx(unsigned int section, QByteArray data, unsigned int &fileOffset, unsigned int &memOffset)
{
    if(!parsed || (section >= numberOfSections))
        return false;

    if(getSectionFreeSpace(section) >= static_cast<size_t>(data.length()))
        return addDataToSection(section, data, fileOffset, memOffset);

    if(isSectionRawDataEmpty(section))
        return addDataToSectionExVirtual(section, data, fileOffset, memOffset);

    PIMAGE_SECTION_HEADER header = getSectionHeader(section);

    // Za sekcją w pliku znajduje się inna sekcja i nie można jej rozszerzyć.
    if(section != getLastSectionNumberRaw())
        return false;

    size_t newSectionRawSize =
            alignNumber(header->Misc.VirtualSize + data.length(), getOptionalHeader()->FileAlignment);

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

    getOptionalHeader()->SizeOfImage =
            alignNumber(getSectionHeader(getLastSectionNumberMem())->VirtualAddress +
                        getSectionHeader(getLastSectionNumberMem())->Misc.VirtualSize,
                        getOptionalHeader()->SectionAlignment);

    getOptionalHeader()->SizeOfInitializedData += numBytesToAdd;

    if(!isSectionExecutable(section))
    {
        getOptionalHeader()->SizeOfCode += header->SizeOfRawData;
        makeSectionExecutable(section);
    }
    else
        getOptionalHeader()->SizeOfCode += numBytesToAdd;

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

    getOptionalHeader()->AddressOfEntryPoint = newEP;

    return true;
}
