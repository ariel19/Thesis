#ifndef ELFFILE_H
#define ELFFILE_H

#ifdef __linux__
#include <elf.h>
#else
#include <core/sys_headers/elf.h>
#endif
#include <QFile>
#include <QString>
#include <QList>
#include <QMap>
#include <map>
#include <core/file_types/binaryfile.h>

typedef uint32_t offset_t;
typedef Elf64_Half esize_t;
typedef Elf64_Off ex_offset_t;

class ELF : public BinaryFile {
public:
    /**
     * @brief Type sekcji.
     */
    enum class SectionType {
        INIT,
        CTORS,
        INIT_ARRAY,
        TEXT
    };

    /**
     * @brief Konstruktor.
     * @param _fname nazwa pliku.
     */
    ELF(QByteArray _data);

    /**
     * @brief Destruktor.
     */
    virtual ~ELF();

    /**
     * @brief Pobiera zawartość pliku.
     * @return Zawartość aktualnie analizowalnego pliku.
     */
    const QByteArray& get_elf_content() const { return b_data; }

    /**
     * @brief Sprawdza czy w pamięci przechowywany jest poprawny plik.
     * @return True jeżeli poprawny, False w innym przypadku.
     */
    bool is_valid() const { return parsed; }

    /**
     * @brief Dostarcza informacje czy plik jest poprawnym plikiem ELF 32-bitowym.
     * @return True jezeli spełnia warunki, False w pozostałych przypadkach.
     */
    bool is_x86() const { return is_valid() & (cls == classes::ELF32); }

    /**
     * @brief Dostarcza informacje czy plik jest poprawnym plikiem ELF 64-bitowym.
     * @return True jezeli spełnia warunki, False w pozostałych przypadkach.
     */
    bool is_x64() const { return is_valid() & (cls == classes::ELF64); }

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

    /**
     * @brief Rozszerza najbardziej pasujący segment LOAD i kopiuje do niego podany kod.
     * @param data dane, które chcemy skopiować w miejsce rozszerzonego segmentu.
     * @param only_x flaga, która odpowiada za rozszerzanie tylko wykonywalnych sekcji.
     * @param va nowy adres wirtualny w rozszerzonym segmencie.
     * @return Reprezentacjz binarna nowego pliku, długość danych równa się 0 jeżeli operacja nie powiodła się.
     */
    QByteArray extend_segment(const QByteArray &data, bool only_x, Elf64_Addr &va);

    /**
     * @brief Zapisuje podane dane do określonego pliku.
     * @param fname nazwa pliku.
     * @param data zapisywane dane.
     * @return True, jeżeli operacja zapisu się powiodła, False w innych przypadkach.
     */
    bool write_to_file(const QString &fname, const QByteArray &data) const;

    /**
     * @brief Zapisuje wewnętrzne dane do pliku.
     * @param fname nazwa pliku.
     * @return True, jeżeli operacja zapisu się powiodła, False w innych przypadkach.
     */
    bool write_to_file(const QString &fname) const;

    /**
     * @brief Ustawia punkt wejściowy dla pliku wykonywalnego.
     * @param entry_point wartość punktu wejściowego.
     * @param data dane, w których należy ustawić punkt wejściowy.
     * @param old_ep wartosc starego punkt wejsciowego, parametr opcjonalny.
     * @return True jeżeli operacja się powiodła, False w innych przypadkach.
     */
    bool set_entry_point(const Elf64_Addr &entry_point, QByteArray &data, Elf64_Addr *old_ep = nullptr);

    /**
     * @brief Ustawia punkt wejściowy dla pliku wykonywalnego.
     * @param entry_point wartość punktu wejściowego.
     * @param old_ep wartosc starego punkt wejsciowego, parametr opcjonalny.
     * @return True jeżeli operacja się powiodła, False w innych przypadkach.
     */
    bool set_entry_point(const Elf64_Addr &entry_point, Elf64_Addr *old_ep = nullptr);

    /**
     * @brief Dostarcza informacje o punkcie wejściowym pliku podanego jako parameter.
     * @param data zawartosc pliku ELF.
     * @param old_ep referencja na wartość punktu wejściowego programu.
     * @return True jeżeli operacja się powiodła, False w innych przypadkach.
     */
    bool get_entry_point(const QByteArray &data, Elf64_Addr &old_ep) const;

    /**
     * @brief Dostarcza informacje o punkcie wejściowym pliku.
     * @param old_ep referencja na wartość punktu wejściowego programu.
     * @return True jeżeli operacja się powiodła, False w innych przypadkach.
     */
    bool get_entry_point(Elf64_Addr &old_ep) const;

