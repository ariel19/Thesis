#ifndef ELFFILE_H
#define ELFFILE_H

// #include <elf.h>
#include "elf.h"
#include <QFile>
#include <QString>

typedef uint32_t offset_t;

class ELF {
    bool parsed;
    QFile elf_file;
    QByteArray b_data;

    offset_t elf_header_idx;

    /**
     * @brief Pobiera zawartość struktury Elf32_Ehdr.
     * @return Wskaźnik na strukturę Elf32_Ehdr jeżeli dane są poprawne, nullptr w innych przypadkach.
     */
    Elf32_Ehdr* getElfHeader();

    /**
     * @brief Sprawdza czy wartości magiczne w podanej strukturze zgadzają się z ELF.
     * @param elf_hdr skaźnik na strukturę Elf32_Ehdr.
     * @return True jeżeli wartość magiczna jest poprawna, False w innych przypadkach.
     */
    bool check_magic(const Elf32_Ehdr *elf_hdr) const;

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
    bool isValid() const { return elf_file.isOpen() & parsed; }

    /**
     * @brief Sprawdza czy plik jest otwarty.
     * @return True jeżeli jest otwarty, False jeżeli nie.
     */
    bool isOpen() const { return elf_file.isOpen(); }
};

#endif // ELFFILE_H
