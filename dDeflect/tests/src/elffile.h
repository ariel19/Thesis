#ifndef ELFFILE_H
#define ELFFILE_H

// #include <elf.h>
#include "elf.h"
#include <QFile>
#include <QString>
#include <QList>

typedef uint32_t offset_t;
typedef Elf64_Half esize_t;
typedef Elf64_Off ex_offset_t;

class ELF {
    struct classes {
        enum CLASS {
            NONE,
            ELF32,
            ELF64
        };
    };

    bool parsed;
    QFile elf_file;
    QByteArray b_data;
    classes::CLASS cls;

    offset_t elf_header_idx;

    /// size of ph header structure according to the architecture
    /// number of program header entries
    esize_t ph_size,
            ph_num;

    /// list of ph index headers
    QList<ex_offset_t> ph_idx;

    /**
     * @brief Wypełnia listę z indeksami struktur Program Header.
     * @return True jeżeli wielkość tablicy się zgadza z zadeklarowaną, False w innych przypadkach.
     */
    bool get_ph_addresses();

    /**
     * @brief Pobiera zawartość struktury Elf32_Ehdr.
     * @return Wskaźnik na strukturę Elf32_Ehdr jeżeli dane są poprawne, nullptr w innych przypadkach.
     */
    void* get_elf_header();

    /**
     * @brief Pobiera zawartość struktury Elf32_Phdr.
     * @param idx Indeks
     * @return Wskaźnik na strukturę Elf32_Phdr jeżeli dane są poprawne, nullptr w innych przypadkach.
     */
    void* get_ph_header(uint32_t idx = 0);

    /**
     * @brief Sprawdza czy wartości magiczne w podanej strukturze zgadzają się z ELF.
     * @param elf_hdr wskaźnik na strukturę Elf32_Ehdr.
     * @return True jeżeli wartość magiczna jest poprawna, False w innych przypadkach.
     */
    bool check_magic(const Elf32_Ehdr *elf_hdr) const;

    /**
     * @brief Sprawdza czy architektura, kolejność bajtów są podtrzymywane.
     * @param elf_hdr wskaźnik na strukturę Elf32_Ehdr.
     * @return True jeżeli architektura, kolejność bajtów są podtrzymywane, False w innych przypadkach.
     */
    bool is_supported(const Elf32_Ehdr *elf_hdr);

    /**
     * @brief Pobiera informacje dotyczące offsetu
     * @param elf_hdr
     * @return
     */
    bool get_ph_info(const void *elf_hdr);

    /**
     * @brief Parsowanie danych, znajdujacych sie w pamięci jako pliku ELF.
     * @return True jeżeli dane w pamięci sa zgodne z formatem ELF, False w pozostałych przypadkach.
     */
    bool parse();

public:
    /**
     * @brief Konstruktor.
     * @param _fname nazwa pliku.
     */
    ELF(QString _fname);

    /**
     * @brief Destruktor.
     */
    virtual ~ELF();

    /**
     * @brief Sprawdza czy w pamięci przechowywany jest poprawny plik.
     * @return True jeżeli poprawny, False w innym przypadku.
     */
    bool is_valid() const { return elf_file.isOpen() & parsed; }

    /**
     * @brief Sprawdza czy plik jest otwarty.
     * @return True jeżeli jest otwarty, False jeżeli nie.
     */
    bool is_open() const { return elf_file.isOpen(); }

    /**
     * @brief Pobiera ilość segmentów w pliku.
     * @return Ilość segmentów w pliku, -1 w razie błędu.
     */
    int get_number_of_segments() const { return is_valid() ? ph_num : - 1; }

    /**
     * @brief Pobiera offset w pliku dla podanego segmentu.
     * @param idx indeks segmentu.
     * @return Offset jeżeli dane są poprawne, nullptr w innych przypadkach.
     */
    void* get_ph_seg_offset(uint32_t idx = 0);
};

#endif // ELFFILE_H
