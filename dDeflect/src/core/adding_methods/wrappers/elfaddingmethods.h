#ifndef ELFADDINGMETHODS_H
#define ELFADDINGMETHODS_H

#include <QTemporaryFile>
#include <QFileInfo>

#include <core/adding_methods/wrappers/daddingmethods.h>

template <typename RegistersType>
class ELFAddingMethods : public DAddingMethods<RegistersType> {
public:
    ELFAddingMethods(ELF *f);
    ~ELFAddingMethods();

    /**
     * @brief Metoda zabezpiecza plik binarny ELF za pomocą wyspecyfikowanej metody.
     * @param inject_desc opis metody wstrzykiwania kodu.
     * @return True, jeżeli operacja się powiodła, False w innych przypadkach.
     */
    virtual bool secure(const QList<typename DAddingMethods<RegistersType>::InjectDescription*> &inject_desc) override;

    /**
     * @brief Metoda zabezpiecza plik binarny ELF.
     * @param code_cover procentowy stopień zaśmiecania kodu.
     * @return True, jeżeli operacja się powiodła, False w innych przypadkach.
     */
    bool obfuscate(uint8_t code_cover);

private:
    enum class PlaceholderMnemonics {
        DDETECTIONHANDLER,
        DDETECTIONMETHOD,
        DDRET
    };

    /**
     * @brief Typy placeholderów.
     */
    enum class PlaceholderTypes {
        PARAM_PRE,
        PARAM_POST,
        PLACEHOLDER_PRE,
        PLACEHOLDER_POST
    };

    typedef struct _rel_jmp_info {
        Elf64_Off ndata_off;
        uint32_t ndata_size;
        Elf64_Off fdata_off;
        Elf64_Addr data_vaddr;

        _rel_jmp_info(Elf64_Off _ndata_off, uint32_t _ndata_size,
                     Elf64_Off _fdata_off, Elf64_Addr _data_vaddr) :
                    ndata_off(_ndata_off), ndata_size(_ndata_size),
                    fdata_off(_fdata_off), data_vaddr(_data_vaddr) {}
        _rel_jmp_info() {}
    } rel_jmp_info;

    QMap<PlaceholderTypes, QString> placeholder_id;
    QMap<PlaceholderMnemonics, QString> placeholder_mnm;

    bool secure_one(typename DAddingMethods<RegistersType>::InjectDescription* inject_desc);

    /**
     * @brief Metoda odpowiada za generowanie kodu dla dowolnego opakowania.
     * @param wrap klasa opisująca kawałek kodu do wygenerowania.
     * @param code wygenerowany kod.
     * @return True, jeżeli operacja się powiodła, False w innych przypadkach.
     */
    bool wrapper_gen_code(typename DAddingMethods<RegistersType>::Wrapper *wrap, QString &code);

    /**
     * @brief Metoda odpowiada za wypełnianie parametrów w podanym kodzie.
     * @param code kod.
     * @param params parametry.
     * @return ilośc zamienionych parametrów.
     */
    uint64_t fill_params(QString &code, const QMap<QString, QString> &params);

    /**
     * @brief Metoda odpowiada za wypełnianie parametrów magicznych w podanym kodzie (te które, nie mogą być wyliczone dynamicznie).
     * @param params parametry.
     * @param elf instancja klasy pliku ELF.
     * @return True, jeżeli operacja się powiodła, False w innych przypadkach.
     */
    bool fill_magic_params(QMap<QString, QString> &params, const ELF *elf);

    // void fill_dyn_magic_params();

    /**
     * @brief Metoda odpowiada za wypełnianie placeholdera w podanym kodzie, za pomocą podanego kodu.
     * @param code kod.
     * @param gen_code kod, którym zostanie zamieniony placeholder.
     * @param plc_mnm placeholder.
     * @return ilośc zamienionych parametrów.
     */
    uint64_t fill_placeholders(QString &code, const QString &gen_code, PlaceholderMnemonics plc_mnm);

    /**
     * @brief Metoda odpowiada za kompilację kodu źródłowego assembly.
     * @param code2compile kod, który musi zostać skompilowany.
     * @param compiled_code skompilowany kod.
     * @return True, jeżeli operacja się powiodła, False w innych przypadkach.
     */
    bool compile(const QString &code2compile, QByteArray &compiled_code);

    /**
     * @brief Metoda odpowiada za pobieranie adresów z wyspecyfikowanych danych.
     * @param data tablica z adresami.
     * @param addr_size wielkość adresu w bajtach.
     * @param addr_list lista adresów.
     * @param except_list lista adresów, które nie trzeba dołączać do listy wynikowej.
     * @return True, jeżeli operacja się powiodła, False w innych przypadkach.
     */
    bool get_addresses(const QByteArray &data, uint8_t addr_size, QList<Elf64_Addr> &addr_list,
                       const QList<Elf64_Addr> &except_list);

    /**
     * @brief Metoda odpowiada za generowanie kodu dla funkcji, która zmienia prawa dostępu do strony pamięci.
     * @param vaddr adres wirtualny.
     * @param mem_size wielkość pamięci.
     * @param flags prawa dostępu do pamięci.
     * @param code wygenerowany kod.
     * @return True, jeżeli operacja się powiodła, False w innych przypadkach.
     */
    template <typename AddrT, typename XwordT, typename WordT>
    bool set_prot_flags_gen_code(AddrT vaddr, XwordT mem_size, WordT flags, QString &code);

    /**
     * @brief Metoda odpowiada za pobieranie offsetow instrukcji w pliku.
     * @param opcodes lista instrukcji.
     * @param file_off offset w pliku.
     * @param base_off wartość bazowa offsetu.
     */
    void get_file_offsets_from_opcodes(QStringList &opcodes, QList<Elf64_Addr> &file_off, Elf64_Addr base_off);

    bool get_address_offsets_from_text_section(QList<Elf64_Addr> &__file_off, Elf64_Addr &base_off,
                                               QPair<QByteArray, Elf64_Addr> &text_data);

    /**
     * @brief Metoda zabezpiecza plik binarny ELF.
     * @param code_cover procentowy stopień zaśmiecania kodu.
     * @return True, jeżeli operacja się powiodła, False w innych przypadkach.
     */
    bool safe_obfuscate(uint8_t code_cover);
};

#endif // ELFADDINGMETHODS_H
