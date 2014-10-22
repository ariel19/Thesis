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

PEFile::PEFile(QByteArray d) : parsed(false), sectionHeadersIdx(NULL)
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
