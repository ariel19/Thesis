#ifndef DADDINGMETHODS_H
#define DADDINGMETHODS_H

#include <QList>
#include <QMap>
#include <QString>

#include <core/file_types/elffile.h>

class DAddingMethods {
public:
    /**
     * @brief Typy możliwości wstrzyknięcia kodu.
     */
    enum class CallingMethod {
        OEP,
        Thread,
        Trampoline
    };

    /**
     * @brief Rejestry dla architektury x86.
     */
    enum class Registers_x86 {
        EAX,
        EBX,
        ECX,
        EDX,
        ESI,
        EDI,
        EBP,
        ESP
    };

    /**
     * @brief Rejestry dla architektury x64.
     */
    enum class Registers_x64 {
        RAX,
        RBX,
        RCX,
        RDX,
        RSI,
        RDI,
        RBP,
        RSP,
        R8,
        R9,
        R10,
        R11,
        R12,
        R13,
        R14,
        R15
    };

    /**
     * @brief Klasa bazowa reprezentująca opakowanie dla kawałków kodu.
     */
    template <typename RegistersType>
    class Wrapper {
        QList<RegistersType> used_regs;
        QMap<QString, QString> params;
        RegistersType ret;
        QString code;
        Wrapper<RegistersType> *detect_handler;
    };

    /**
     * @brief Klasa reprezentująca opakowanie dla tworzenia nowego wątku.
     */
    template <typename RegistersType>
    class ThreadWrapper : public Wrapper<RegistersType> {
        Wrapper<RegistersType> *thread_action;
    };

    /**
     * @brief Klasa reprezentująca opakowanie dla tworzenia nowego punktu wejściowego.
     */
    template <typename RegistersType>
    class OEPWrapper : public Wrapper<RegistersType> {
        Wrapper<RegistersType> *oep_action;
    };

    /**
     * @brief Klasa reprezentująca opakowanie dla tworzenia tramplin w funkcjach bibliotecznych.
     */
    template <typename RegistersType>
    class TrampolineWrapper : public Wrapper<RegistersType> {

    };

    /**
     * @brief Klasa opisująca metodę wstrzykiwania kodu.
     */
    template <typename RegistersType>
    class InjectDescription {
        CallingMethod cm;
        Wrapper<RegistersType> *adding_method;
    };

    /**
     * @brief Konstruktor.
     */
    DAddingMethods();

    /**
     * @brief Metoda zabezpiecza plik, podany jako argument za pomocą wyspecyfikowanej metody.
     * @param elf plik do zabezpieczania.
     * @param inject_desc opis metody wstrzykiwania kodu.
     * @return True, jeżeli operacja się powiodła, False w innych przypadkach.
     */
    template <typename RegistersType>
    bool secure_elf(ELF &elf, const InjectDescription<RegistersType> &inject_desc);

private:
    enum class PlaceholderMnemonics {
        DDETECTIONHANDLER,
        DDETECTIONMETHOD
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

    QMap<PlaceholderTypes, QString> placeholder_id;
    QMap<PlaceholderMnemonics, QString> placeholder_mnm;

    /**
     * @brief Metoda odpowiada za generowanie kodu dla dowolnego opakowania.
     * @param wrap klasa opisująca kawałek kodu do wygenerowania.
     * @param code wygenerowany kod.
     * @return True, jeżeli operacja się powiodła, False w innych przypadkach.
     */
    template <typename RegistersType>
    bool wrapper_gen_code(Wrapper<RegistersType> *wrap, QString &code);

    /**
     * @brief Metoda odpowiada za wypełnianie parametrów w podanym kodzie.
     * @param code kod.
     * @param params parametry.
     * @return ilośc zamienionych parametrów.
     */
    uint64_t fill_params(QString &code, const QMap<QString, QString> &params);

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
};

#endif // DADDINGMETHODS_H
