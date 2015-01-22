#ifndef SOURCECODEDESCRIPTION_H
#define SOURCECODEDESCRIPTION_H

#include <QObject>
#include <QList>
#include <QString>

class SourceCodeDescription : public QObject {
    Q_OBJECT
public:
    /**
     * @brief Obsługiwane systemy operacyjne.
     */
    enum class SystemType {
        Windows,
        Linux
    };

    /**
     * @brief Konstruktor.
     * @param parent rodzic.
     */
    explicit SourceCodeDescription(QObject *parent = 0) :
        QObject(parent) {}

    /**
     * @brief Nazwa metody.
     */
    QString name;

    /**
     * @brief Ścieżka do pliku z zawartością metody.
     */
    QString path;

    /**
     * @brief Opis metody.
     */
    QString description;

    /**
     * @brief Nagłówki, których potrzebuje metoda.
     */
    QList<QString> headers;

    /**
     * @brief Identyfikator systemu.
     */
    SystemType sys_type;

    /**
     * @brief Metoda odpowiada za wczytanie obiektu JSON do aktualnej instancji klasy.
     * @param json obiekt JSON.
     * @return True, jeżeli wczytanie się powiodło, False w innych przypadkach.
     */
    bool read(const QJsonObject & json);

signals:

public slots:

};

#endif // SOURCECODEDESCRIPTION_H
