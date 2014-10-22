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

PEFile::PEFile(QByteArray d) : parsed(false), sectionHeadersIdx(NULL)
{
    b_data = d;
}

PEFile::~PEFile()
{
    if(sectionHeadersIdx)
        delete [] sectionHeadersIdx;
}

bool PEFile::parse()
{
    parsed = false;

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

    numberOfSections = fileHeader->NumberOfSections;

    if(sectionHeadersIdx)
        delete [] sectionHeadersIdx;

    sectionHeadersIdx = new unsigned int[numberOfSections];

    unsigned int firstSectionHeaderIdx = optionalHeaderIdx + sizeof(IMAGE_OPTIONAL_HEADER);

    for(unsigned int i = 0; i < numberOfSections; ++i)
        sectionHeadersIdx[i] = firstSectionHeaderIdx + i * sizeof(IMAGE_SECTION_HEADER);

   // TODO: Data directories (var)

    parsed = true;

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

unsigned int PEFile::getLastSectionNumber() const
{
    // TODO
    return 0;
}