    /**
     * @brief Pobiera zawartość sekcji, jeżeli podana sekcja istnieje.
     * @param data zawartość pliku ELF.
     * @param sec_type typ sekcji.
     * @param section_data zawartość sekcji oraz adres witualny.
     * @return True jeżeli sekcja istnieje, False w innych przypadkach.
     */
    bool get_section_content(const QByteArray &data, SectionType sec_type, QPair<QByteArray, Elf64_Addr> &section_data);

    /**
     * @brief Zamienia zawartość sekcji nowymi danymi, jeżeli podana sekcja istnieje.
     * @param data zawartość pliku ELF.
     * @param sec_type typ sekcji.
     * @param section_data zawartość sekcji.
     * @param filler bajt, którym jest dopełniana sekcja.
     * @return True jeżeli sekcja istnieje, False w innych przypadkach.
     */
    bool set_section_content(QByteArray &data, SectionType sec_type, const QByteArray &section_data, const char filler = '\x00');

    /**
     * @brief Pobiera flagi ochrony pamięci dla segmentu, który ładuje się pod podanym adresem wirtualnym.
     * @param vaddr adres wirtualny pod który ładuje segment.
     * @param prot_flags flagi ochrony pamięci.
     * @return True jeżeli segment istnieje, False w innych przypadkach.
     */
    bool get_segment_prot_flags(const Elf64_Addr vaddr, int &prot_flags) const;

    /**
     * @brief Pobiera wartość wyrównania segmentu.
     * @param vaddr adres wirtualny pod który ładuje segment.
     * @param align wartość wyrównywania strony.
     * @return True jeżeli segment istnieje, False w innych przypadkach.
     */
    bool get_segment_align(const Elf64_Addr vaddr, Elf64_Addr &align) const;

    /**
     * @brief Pobiera zawartość pierwszego segmenu LOAD, do którego pasują podane flagi ochrony pamięci.
     * @param prot_flags flagi ochrony pamięci.
     * @param segment_data zawartość segmentu oraz adres wirtualny.
     * @return True jeżeli segment istnieje, False w innych przypadkach.
     */
    bool get_load_segment_info(int prot_flags, QPair<QByteArray, Elf64_Addr> &segment_data) const;

private:
    typedef struct _section_info {
        QString    sh_name;
        uint32_t   sh_type;
        _section_info() {}
        _section_info(const QString &name, const uint32_t type) :
            sh_name(name), sh_type(type) {}
    } section_info;

    static QMap<SectionType, section_info> section_type;

    typedef struct _best_segment {
        uint32_t post_pad,
                 pre_pad;
        void *ph;
        bool change_vma;

    public:
        _best_segment() :
            post_pad(0), pre_pad(0),
            ph(nullptr), change_vma(false) {}
    } best_segment;

    struct classes {
        enum CLASS {
            NONE,
            ELF32,
            ELF64
        };
    };

    bool parsed;
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
     * @brief Uzupełnia informacje, dotyczące najlepszego segmentu na podstawie podanych argumentów.
     * @param bs struktura, przedstawiająca informacje o segmencie.
     * @param only_x flaga, która odpowiada za rozszerzanie tylko wykonywalnych sekcji.
     * @param ph wskaźnik na strukture, reprezentującą Elf_Phdr.
     * @param pad_post ilość bajtów potrzebnych do wypełnanie przed dodawanymi danymi.
     * @param pad_pre ilość bajtów potrzebnych do wypełnanie po dodawanych danych.
     * @param change_va informacja czy musi zostać zmieniony adres wirtualy.
     */
    template <typename ElfProgramHeader>
    void __best_segment_choose(best_segment &bs, bool only_x, ElfProgramHeader *ph,
                               uint32_t pad_post, uint32_t pad_pre, bool change_va);

    /**
     * @brief Sprawdza czy aktualnie przetwarzany segment da się rozszerzyć.
     * @param bs struktura, przedstawiająca informacje o segmencie.
     * @param only_x flaga, która odpowiada za rozszerzanie tylko wykonywalnych sekcji.
     * @param load_seg referencja na listę ładowalnych (LOAD) segmentów pliku.
     * @param i aktualnie przetwarzany segment.
     * @param data_size wielkość wstawianych danych.
     * @return True jeżeli rozszerzenie jest możliwe, False w innych przypadkach.
     */
    template <typename ElfProgramHeaderType, typename ElfOffsetType>
    bool __extend_segment_eligible(best_segment &bs, bool only_x, const QList<std::pair<esize_t, void*> > &load_seg,
                                   int i, const int data_size);
    /**
     * @brief Wypełnia listę z indeksami struktur Program Header.
     * @return True jeżeli wielkość tablicy się zgadza z zadeklarowaną, False w innych przypadkach.
     */
    bool __get_ph_addresses();

