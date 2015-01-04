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
#include <QByteArray>
#include <QTemporaryFile>
#include <cstdint>
#include <chrono>
#include <core/file_types/pecodedefines.h>


/**
 * @brief Klasa odpowiedzialna za parsowanie plików PE
 */
class PEFile
{
private:

    struct RelocationTable
    {
        struct TypeOffset
        {
            uint8_t Type;
            uint16_t Offset;
        };

        uint32_t VirtualAddress;
        uint32_t SizeOfBlock;
        QList<TypeOffset> TypeOffsets;

        bool addOffset(uint16_t offset, uint8_t type);
        QByteArray toBytes();
    };

    /**
     * @brief Flaga zawierająca informację czy plik został poprawnie sparsowany.
     */
    bool parsed;

    /**
     * @brief Flaga zawierająca informacje o architekturze pliku.
     */
    bool _is_x64;

    /**
     * @brief Zawartość pliku PE.
     */
    QByteArray b_data;


    /**
     * @brief Indeks struktury IMAGE_DOS_HEADER.
     */
    unsigned int dosHeaderIdx;

    /**
     * @brief Indeks struktury IMAGE_NT_HEADERS.
     */
    unsigned int ntHeadersIdx;

    /**
     * @brief Indeks struktury IMAGE_FILE_HEADER.
     */
    unsigned int fileHeaderIdx;

    /**
     * @brief Indeks struktury IMAGE_OPTIONAL_HEADER.
     */
    unsigned int optionalHeaderIdx;


    /**
     * @brief Rozmiar struktury IMAGE_OPTIONAL_HEADER.
     */
    size_t optionalHeaderSize;

    /**
     * @brief Liczba sekcji w pliku PE.
     */
    unsigned int numberOfSections;

    /**
     * @brief Liczba wpisów w IMAGE_DATA_DIRECTORY.
     */
    unsigned int numberOfDataDirectories;

    /**
     * @brief Generator liczb losowych.
     */
    std::default_random_engine gen;

    /**
     * @brief Tablica indeksów do nagłówków sekcji (IMAGE_SECTION_HEADER).
     */
    unsigned int *sectionHeadersIdx;

    /**
     * @brief Tablica indeksów wpisów w IMAGE_DATA_DIRECTORY.
     */
    unsigned int *dataDirectoriesIdx;

    /**
     * @brief Metoda odpowiedzialna za parsowanie pliku PE i wypełnianie wszystkich struktur.
     * @return True w przypadku poprawnie sparsowanego pliku.
     */
    bool parse();

    /**
     * @brief Wewnętrzna metoda badająca czy plik jest plikiem PE32+ (PE x64).
     * @param pe_offset Wyliczony offset do sprawdzenia.
     * @return True w przypadku pliku PE32+.
     */
    bool isPE_64(unsigned int pe_offset) const;


    /**
     * @brief Metoda pobierająca strukturę IMAGE_DOS_HEADER.
     * @return Wskaźnik do IMAGE_DOS_HEADER.
     */
    PIMAGE_DOS_HEADER getDosHeader();

    /**
     * @brief Metoda pobierająca 32-bitową wersję nagłówka IMAGE_NT_HEADERS.
     * @return Wskaźnik do IMAGE_NT_HEADERS32.
     */
    PIMAGE_NT_HEADERS32 getNtHeaders32();

    /**
     * @brief Metoda pobierająca 64-bitową wersję nagłówka IMAGE_NT_HEADERS.
     * @return Wskaźnik do IMAGE_NT_HEADERS64.
     */
    PIMAGE_NT_HEADERS64 getNtHeaders64();

    /**
     * @brief Metoda pobierająca strukturę IMAGE_FILE_HEADER.
     * @return Wskaźnik do IMAGE_FILE_HEADER.
     */
    PIMAGE_FILE_HEADER getFileHeader();

    /**
     * @brief Metoda pobierająca 32-bitową wersję nagłówka IMAGE_OPTIONAL_HEADER.
     * @return Wskaźnik do IMAGE_OPTIONAL_HEADER32.
     */
    PIMAGE_OPTIONAL_HEADER32 getOptionalHeader32();

