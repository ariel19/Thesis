#ifndef PEADDINGMETHODS_H
#define PEADDINGMETHODS_H

#include <QProcess>

#include <core/adding_methods/wrappers/daddingmethods.h>
#include <core/file_types/pefile.h>

/**
 * @brief Klasa odpowiedzialna za dodawanie metod zabezpieczających do plików PE
 */
class PEAddingMethods : public DAddingMethods
{
private:
    /**
     * @brief Generator liczb losowych
     */
    std::default_random_engine gen;


    /**
     * @brief Metoda generująca kod ładujący parametry dla metod.
     * @param pe Plik PE
     * @param code Wygenerowany kod
     * @param getFunctionsCodeAddr Adres metody uzyskującej dostęp do API Windows
     * @param params Parametry metody
     * @param ptrs Mapa z adresami wcześniej wklejonych kawałków kodu
     * @param threadCodePtr Adres wklejonego kodu stworzenia wątku
     * @return True w przypadku sukcesu
     */
    template <typename Register, typename T>
    bool generateParametersLoadingCode(PEFile &pe, BinaryCode<Register> &code, T getFunctionsCodeAddr, QMap<Register,
                                       QString> params, QMap<QByteArray, uint64_t> &ptrs, T threadCodePtr);

    /**
     * @brief Metoda generująca kod sprawdzający warunek wywołania akcji (handlera) dla metody
     * @param pe Plik PE
     * @param code Wygenerowany kod
     * @param action Adres metody, która ma być wywołana jako akcja
     * @param cond Rejestr, w którym jest zwracana z metody flaga warunku
     * @param act Rejestr w którym znajdować będzie się adres akcji
     * @return True w przypadku powodzenia
     */
    template <typename Register>
    bool generateActionConditionCode(PEFile &pe, BinaryCode<Register> &code, uint64_t action, Register cond, Register act);

    /**
     * @brief Metoda dodająca kod w EntryPoint
     * @param pe Plik PE
     * @param epMethods Wybrane metody zabezpieczania
     * @param codePointers Mapa z adresami wcześniej wklejonych kawałków kodu
     * @param relocations Tablica relokacji do wypełnienia
     * @return True w przypadku sukcesu
     */
    template <typename Register>
    bool injectEpCode(PEFile &pe, QList<uint64_t> &epMethods, QMap<QByteArray, uint64_t> &codePointers, QList<uint64_t> &relocations);

    /**
     * @brief Metoda dodająca kod do Thread Local Storage
     * @param pe Plik PE
     * @param tlsMethods Wybrane metody zabezpieczania
     * @param codePointers Mapa z adresami wcześniej wklejonych kawałków kodu
     * @param relocations Tablica relokacji do wypełnienia
     * @return True w przypadku sukcesu
     */
    template <typename Register>
    bool injectTlsCode(PEFile &pe, QList<uint64_t> &tlsMethods, QMap<QByteArray, uint64_t> &codePointers, QList<uint64_t> &relocations);

    /**
     * @brief Metoda dodająca kod jako trampolinę
     * @param pe Plik PE
     * @param tramMethods Wybrane metody zabezpieczania
     * @param codePointers Mapa z adresami wcześniej wklejonych kawałków kodu
     * @param relocations Tablica relokacji do wypełnienia
     * @param text_section Zawartość sekcji .text
     * @param text_section_offset Offset sekcji .text w pliku
     * @param codeCoverage Procentowe pokrycie kodu
     * @return True w przypadku sukcesu
     */
    template <typename Register>
    bool injectTrampolineCode(PEFile &pe, QList<uint64_t> &tramMethods, QMap<QByteArray, uint64_t> &codePointers,
                              QList<uint64_t> &relocations, QByteArray text_section, uint32_t text_section_offset, uint8_t codeCoverage);

    /**
     * @brief Metoda generująca kod wątku
     * @param pe Plik PE
     * @param wrappers Metody do uruchomienia w wątku
     * @param ptrs Mapa z adresami wcześniej wklejonych kawałków kodu
     * @param sleepTime Czas snu wątku
     * @param relocations Tablica relokacji do wypełnienia
     * @return True w przypadku sukcesu
     */
    template <typename Register>
    uint64_t generateThreadCode(PEFile &pe, QList<Wrapper<Register>*> wrappers, QMap<QByteArray, uint64_t> &ptrs, uint16_t sleepTime, QList<uint64_t> &relocations);

    /**
     * @brief Generowanie kodu metody
     * @param pe Plik PE
     * @param w Opis metody
     * @param ptrs Mapa z adresami wcześniej wklejonych kawałków kodu
     * @param relocations Tablica relokacji do wypełnienia
     * @param isTlsCallback Informacja czy metoda jest callbackiem TLS
     * @return True w przypadku sukcesu
     */
    template <typename Register>
    uint64_t generateCode(PEFile &pe, Wrapper<Register> *w, QMap<QByteArray, uint64_t> &ptrs, QList<uint64_t> &relocations, bool isTlsCallback = false);

    /**
     * @brief Metoda tworząca listę offsetów instrukcji na podstawie zdekompilowanego kodu
     * @param opcodes Linie zdekompilowanego kodu
     * @param fileOffsets Offsety
     * @param baseOffset Adres bazowy zdekompilownego fragmentu
     */
    void getFileOffsetsFromOpcodes(QStringList &opcodes, QList<uint32_t> &fileOffsets, uint32_t baseOffset);

    /**
     * @brief Metoda generująca kod trampoliny
     * @param realAddr Pierwotny adres skoku
     * @param wrapperAddr Adres metody do wywołania
     * @return Wygenerowany kod
     */
    template <typename Register>
    BinaryCode<Register> generateTrampolineCode(uint64_t realAddr, uint64_t wrapperAddr);

    /**
     * @brief Metoda losująca rejestr
     * @return Losowy rejestr
     */
    template <typename Register>
    uint8_t getRandomRegister();

public:
    /**
     * @brief Konstruktor
     */
    PEAddingMethods();

    ~PEAddingMethods();

    /**
     * @brief Metoda zabezpieczająca plik PE podanymi metodami
     * @param pe Plik do zabezpieczenia
     * @param descs Lista wybranych metod
     * @param codeCoverage Procentowe pokrycie kodu dla metod zamieniających adresy skoków
     * @return True w przypadku sukcesu
     */
    template <typename Register>
    bool injectCode(PEFile &pe, QList<InjectDescription<Register>*> descs, uint8_t codeCoverage);
};

#endif // PEADDINGMETHODS_H