    /**
     * @brief Pobiera zawartość struktury Elf32_Ehdr.
     * @return Wskaźnik na strukturę Elf32_Ehdr jeżeli dane są poprawne, nullptr w innych przypadkach.
     */
    void* __get_elf_header();

    /**
     * @brief Pobiera zawartość struktury Elf32_Phdr.
     * @param idx Indeks
     * @return Wskaźnik na strukturę Elf32_Phdr jeżeli dane są poprawne, nullptr w innych przypadkach.
     */
    void* __get_ph_header(uint32_t idx = 0);

    /**
     * @brief Sprawdza czy wartości magiczne w podanej strukturze zgadzają się z ELF.
     * @param elf_hdr wskaźnik na strukturę Elf32_Ehdr.
     * @return True jeżeli wartość magiczna jest poprawna, False w innych przypadkach.
     */
    bool __check_magic(const Elf32_Ehdr *elf_hdr) const;

    /**
     * @brief Sprawdza czy architektura, kolejność bajtów są podtrzymywane.
     * @param elf_hdr wskaźnik na strukturę Elf32_Ehdr.
     * @return True jeżeli architektura, kolejność bajtów są podtrzymywane, False w innych przypadkach.
     */
    bool __is_supported(const Elf32_Ehdr *elf_hdr);

    /**
     * @brief Pobiera informacje dotyczące pierwszego offsetu w pliku.
     * @param elf_hdr wskaźnik na strukturę, przechowującą informacje.
     * @return True jeżeli dane wejściowe są poprawne, False w innych przypadkach.
     */
    template <typename ElfHeaderType>
    bool __get_ph_address(const void *elf_hdr);

    /**
     * @brief Pobiera informacje dotyczące offsetu.
     * @param elf_hdr wskaźnik na strukturę, przechowującą informacje.
     * @return True jeżeli dane wejściowe są poprawne, False w innych przypadkach.
     */
    bool __get_ph_info(const void *elf_hdr);

    /**
     * @brief Parsowanie danych, znajdujących się w pamięci jako pliku ELF.
     * @return True jeżeli dane w pamięci sa zgodne z formatem ELF, False w pozostałych przypadkach.
     */
    bool __parse();

    /**
     * @brief Zaokrągla w dół adres, podany jako argument zgodnie z wyspecyfikowanym wyrównaniem.
     * @param addr adres do zaokrąglenia.
     * @param align wartość wyrównania.
     * @return Zaokrąglony adres.
     */
    Elf64_Xword __round_address_down(ex_offset_t addr, ex_offset_t align) const;

    /**
     * @brief Znajduje ilość bajtów, którymi musimy dopełnić nasze dane z przodu.
     * @param ph wskaźnik na strukture 32/64-bitowego nagłówka ELF.
     * @param phn wskaźnik na strukture 32/64-bitowego nagłówka ELF.
     * @param dsize wielkość danych.
     * @param pre_pad adres komórki pamięci, pod którą zapiszemy wartość.
     * @return True jeżeli operacja się powidła, False w innych przypadkach.
     */
    template <typename ElfProgramHeaderType, typename ElfOffsetType>
    bool __find_pre_pad(const ElfProgramHeaderType *ph, const ElfProgramHeaderType *phn,
                        const int dsize, uint32_t *pre_pad);

    /**
     * @brief Znajduje ilość bajtów, którymi musimy dopełnić nasze dane z tyłu.
     * @param ph wskaźnik na strukture 32/64-bitowego nagłówka ELF.
     * @param phn wskaźnik na strukture 32/64-bitowego nagłówka ELF.
     * @param dsize wielkość danych.
     * @param pre_pad wartość dopełnenia przed danymi.
     * @param post_pad adres komórki pamięci, pod którą zapiszemy wartość.
     * @param change_vma adres komórki pamięci pod którą zapiszemy potrzebe zmiany VMA.
     * @return True jeżeli operacja się powidła, False w innych przypadkach.
     */
    template <typename ElfProgramHeaderType, typename ElfOffsetType>
    bool __find_post_pad(const ElfProgramHeaderType *ph, const ElfProgramHeaderType *phn,
                         const int dsize, const uint32_t pre_pad,
                         uint32_t *post_pad, bool *change_vma);