    /**
     * @brief Metoda pobierająca 64-bitową wersję nagłówka IMAGE_OPTIONAL_HEADER.
     * @return Wskaźnik do IMAGE_OPTIONAL_HEADER64.
     */
    PIMAGE_OPTIONAL_HEADER64 getOptionalHeader64();

    /**
     * @brief Metoda pobierająca n-tą strukturę IMAGE_SECTION_HEADER.
     * @param n Numer sekcji, której nagłówek chcemy pobrać.
     * @return Wskaźnik na IMAGE_SECTION_HEADER.
     */
    PIMAGE_SECTION_HEADER getSectionHeader(unsigned int n);

    /**
     * @brief Metoda pobierająca n-tą strukturę IMAGE_DATA_DIRECTORY.
     * @param n Numer IMAGE_DATA_DIRECTORY, który chcemy pobrać.
     * @return Wskaźnik do IMAGE_DATA_DIRECTORY.
     */
    PIMAGE_DATA_DIRECTORY getDataDirectory(unsigned int n);

    /**
     * @brief Metoda pobierająca 32-bitową wersję struktury IMAGE_TLS_DIRECTORY.
     * @return Wskaźnik do IMAGE_TLS_DIRECTORY32. NULL gdy sekcja nie istnieje.
     */
    PIMAGE_TLS_DIRECTORY32 getTlsDirectory32();

    /**
     * @brief Metoda pobierająca 64-bitową wersję struktury IMAGE_TLS_DIRECTORY.
     * @return Wskaźnik do IMAGE_TLS_DIRECTORY64. NULL gdy sekcja nie istnieje.
     */
    PIMAGE_TLS_DIRECTORY64 getTlsDirectory64();


    /**
     * @brief Pobiera zawartość pola opisującego wyrównanie danych w pliku.
     * @return IMAGE_OPTIONAL_HEADER.FileAlignment.
     */
    unsigned int getOptHdrFileAlignment();

    /**
     * @brief Pobiera zawartość pola opisującego wyrównanie danych w pamięci.
     * @return IMAGE_OPTIONAL_HEADER.SectionAlignment.
     */
    unsigned int getOptHdrSectionAlignment();

    /**
     * @brief Pobiera sygnaturę pliku PE.
     * @return IMAGE_NT_HEADERS.Signature.
     */
    unsigned int getNtHdrSignature();

    /**
     * @brief Pobiera adres bazowy pliku w pamięci.
     * @return IMAGE_OPTIONAL_HEADER.ImageBase.
     */
    uint64_t getOptHdrImageBase();

    /**
     * @brief Pobiera liczbę wpisów w tablicy zawierającej IMAGE_DATA_DIRECTORY.
     * @return IMAGE_OPTIONAL_HEADER.NumberOfRvaAndSizes.
     */
    unsigned int getOptHdrNumberOfRvaAndSizes();

    /**
     * @brief Pobiera rozmiar kodu.
     * @return IMAGE_OPTIONAL_HEADER.SizeOfCode.
     */
    size_t getOptHdrSizeOfCode();

    /**
     * @brief Ustawia rozmiar kodu.
     * @param size Nowy rozmiar
     * @return True w przypadku powodzenia.
     */
    bool setOptHdrSizeOfCode(size_t size);

    /**
     * @brief Pobiera rozmiar zainicjalizowanych danych.
     * @return IMAGE_OPTIONAL_HEADER.SizeOfInitializedData
     */
    size_t getOptHdrSizeOfInitializedData();

    /**
     * @brief Ustawia rozmiar zainicjalizowanych danych.
     * @param size Nowy rozmiar
     * @return True w przypadku powodzenia.
     */
    bool setOptHdrSizeOfInitializedData(size_t size);

    /**
     * @brief Pobiera EntryPoint
     * @return IMAGE_OPTIONAL_HEADER.AddressOfEntryPoint
     */
    unsigned int getOptHdrAddressOfEntryPoint();

