#ifndef PECODEDEFINES_H
#define PECODEDEFINES_H

#include <QByteArray>
#include <QList>
#include <QMap>
#include <QRegExp>
#include <QStack>

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
    ESP,
    None,
    All
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
    R15,
    None,
    All
};


/**
 * @brief Klasa przechowująca kod binarny i offsety relokacji adresów
 */
template <typename Register>
class BinaryCode
{
private:
    /**
     * @brief Kod binarny
     */
    QByteArray code;

    /**
     * @brief Lista offsetów relokacji
     */
    QList<uint64_t> relocations;

    /**
     * @brief Rozmiar adresu
     */
    static const uint8_t addrSize;

public:
    /**
     * @brief BinaryCode - Konstruktor
     */
    BinaryCode() { }

    /**
     * @brief Dodaje kod do tablicy
     * @param _code Kod binarny
     * @param relocation Flaga odpowiadająca za dodanie informacji o relokacji w miejscu dodanego kodu.
     */
    void append(QByteArray _code, bool relocation = false);

    /**
     * @brief Pobiera kod
     * @return Kod binarny
     */
    QByteArray getBytes();

    /**
     * @brief Pobiera tablicę relokacji jako wartości absolutne
     * @param codeBase Adres bazowy relokowanego kodu
     * @return Lista relokacji
     */
    QList<uint64_t> getRelocations(uint64_t codeBase);

    /**
     * @brief Metoda pobierająca wielkość kodu
     * @return Rozmiar kodu
     */
    int length();
};


/**
 * @brief Klasa definiująca sposób generowania kodu
 */
template <typename Register>
class CodeDefines
{
private:

    /**
     * @brief Mapa kodów odpowiadających instrukcjom: push reg
     */
    static const QMap<Register, QByteArray> _save_reg;

    /**
     * @brief Mapa kodów odpowiadających instrukcjom: pop reg
     */
    static const QMap<Register, QByteArray> _restore_reg;

    /**
     * @brief Mapa kodów odpowiadających instrukcjom: mov reg, value
     */
    static const QMap<Register, QByteArray> _mov_to_reg;

    /**
     * @brief Mapa kodów odpowiadających instrukcjom: call reg
     */
    static const QMap<Register, QByteArray> _call_reg;

    /**
     * @brief Mapa kodów odpowiadających instrukcjom: test reg, reg
     */
    static const QMap<Register, QByteArray> _test_reg;

    /**
     * @brief Mapa kodów odpowiadających instrukcjom: mov reg, [esp + x]
     */
    static const QMap<Register, QByteArray> _esp_mem_to_reg;

    /**
     * @brief Mapa kodów odpowiadających instrukcjom: mov [esp + x], reg
     */
    static const QMap<Register, QByteArray> _reg_to_esp_mem;

    /**
     * @brief Mapa kodów odpowiadających instrukcjom: jmp reg
     */
    static const QMap<Register, QByteArray> _jmp_reg;


    /**
     * @brief Kod odpowiadający instrukcji: jz offset
     */
    static const QByteArray _jz_rel;

    /**
     * @brief Kod odpowiadający instrukcji: jmp offset
     */
    static const QByteArray _jmp_rel;

    /**
     * @brief Kod odpowiadający instrukcji: sub esp, x
     */
    static const QByteArray _reserve_stack;

    /**
     * @brief Kod odpowiadający instrukcji: add esp, x
     */
    static const QByteArray _clear_stack;

    /**
     * @brief Kod odpowiadający instrukcji: push x
     */
    static const QByteArray _store_value;

    /**
     * @brief Kod odpowiadający instrukcji: ret n
     */
    static const QByteArray _ret_n;

    /**
     * @brief Kod odpowiadający instrukcji: pushad
     */
    static const QByteArray _pushad;

    /**
     * @brief Kod odpowiadający instrukcji: popad
     */
    static const QByteArray _popad;

    /**
     * @brief Kod odpowiadający instrukcji: mov [esp + 4], x
     */
    static const QByteArray _store_high_bytes;

    /**
     * @brief Stos seedów generatora liczb losowych
     */
    static QStack<uint64_t> seed;

public:

    /**
     * @brief Kod odpowiadający instrukcji: push ebp; mov ebp, esp;
     */
    static const QByteArray startFunc;

    /**
     * @brief Kod odpowiadający instrukcji: pop ebp
     */
    static const QByteArray endFunc;

    /**
     * @brief Kod odpowiadający instrukcji: ret
     */
    static const QByteArray ret;

    /**
     * @brief Lista wewnętrznych rejestów
     */
    static const QList<Register> internalRegs;

    /**
     * @brief Lista zewnętrznych rejestrów
     */
    static const QList<Register> externalRegs;

    /**
     * @brief Rozmiar Shadow Space
     */
    static const uint8_t shadowSize;

    /**
     * @brief Liczba bajtów potrzebna do wyrównania stosu do 16
     */
    static const uint8_t align16Size;

    /**
     * @brief Rozmiar komórki stosu
     */
    static const uint8_t stackCellSize;

