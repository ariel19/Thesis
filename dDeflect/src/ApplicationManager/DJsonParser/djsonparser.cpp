#include "djsonparser.h"

/**
 * @brief DJsonParser::getInjectionRead zwraca wskaźnik do wczytanej metody
 * @return wskaźnik do wczytanej metody
 */
DAddingMethods::InjectDescription<DAddingMethods::Registers_x86> *DJsonParser::getInjectionRead() const
{
    return injectionRead;
}

DJsonParser::DJsonParser(QString path) : m_path(path)
{
}
/**
 * @brief DJsonParser::loadInjectDescription tworzy obiekt InjectDescription deserializując go z z injectDescription.json
 * @param name nazwa metody
 * @return zwraca true w razie powodzenia false w przeciwnym wypadku
 */
DAddingMethods::InjectDescription<DAddingMethods::Registers_x86> * DJsonParser::loadInjectDescription(QString name)
{
    QFile loadFile(m_path+name);
    DAddingMethods::InjectDescription<DAddingMethods::Registers_x86> * p = new DAddingMethods::InjectDescription<DAddingMethods::Registers_x86>();
    if (!loadFile.open(QIODevice::ReadOnly)) {
        qWarning("Couldn't open save file.");
        return NULL;
    }

    QByteArray saveData = loadFile.readAll();

    QJsonDocument loadDoc(QJsonDocument::fromJson(saveData));

    p->read(loadDoc.object());

    return p;
}
/**
 * @brief DJsonParser::saveIncjectionDescription zapisuje referencje obiekt wskazywany przez injection do pliku w formacie .json
 * @param injection referencja obiektu do zapisu
 * @param name nazwa metody
 * @return true w przypadku powodzenia false w p.p.
 */
bool DJsonParser::saveIncjectDescription(QString name, DAddingMethods::InjectDescription<DAddingMethods::Registers_x86> &injection)
{
    QFile saveFile(m_path+name+".json");

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
