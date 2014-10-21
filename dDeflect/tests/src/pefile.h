#ifndef PEFILE_H
#define PEFILE_H

#include <windows.h>
#include <QFile>

class PEFile
{
private:
    bool parsed;
    QByteArray b_data;

    char *data;
    size_t length;

    PIMAGE_DOS_HEADER dosHeader;
    PIMAGE_NT_HEADERS ntHeaders;
    PIMAGE_FILE_HEADER fileHeader;
    PIMAGE_OPTIONAL_HEADER optionalHeader;
    size_t optionalHeaderSize;
    unsigned int numberOfSections;

    PIMAGE_SECTION_HEADER *sectionHeaders;

public:
    PEFile(QByteArray d);
    ~PEFile();
    bool parse();
    bool makeSectionExecutable(unsigned int section);
    QByteArray getData();
};

#endif // PEFILE_H
