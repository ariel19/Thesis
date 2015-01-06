#ifndef BINARYFILE_H
#define BINARYFILE_H

#include <QByteArray>

class BinaryFile
{
public:
    BinaryFile(QByteArray _data);
    virtual ~BinaryFile();

    /**
     * @brief Pobiera zawartość pliku binarnego.
     * @return Plik.
     */
    virtual QByteArray getData();

    /**
     * @brief Sprawdza czy w pamięci przechowywany jest poprawny plik.
     * @return True jeżeli plik jest poprawny.
     */
    virtual bool is_valid() const = 0;

    /**
     * @brief Metoda informująca czy wczytany plik jest 64-bitowy.
     * @return True gdy plik x64
     */
    virtual bool is_x64() const = 0;

    /**
     * @brief Metoda informująca czy wczytany plik jest 32-bitowy.
     * @return True gdy plik x86
     */
    virtual bool is_x86() const = 0;

protected:
    bool parsed;
    QByteArray b_data;
};

#endif // BINARYFILE_H
