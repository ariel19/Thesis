#include "djsonparser.h"

DJsonParser::DJsonParser() : m_path("")
{
}
/**
 * @brief DJsonParser::loadInjectDescription tworzy obiekt InjectDescription deserializując go z z injectDescription.json
 * @return zwraca true w razie powodzenia false w przeciwnym wypadku
 */
bool DJsonParser::loadInjectDescription()
{
    QFile loadFile(m_path+"injectDescription.json");

    if (!loadFile.open(QIODevice::ReadOnly)) {
        qWarning("Couldn't open save file.");
        return false;
    }

    QByteArray saveData = loadFile.readAll();

    QJsonDocument loadDoc(QJsonDocument::fromJson(saveData));

    injectionRead->read(loadDoc.object());

    return true;
}
/**
 * @brief DJsonParser::saveIncjectionDescription zapisuje referencje obiekt wskazywany przez injection do pliku w formacie .json
 * @param injection referencja obiektu do zapisu
 * @return true w przypadku powodzenia false w p.p.
 */
bool DJsonParser::saveIncjectionDescription(DAddingMethods::InjectDescription<DAddingMethods::Registers_x86> &injection)
{
    QFile saveFile(m_path+"injectDescription.json");

    if (!saveFile.open(QIODevice::WriteOnly)) {
        qWarning("Couldn't open save file.");
        return false;
    }

    QJsonObject iDescriptionObject;
    injection.write(iDescriptionObject);
    QJsonDocument saveDoc(iDescriptionObject);
    saveFile.write(saveDoc.toJson());

    return true;
}
