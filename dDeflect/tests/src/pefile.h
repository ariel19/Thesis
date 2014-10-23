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
    unsigned int numberOfDataDirectories;

    unsigned int *sectionHeadersIdx;
    unsigned int *dataDirectoriesIdx;

    PIMAGE_DOS_HEADER getDosHeader();
    PIMAGE_NT_HEADERS getNtHeaders();
    PIMAGE_FILE_HEADER getFileHeader();
    PIMAGE_OPTIONAL_HEADER getOptionalHeader();
    PIMAGE_SECTION_HEADER getSectionHeader(unsigned int n);
    PIMAGE_DATA_DIRECTORY getDataDirectory(unsigned int n);

    bool parse();

    size_t getFreeSpaceBeforeNextSectionMem(unsigned int section);
    unsigned int alignNumber(unsigned int number, unsigned int alignment);

public:
    PEFile(QByteArray d);
    ~PEFile();

    /**
     * @brief Sprawdza czy w pamięci przechowywany jest poprawny plik.
     * @return
     */
    bool isValid();

    /**
     * @brief Zmienia uprawnienia sekcji, aby była ona wykonywalna.
     * @param section: Numer sekcji.
     * @return true w przypadku sukcesu.
     */
    bool makeSectionExecutable(unsigned int section);

    /**
     * @brief Pobiera zawartość pliku PE.
     * @return Plik PE.
     */
    QByteArray getData();

    /**
     * @brief Pobiera numer ostatniej sekcji w pamięci.
     * @return Numer ostatniej sekcji w pamięci.
     */
    unsigned int getLastSectionNumberMem();

    /**
     * @brief Pobiera numer ostatniej sekcji w pliku.
     * @return Numer ostatniej sekcji w pliku.
     */
    unsigned int getLastSectionNumberRaw();

    /**
     * @brief Pobiera ilość sekcji w pliku.
     * @return Ilość sekcji.
     */
    unsigned int getNumberOfSections() const;

    /**
     * @brief Zwraca ilość wolnego miejsca w sekcji.
     * @param section: Numer sekcji.
     * @return Ilość bajtów.
     */
    size_t getSectionFreeSpace(unsigned int section);

    /**
     * @brief Pobiera informację o tym, czy sekcja istnieje w pliku czy jest tylko wirtualna.
     * @param section: Numer sekcji.
     * @return Ilość bajtów.
     */
    bool isSectionRawDataEmpty(unsigned int section);

    bool resizeLastSection(QByteArray data, unsigned int &offset);

    bool isSectionExecutable(unsigned int section);
};

#endif // PEFILE_H