    /**
     * @brief Wyrażenie regularne znajdujące znak nowej linii
     */
    static const QRegExp newLineRegExp;

    /**
     * @brief Wyrażenie regularne znajdujące instrukcję call offset w zdekompilowanym kodzie
     */
    static const QRegExp callRegExp;

    /**
     * @brief Wyrażenie regularne znajdujące instrukcję jmp offset w zdekompilowanym kodzie
     */
    static const QRegExp jmpRegExp;

    /**
     * @brief Metoda odpowiadająca instrukcji: push reg
     * @param reg Rejestr do zapisania
     * @return Kod
     */
    static QByteArray saveRegister(Register reg);

    /**
     * @brief Metoda odpowiadająca instrukcji: pop reg
     * @param reg Rejestr do odczytania
     * @return Kod
     */
    static QByteArray restoreRegister(Register reg);

    /**
     * @brief Metoda odpowiadająca instrukcji: mov reg, value
     * @param value Liczba do zapisania w rejestrze
     * @param reg Rejestr
     * @return Kod
     */
    template <typename T>
    static QByteArray movValueToReg(T value, Register reg);

    /**
     * @brief Metoda odpowiadająca instrukcji: call reg
     * @param reg Rejestr
     * @return Kod
     */
    static QByteArray callReg(Register reg);

    /**
     * @brief Metoda odpowiadająca instrukcji: call offset
     * @param pos Offset
     * @return Kod
     */
    static QByteArray callRelative(uint32_t pos);

    /**
     * @brief Metoda odpowiadająca instrukcji: jmp reg
     * @param reg Rejestr do skoku
     * @return Kod
     */
    static QByteArray jmpReg(Register reg);

    /**
     * @brief Metoda odpowiadająca instrukcji: test reg, reg
     * @param reg Rejestr
     * @return Kod
     */
    static QByteArray testReg(Register reg);

    /**
     * @brief Metoda odpowiadająca instrukcji: jz offset
     * @param pos Offset
     * @return Kod
     */
    static QByteArray jzRelative(int8_t pos);

    /**
     * @brief Metoda odpowiadająca instrukcji: jmp offset
     * @param pos Offset
     * @return Kod
     */
    static QByteArray jmpRelative(int8_t pos);

    /**
     * @brief Metoda odpowiadająca instrukcjom zapisania wszystkich wewnętrznych rejestrów na stos
     * @return Kod
     */
    static QByteArray saveAllInternal();

    /**
     * @brief Metoda odpowiadająca instrukcjom odczytu wszystkich wewnętrznych rejestrów ze stosu
     * @return Kod
     */
    static QByteArray restoreAllInternal();

    /**
     * @brief Metoda odpowiadająca instrukcji: sub esp, noParams * 4 / sub rsp, npParams * 8
     * @param noParams Liczba komórek do zarezerwowania
     * @return Kod
     */
    static QByteArray reserveStackSpace(uint16_t noParams);

    /**
     * @brief Metoda odpowiadająca instrukcji: add esp, noParams * 4 / add rsp, npParams * 8
     * @param noParams Liczba komórek do zwolnienia
     * @return Kod
     */
    static QByteArray clearStackSpace(uint16_t noParams);

    /**
     * @brief Metoda odpowiadająca instrukcji: push value
     * @param dword Wartość do zapisania
     * @return Kod
     */
    template <typename T>
    static QByteArray storeValue(T dword);

    /**
     * @brief Metoda odpowiadająca instrukcji: mov reg, [esp + base]
     * @param reg Rejestr
     * @param base Wartość przesunięcia
     * @return Kod
     */
    static QByteArray readFromEspMemToReg(Register reg, int8_t base);

    /**
     * @brief Metoda odpowiadająca instrukcji: mov [esp + base], reg
     * @param reg Rejestr
     * @param base Przesunięcie
     * @return Kod
     */
    static QByteArray readFromRegToEspMem(Register reg, int8_t base);

    /**
     * @brief Metoda odpowiadająca instrukcji: ret n
     * @param n Liczba bajtów do zwolnienia
     * @return Kod
     */
    static QByteArray retN(uint16_t n);

    /**
     * @brief Zapisanie wszystkich rejestów na stosie
     * @return Kod
     */
    static QByteArray saveAll();

    /**
     * @brief Odczyt wszystkich rejestrów ze stosu
     * @return Kod
     */
    static QByteArray restoreAll();

    /**
     * @brief Kod zaciemniający działanie
     * @param gen Generator liczb losowych
     * @param min_len Minimalna długość losowych danych
     * @param max_len Maksymalna długość losowych danych
     * @return Kod
     */
    static QByteArray obfuscate(std::default_random_engine &gen, uint8_t min_len, uint8_t max_len);

    /**
     * @brief Generwanie kodu źródłowego języka assembly, zaciemniającego działanie programu.
     * @param gen Generator liczb losowych.
     * @param min_len Minimalna długość losowych danych.
     * @param max_len Maksymalna długość losowych danych.
     * @return Kod.
     */
    static QString obfuscate_source(std::default_random_engine &gen, uint8_t min_len, uint8_t max_len);
};

#endif // PECODEDEFINES_H
