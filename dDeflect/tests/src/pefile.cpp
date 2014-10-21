#include "pefile.h"

PEFile::PEFile(QByteArray d) : parsed(false), sectionHeaders(NULL)
{
    b_data = d;
    data = b_data.data();
    length = b_data.length();
    // TODO: data moze sie zmienic!
}

PEFile::~PEFile()
{
    if(sectionHeaders)
        delete [] sectionHeaders;
}

bool PEFile::parse()
{
    parsed = false;

    data = b_data.data();

    dosHeader = (PIMAGE_DOS_HEADER)data;

    if(length < sizeof(IMAGE_DOS_HEADER) || dosHeader->e_magic != IMAGE_DOS_SIGNATURE)
        return false;

    ntHeaders = (PIMAGE_NT_HEADERS)((char*)dosHeader + dosHeader->e_lfanew);

    if(length < dosHeader->e_lfanew + sizeof(IMAGE_NT_HEADERS::Signature)
            || ntHeaders->Signature != IMAGE_NT_SIGNATURE)
        return false;

    fileHeader = (PIMAGE_FILE_HEADER)(&ntHeaders->FileHeader);

    if(length < (char*)fileHeader - (char*)dosHeader + sizeof(IMAGE_FILE_HEADER))
        return false;

    optionalHeader = (PIMAGE_OPTIONAL_HEADER)(&ntHeaders->OptionalHeader);
    optionalHeaderSize = fileHeader->SizeOfOptionalHeader;

    if(length < (char*)optionalHeader - (char*)dosHeader + optionalHeaderSize)
        return false;

    if(optionalHeader->Magic != IMAGE_NT_OPTIONAL_HDR_MAGIC)
        return false;

    numberOfSections = fileHeader->NumberOfSections;

    if(sectionHeaders)
        delete [] sectionHeaders;

    sectionHeaders = new PIMAGE_SECTION_HEADER[numberOfSections];

    for(unsigned int i = 0; i < numberOfSections; ++i)
        sectionHeaders[i] =
                (PIMAGE_SECTION_HEADER)((char*)optionalHeader +
                                        sizeof(IMAGE_OPTIONAL_HEADER) + i * sizeof(IMAGE_SECTION_HEADER));

    puts((char*)(sectionHeaders[numberOfSections-1]->Name));

    parsed = true;

    return true;
}

bool PEFile::makeSectionExecutable(unsigned int section)
{
    if(!parsed)
        return false;

    if(!sectionHeaders || section >= numberOfSections)
        return false;

    sectionHeaders[section]->Characteristics |=
            (IMAGE_SCN_CNT_CODE | IMAGE_SCN_MEM_EXECUTE | IMAGE_SCN_MEM_WRITE);
    sectionHeaders[section]->Characteristics &= (~IMAGE_SCN_MEM_DISCARDABLE);

    return true;
}

QByteArray PEFile::getData()
{
    return b_data;
}
