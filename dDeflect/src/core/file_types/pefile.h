#ifndef PEFILE_H
#define PEFILE_H

#ifdef __linux__
#include <core/sys_headers/winheader.h>
#else
#include <windef.h>
#endif

#include <QFile>
#include <QMap>
#include <QString>
#include <QList>
#include <QStringList>
#include <QCryptographicHash>
#include <core/file_types/pecodedefines.h>
#include <core/file_types/pehelpers.h>

class PEFile
{
private:
    bool parsed;
    bool is_x64;
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

    unsigned int getOptHdrFileAlignment();
    unsigned int getOptHdrSectionAlignment();
    size_t getOptHdrSizeOfCode();
    size_t getOptHdrSizeOfInitializedData();
    unsigned int getOptHdrAddressOfEntryPoint();
    unsigned int getNtHdrSignature();
    unsigned int getOptHdrNumberOfRvaAndSizes();
    uint64_t getOptHdrImageBase(); // TODO: 32/64

    void setOptHdrSizeOfCode(size_t size);
    void setOptHdrSizeOfInitializedData(size_t size);
    void setOptHdrSizeOfImage(size_t size);
    void setOptHdrSizeOfHeaders(size_t size);
    void setOptHdrAddressOfEntryPoint(unsigned int ep);

    PIMAGE_DOS_HEADER getDosHeader();

    PIMAGE_NT_HEADERS32 getNtHeaders32();
    PIMAGE_NT_HEADERS64 getNtHeaders64();

    PIMAGE_FILE_HEADER getFileHeader();

    PIMAGE_OPTIONAL_HEADER32 getOptionalHeader32();
    PIMAGE_OPTIONAL_HEADER64 getOptionalHeader64();

    PIMAGE_SECTION_HEADER getSectionHeader(unsigned int n);
    PIMAGE_DATA_DIRECTORY getDataDirectory(unsigned int n);

    bool parse();
    bool parse32();
    bool parse64();
    bool isPE_64(unsigned int pe_offset);

    size_t getFreeSpaceBeforeNextSectionMem(unsigned int section);
    size_t getFreeSpaceBeforeFirstSectionFile();
    unsigned int alignNumber(unsigned int number, unsigned int alignment);
    bool addDataToSectionExVirtual(unsigned int section, QByteArray data, unsigned int &fileOffset, unsigned int &memOffset);

    uint64_t generateCode(Wrapper *w, QMap<QByteArray, uint64_t> &ptrs);
    uint64_t generateString(QString str, QMap<QByteArray, uint64_t> &ptrs);
    uint64_t injectUniqueData(QByteArray data, QMap<QByteArray, uint64_t> &ptrs);

public:
    PEFile(QByteArray d);
    ~PEFile();

    bool injectCode(QList<InjectDescription> descs);

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

    /**
     * @brief Rozszerza sekcję i dodaje do niej dane.
     * @param data: Dane do dodania.
     * @param fileOffset: Obliczony offset dodanych danych w pliku.
     * @param memOffset: Obliczony offset dodanych danych w pmięci (RVA).
     * @return True w przypadku poprawnego dodania danych.
     */
    bool resizeLastSection(QByteArray data, unsigned int &fileOffset, unsigned int &memOffset);

    /**
     * @brief Dodanie danych do istniejącej sekcji bez rozszerzania.
     * @param section: Numer sekcji.
     * @param data: Dane
     * @param fileOffset: Obliczony offset dodanych danych w pliku.
     * @param memOffset: Obliczony offset dodanych danych w pmięci (RVA).
     * @return True w przypadku poprawnego dodania danych.
     */
    bool addDataToSection(unsigned int section, QByteArray data, unsigned int &fileOffset, unsigned int &memOffset);

    /**
     * @brief Dodanie danych do istniejącej sekcji z rozszerzeniem jeśli wymagane.
     * @param section: Numer sekcji.
     * @param data: Dane
     * @param fileOffset: Obliczony offset dodanych danych w pliku.
     * @param memOffset: Obliczony offset dodanych danych w pmięci (RVA).
     * @return True w przypadku poprawnego dodania danych.
     */
    bool addDataToSectionEx(unsigned int section, QByteArray data, unsigned int &fileOffset, unsigned int &memOffset);

    /**
     * @brief Pobiera informację czy sekcja jest wykonywalna.
     * @param section: numer sekcji.
     * @return Czy sekcja jest wykonywalna
     */
    bool isSectionExecutable(unsigned int section);

    /**
     * @brief Ustawia nowy EntryPoint.
     * @param newEP: nowy EP;
     * @return true w przypadku sukcesu.
     */
    bool setNewEntryPoint(unsigned int newEP);

    /**
     * @brief Pobiera EntryPoint.
     * @return EP
     */
    unsigned int getEntryPoint();

    /**
     * @brief Pobiera ImageBase
     * @return
     */
    uint64_t getImageBase(); // TODO: template w zależności od 32/64

    /**
     * @brief Dodaje nową wykonywalną sekcję.
     * @param name: Nazwa
     * @param data: Zawartość nowej sekcji.
     * @param fileOffset: Zwracany offset nowej sekcji w pliku.
     * @param memOffset: Offset nowej sekcji w pamięci.
     * @return True w przypadku sukcesu.
     */
    bool addNewSection(QString name, QByteArray data, unsigned int &fileOffset, unsigned int &memOffset);
};

#endif // PEFILE_H