    /**
     * @brief Ustawia EntryPoint
     * @param ep Nowy adres EntryPoint
     * @return True w przypadku powodzenia
     */
    bool setOptHdrAddressOfEntryPoint(unsigned int ep);

    /**
     * @brief Ustawia rozmiar obrazu pliku w pamięci.
     * @param size Nowy rozmiar
     * @return True w przypadku powodzenia
     */
    bool setOptHdrSizeOfImage(size_t size);

    /**
     * @brief Ustawia rozmiar nagłówków.
     * @param size Nowy rozmiar
     * @return True w przypadku powodzenia.
     */
    bool setOptHdrSizeOfHeaders(size_t size);

    /**
     * @brief Pobiera index struktury IMAGE_TLS_DIRECTORY w pliku.
     * @return Offset od początku pliku.
     */
    uint64_t getTlsDirectoryFileOffset();






    /**
     * @brief Pobiera typ relokacji
     * @return Typ relokacji
     */
    uint8_t getRelocationType();

    /**
     * @brief Pobiera ilość wolnego miejsca przed następną sekcją w pamięci.
     * @param section Numer sekcji
     * @return Ilość miejsca w bajtach.
     */
    size_t getFreeSpaceBeforeNextSectionMem(unsigned int section);

    /**
     * @brief Pobiera ilość wolnego miejsca przed pierwszą sekcją w pliku.
     * @return Ilość miejsca w bajtach.
     */
    size_t getFreeSpaceBeforeFirstSectionFile();

    /**
     * @brief Metoda wyrównująca liczbę w górę do określonego zakresu.
     * @param number Liczba do wyrównania
     * @param alignment Wyrównanie
     * @return Wyrównana liczba
     */
    unsigned int alignNumber(unsigned int number, unsigned int alignment);

    /**
     * @brief Dodawanie danych do sekcji wirtualnej
     * @param section Numer sekcji
     * @param data Dane do dodania
     * @param fileOffset Offset w pliku do dodanych danych
     * @param memOffset Offset w pamięci do dodanych danych
     * @return True w przypadku powodzenia
     */
    bool addDataToSectionExVirtual(unsigned int section, QByteArray data, unsigned int &fileOffset, unsigned int &memOffset);










    QString getRandomSectionName();

    bool getRelocations(QList<RelocationTable> &rt);
    uint32_t getSectionByVirtualAddress(uint32_t va);
    uint32_t getRelocationsSize();
    uint32_t getRelocationsVirtualAddress();


    uint32_t fileOffsetToRVA(uint32_t fileOffset);





public:
    /**
     * @brief Konstruktor
     * @param d Zawartość pliku PE
     */
    PEFile(QByteArray d);

    ~PEFile();

    /**
     * @brief Sprawdza czy w pamięci przechowywany jest poprawny plik.
     * @return True jeżeli plik jest poprawny.
     */
    bool isValid();

    /**
     * @brief Zmienia uprawnienia sekcji, aby była ona wykonywalna.
     * @param section Numer sekcji.
     * @return True w przypadku sukcesu.
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
     * @param section Numer sekcji.
     * @return Ilość bajtów.
     */
    size_t getSectionFreeSpace(unsigned int section);

    /**
     * @brief Pobiera informację o tym, czy sekcja istnieje w pliku czy jest tylko wirtualna.
     * @param section Numer sekcji.
     * @return Ilość bajtów.
     */
    bool isSectionRawDataEmpty(unsigned int section);

    /**
     * @brief Rozszerza sekcję i dodaje do niej dane.
     * @param data Dane do dodania.
     * @param fileOffset Obliczony offset dodanych danych w pliku.
     * @param memOffset Obliczony offset dodanych danych w pmięci (RVA).
     * @return True w przypadku poprawnego dodania danych.
     */
    bool resizeLastSection(QByteArray data, unsigned int &fileOffset, unsigned int &memOffset);