    /**
     * @brief Wylicza offset w pliku oraz adres wirtualny dla nowych danych, dodawanych do pliku.
     * @param bs referencja na strukture, ktora zawiera informacje o najlepszym znalezionym segmencie.
     * @return Para nowy offset w pliku oraz nowy adres wirtualny.
     */
    template <typename ElfProgramHeaderType>
    std::pair<ex_offset_t, ex_offset_t> __get_new_data_va_fo(ELF::best_segment &bs);

    /**
     * @brief Tworzy nową zawartość pliku wynikowego po dodaniu nowego kodu na podstawie instancji struktury best_segment.
     * @param data nowy kod.
     * @param bs struktura przechowujące informacje o wyrównaniach oraz adresach.
     * @return Nowa zawartość pliku (oraz adres wirtualny nowych danych) lub pustą tablice, jeżeli operacja nie powiadła się.
     */
    std::pair<QByteArray, Elf64_Addr> __construct_data(const QByteArray &data, best_segment &bs);

    /**
     * @brief Naprawia nagłówek pliku ELF.
     * @param data zawartość pliku.
     */
    template <typename ElfHeaderType>
    void __fix_elf_header(QByteArray &data, ex_offset_t file_off, uint32_t insert_space);

    /**
     * @brief Naprawia tablicę sekcji.
     * @param data zawartość pliku.
     */
    template <typename ElfHeaderType, typename ElfSectionHeaderType>
    void __fix_section_table(QByteArray &data, const ex_offset_t file_off, const uint32_t insert_space);

    /**
     * @brief Naprawia tablicę segmentów.
     * @param data zawartość pliku.
     */
    template <typename ElfProgramHeaderType>
    Elf64_Addr __fix_segment_table(QByteArray &data, const ex_offset_t file_off,
                                   const uint32_t insert_space, const uint32_t payload_size);

    /**
     * @brief Naprawia VMA.
     * @param data zawartość pliku.
     */
    template <typename ElfDynType, typename ElfSymType, typename ElfWordType>
    void __fix_vma(QByteArray &data, const best_segment &bs, ex_offset_t file_off, const Elf64_Addr &new_vma);

    /**
     * @brief Pobiera zawartość sekcji, jeżeli podana sekcja istnieje.
     * @param data zawartość pliku ELF.
     * @param sec_type typ sekcji.
     * @param section_data zawartość sekcji oraz offset w pliku.
     * @return True jeżeli sekcja istnieje, False w innych przypadkach.
     */
    template <typename ElfHeaderType, typename ElfSectionHeaderType>
    bool __get_section_content(const QByteArray &data, SectionType sec_type, QPair<QByteArray, Elf64_Addr> &section_data);

    /**
     * @brief Zamienia zawartość sekcji nowymi danymi, jeżeli podana sekcja istnieje.
     * @param data zawartość pliku ELF.
     * @param sec_type typ sekcji.
     * @param section_data zawartość sekcji.
     * @param filler bajt, którym jest dopełniana sekcja.
     * @return True jeżeli sekcja istnieje, False w innych przypadkach.
     */
    template <typename ElfHeaderType, typename ElfSectionHeaderType>
    bool __set_section_content(QByteArray &data, SectionType sec_type, const QByteArray &section_data, const char filler = '\x00');

    /**
     * @brief Pobiera flagi ochrony pamięci dla segmentu, który ładuje się pod podanym adresem wirtualnym.
     * @param vaddr adres wirtualny pod który ładuje segment.
     * @param prot_flags flagi ochrony pamięci.
     * @return True jeżeli segment istnieje, False w innych przypadkach.
     */
    template <typename ElfProgramHeaderType>
    bool __get_segment_prot_flags(const Elf64_Addr vaddr, int &prot_flags) const;

    /**
     * @brief Pobiera wartość wyrównania segmentu.
     * @param vaddr adres wirtualny pod który ładuje segment.
     * @param align wartość wyrównywania strony.
     * @return True jeżeli segment istnieje, False w innych przypadkach.
     */
    template <typename ElfProgramHeaderType>
    bool __get_segment_align(const Elf64_Addr vaddr, Elf64_Addr &align) const;

    /**
     * @brief Pobiera zawartość pierwszego segmenu LOAD, do którego pasują podane flagi ochrony pamięci.
     * @param prot_flags flagi ochrony pamięci.
     * @param segment_data zawartość segmentu oraz adres wirtualny.
     * @return True jeżeli segment istnieje, False w innych przypadkach.
     */
    template <typename ElfProgramHeaderType>
    bool __get_load_segment_info(int prot_flags, QPair<QByteArray, Elf64_Addr> &segment_data) const;
};

#endif // ELFFILE_H
