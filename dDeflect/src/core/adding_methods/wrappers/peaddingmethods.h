#ifndef PEADDINGMETHODS_H
#define PEADDINGMETHODS_H

#include <QProcess>
#include <QFile>
#include <QFileInfo>
#include <QTemporaryDir>

#include <core/adding_methods/wrappers/daddingmethods.h>
#include <core/file_types/pefile.h>
#include <ApplicationManager/DJsonParser/djsonparser.h>

/**
 * @brief Klasa odpowiedzialna za dodawanie metod zabezpieczających do plików PE
 */
template <typename Register>
class PEAddingMethods : public DAddingMethods<Register>
{
private:

    enum class ErrorCode
    {
        Success,
        BinaryFileNoPe,
        InvalidPeFile,
        NullInjectDescription,
        InvalidInjectDescription,
        NullWrapper,
        PeOperationFailed,
        NoThreadAction,
        ErrorLoadingFunctions,
        ToManyBytesForRelativeJump,
        InvalidParametersFormat,
        CannotCreateTempFile,
        CannotCreateTempDir,
        NasmFailed,
        NdisasmFailed,
        CannotOpenCompiledFile
    };

    /**
     * @brief Nazwa pliku opisującego funkcję pomocniczą ładującą dostęp do Windows API.
     */
    static const QString windowsApiLoadingFunction;

    /**
     * @brief Mapa z adresami wklejanych danych/kawałków kodu/napisów.
     */
    QMap<QByteArray, uint64_t> codePointers;

    /**
     * @brief Tablica adresów na wartości do zrelokowania
     */
    QList<uint64_t> relocations;

    /**
     * @brief Zawartość sekcji .text pliku PE
     */
    QByteArray text_section;

    /**
     * @brief Offset sekcji .text pliku PE
     */
    uint32_t text_section_offset;

    /**
     * @brief Procentowe pokrycie kodu dla metod zamieniających adresy skoków
     */
    uint8_t codeCoverage;

    /**
     * @brief Generator liczb losowych
     */
    std::default_random_engine gen;


    /**
     * @brief Metoda generująca kod ładujący parametry dla metod.
     * @param code Wygenerowany kod
     * @param getFunctionsCodeAddr Adres metody uzyskującej dostęp do API Windows
     * @param params Parametry metody
     * @param threadCodePtr Adres wklejonego kodu stworzenia wątku
     * @return Kod błędu
     */
    template <typename T>
    ErrorCode generateParametersLoadingCode(BinaryCode<Register> &code, T getFunctionsCodeAddr,
                                       QMap<Register, QString> params, T threadCodePtr);

    /**
     * @brief Metoda generująca kod sprawdzający warunek wywołania akcji (handlera) dla metody
     * @param code Wygenerowany kod
     * @param action Adres metody, która ma być wywołana jako akcja
     * @param cond Rejestr, w którym jest zwracana z metody flaga warunku
     * @param act Rejestr w którym znajdować będzie się adres akcji
     * @return Kod błędu
     */
    ErrorCode generateActionConditionCode(BinaryCode<Register> &code, uint64_t action, Register cond, Register act);

    /**
     * @brief Metoda dodająca kod w EntryPoint
     * @param epMethods Wybrane metody zabezpieczania
     * @return Kod błędu
     */
    ErrorCode injectEpCode(QList<uint64_t> &epMethods);

    /**
     * @brief Metoda dodająca kod do Thread Local Storage
     * @param tlsMethods Wybrane metody zabezpieczania
     * @return Kod błędu
     */
    ErrorCode injectTlsCode(QList<uint64_t> &tlsMethods);

    /**
     * @brief Metoda dodająca kod jako trampolinę
     * @param tramMethods Wybrane metody zabezpieczania
     * @return Kod błędu
     */
    ErrorCode injectTrampolineCode(QList<uint64_t> &tramMethods);

    /**
     * @brief Metoda generująca kod wątku
     * @param wrappers Metody do uruchomienia w wątku
     * @param codePtr Adres wygenerowanego kodu
     * @param sleepTime Czas snu wątku
     * @return Kod błędu
     */
    ErrorCode generateThreadCode(QList<typename DAddingMethods<Register>::Wrapper*> wrappers, uint64_t &codePtr, uint16_t sleepTime);

    /**
     * @brief Generowanie kodu metody
     * @param w Opis metody
     * @param codePtr Adres wygenerowanego kodu
     * @param isTlsCallback Informacja czy metoda jest callbackiem TLS
     * @return Kod błędu
     */
    ErrorCode generateCode(typename DAddingMethods<Register>::Wrapper *w, uint64_t &codePtr, bool isTlsCallback = false);

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
    BinaryCode<Register> generateTrampolineCode(uint64_t realAddr, uint64_t wrapperAddr);

    /**
     * @brief Metoda losująca rejestr
     * @return Losowy rejestr
     */
    Register getRandomRegister();

    /**
     * @brief Kompiluje kod assemblerowy
     * @param code Kod
     * @param compiled Skompilowany kod binarny
     * @return Kod błędu
     */
    ErrorCode compileCode(QByteArray code, QByteArray &compiled);

    /**
     * @brief Zabezpiecza plik podanymi metodami. Przekazuje błądy do funkcji nadrzędnej.
     * @param descs Lista wybranych metod
     * @return Kod błędu
     */
    ErrorCode safe_secure(const QList<typename DAddingMethods<Register>::InjectDescription*> &descs);

public:
    /**
     * @brief Konstruktor
     * @param f Plik PE
     */
    PEAddingMethods(PEFile *f);

    ~PEAddingMethods();

    /**
     * @brief Metoda zabezpieczająca plik PE podanymi metodami
     * @param descs Lista wybranych metod
     * @return True w przypadku sukcesu
     */
    virtual bool secure(const QList<typename DAddingMethods<Register>::InjectDescription*> &descs) override;

    /**
     * @brief Ustawia nowe procentowe pokrycie kodu
     * @param new_coverage Pokrycie kodu
     */
    void setCodeCoverage(uint8_t new_coverage);
};

#endif // PEADDINGMETHODS_H
