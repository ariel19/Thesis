#ifndef PEFILE_H
#define PEFILE_H

#include <windows.h>
#include <QFile>

class PEFile
{
private:
    bool parsed;
    QByteArray b_data;

    unsigned int dosHeaderIdx;
    unsigned int ntHeadersIdx;
    unsigned int fileHeaderIdx;
    unsigned int optionalHeaderIdx;

    size_t optionalHeaderSize;
    unsigned int numberOfSections;

    unsigned int *sectionHeadersIdx;

    PIMAGE_DOS_HEADER getDosHeader();
    PIMAGE_NT_HEADERS getNtHeaders();
    PIMAGE_FILE_HEADER getFileHeader();
    PIMAGE_OPTIONAL_HEADER getOptionalHeader();
    PIMAGE_SECTION_HEADER getSectionHeader(unsigned int n);

public:
    PEFile(QByteArray d);
    ~PEFile();
    bool parse();
    bool makeSectionExecutable(unsigned int section);
    QByteArray getData();
    unsigned int getLastSectionNumber() const;
};

#endif // PEFILE_H
