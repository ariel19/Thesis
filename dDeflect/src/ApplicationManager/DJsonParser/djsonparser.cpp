#include "djsonparser.h"

/**
 * @brief DJsonParser::getInjectionRead zwraca wskaźnik do wczytanej metody
 * @return wskaźnik do wczytanej metody
 */
DAddingMethods::Wrapper<Registers_x86> *DJsonParser::getInjectionRead() const
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
template <typename Register>
DAddingMethods::Wrapper<Register> *DJsonParser::loadInjectDescription(QString name)
{
    // TODO: tworzenie Wrapper/OEPWrapper/ThreadWrapper itp
    QFile loadFile(m_path + name);

    if (!loadFile.open(QIODevice::ReadOnly)) {
        qWarning("Couldn't open save file.");
        return nullptr;
    }

    QByteArray saveData = loadFile.readAll();

    QJsonDocument loadDoc(QJsonDocument::fromJson(saveData));
    QJsonObject obj = loadDoc.object();

    DAddingMethods::Wrapper<Register> *p = obj["type"].toString() == "Thread" ?
                new DAddingMethods::ThreadWrapper<Register>() : new DAddingMethods::Wrapper<Register>();
    p->read(obj);

    return p;
}
template DAddingMethods::Wrapper<Registers_x86> *DJsonParser::loadInjectDescription(QString name);
template DAddingMethods::Wrapper<Registers_x64> *DJsonParser::loadInjectDescription(QString name);

/**
 * @brief DJsonParser::saveIncjectionDescription zapisuje referencje obiekt wskazywany przez injection do pliku w formacie .json
 * @param injection referencja obiektu do zapisu
 * @param name nazwa metody
 * @return true w przypadku powodzenia false w p.p.
 */
bool DJsonParser::saveIncjectDescription(QString name, DAddingMethods::Wrapper<Registers_x86> &injection)
{
//    QFile saveFile(m_path+name+".json");

//    if (!saveFile.open(QIODevice::WriteOnly)) {
//        qWarning("Couldn't open save file.");
//        return false;
//    }

//    QJsonObject iDescriptionObject;
//    injection.write(iDescriptionObject);
//    QJsonDocument saveDoc(iDescriptionObject);
//    saveFile.write(saveDoc.toJson());
    // TODO

    return true;
}