    /**
     * @brief Dodanie danych do istniejącej sekcji bez rozszerzania.
     * @param section Numer sekcji.
     * @param data Dane
     * @param fileOffset Obliczony offset dodanych danych w pliku.
     * @param memOffset Obliczony offset dodanych danych w pmięci (RVA).
     * @return True w przypadku poprawnego dodania danych.
     */
    bool addDataToSection(unsigned int section, QByteArray data, unsigned int &fileOffset, unsigned int &memOffset);

    /**
     * @brief Dodanie danych do istniejącej sekcji z rozszerzeniem jeśli wymagane.
     * @param section: Numer sekcji.
     * @param data Dane
     * @param fileOffset Obliczony offset dodanych danych w pliku.
     * @param memOffset Obliczony offset dodanych danych w pmięci (RVA).
     * @param changeVirtual Informacja czy zmieniać sekcję wirtualną.
     * @return True w przypadku poprawnego dodania danych.
     */
    bool addDataToSectionEx(unsigned int section, QByteArray data, unsigned int &fileOffset, unsigned int &memOffset,
                            bool changeVirtual = false);

    /**
     * @brief Pobiera informację czy sekcja jest wykonywalna.
     * @param section numer sekcji.
     * @return Czy sekcja jest wykonywalna
     */
    bool isSectionExecutable(unsigned int section);

    /**
     * @brief Ustawia nowy EntryPoint.
     * @param newEP nowy EP;
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
    uint64_t getImageBase();

    /**
     * @brief Dodaje nową wykonywalną sekcję.
     * @param name Nazwa
     * @param data Zawartość nowej sekcji.
     * @param fileOffset Zwracany offset nowej sekcji w pliku.
     * @param memOffset Offset nowej sekcji w pamięci.
     * @param useReserved Flaga zezwalająca na użycie zarezerwowanego obszaru.
     * @return True w przypadku sukcesu.
     */
    bool addNewSection(QString name, QByteArray data, unsigned int &fileOffset, unsigned int &memOffset, bool useReserved = false);

    QByteArray getTextSection();
    uint32_t getTextSectionOffset();
    bool addRelocations(QList<uint64_t> relocations);

    template <typename Register>
    uint64_t injectUniqueData(BinaryCode<Register> data, QMap<QByteArray, uint64_t> &ptrs, QList<uint64_t> &relocations);
    bool hasTls();
    bool setTlsDirectoryAddress(uint64_t addr);
    uint64_t getTlsDirectoryAddress();

    QList<uint64_t> getTlsCallbacks();
    bool is_x64();
    bool is_x86();
    uint64_t getAddressAtCallInstructionOffset(uint32_t offset);
    bool setAddressAtCallInstructionOffset(uint32_t offset, uint64_t address);
    uint64_t generateString(QString str, QMap<QByteArray, uint64_t> &ptrs);

    /**
     * @brief Pobiera rozmiar struktury IMAGE_TLS_DIRECTORY.
     * @return Rozmiar
     */
    size_t getImageTlsDirectorySize() const;

    /**
     * @brief Pobiera adres wirtualny tablicy ze wskaźnikami do funkcji TLS.
     * @return IMAGE_TLS_DIRECTORY.AddressOfCallBacks
     */
    uint64_t getTlsAddressOfCallBacks();

    uint64_t injectUniqueData(QByteArray data, QMap<QByteArray, uint64_t> &ptrs, bool *inserted = NULL);

    /**
     * @brief Ustawia adres wirtualny tablicy ze wskaźnikami do funkcji TLS.
     * @param addr Nowy adres
     * @return True w przypadku powodzenia
     */
    bool setTlsAddressOfCallBacks(uint64_t addr);

    /**
     * @brief Pobiera adres wirtualny pod którym znajduje się index TLS.
     * @return IMAGE_TLS_DIRECTORY.AddressOfIndex
     */
    uint64_t getTlsAddressOfIndex();

    /**
     * @brief Ustawia adres wirtualny pod którym znajduje się index TLS.
     * @param addr Nowy adres
     * @return True w przypadku powodzenia
     */
    bool setTlsAddressOfIndex(uint64_t addr);
};

#endif
