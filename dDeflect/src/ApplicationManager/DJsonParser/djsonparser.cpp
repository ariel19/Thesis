#include "djsonparser.h"

#include <ApplicationManager/dlogger.h>

/**
 * @brief DJsonParser::getInjectionRead zwraca wskaźnik do wczytanej metody
 * @return wskaźnik do wczytanej metody
 */
DAddingMethods<Registers_x86>::Wrapper *DJsonParser::getInjectionRead() const
{
    return injectionRead;
}


QString DJsonParser::path() const
{
    return m_path;
}

void DJsonParser::setPath(const QString &path)
{
    m_path = path;
}
DJsonParser::DJsonParser()
{
    m_path="";
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
typename DAddingMethods<Register>::Wrapper *DJsonParser::loadInjectDescription(QString name)
{
    QFile loadFile(m_path + name);

    if (!loadFile.open(QIODevice::ReadOnly)) {
        LOG_ERROR("Cannot open json file.");
        return nullptr;
    }

    QByteArray saveData = loadFile.readAll();

    QJsonDocument loadDoc(QJsonDocument::fromJson(saveData));
    QJsonObject obj = loadDoc.object();

    typename DAddingMethods<Register>::Wrapper *p = nullptr;
    QString typeStr = obj["type"].toString();
    typename DAddingMethods<Register>::Wrapper::WrapperType type = DAddingMethods<Register>::Wrapper::wrapperTypes[typeStr];

    switch(type)
    {
    case DAddingMethods<Register>::Wrapper::WrapperType::OepWrapper:
        p = new (std::nothrow) typename DAddingMethods<Register>::OEPWrapper;
        break;
    case DAddingMethods<Register>::Wrapper::WrapperType::TrampolineWrapper:
        p = new (std::nothrow) typename DAddingMethods<Register>::TrampolineWrapper;
        break;
    case DAddingMethods<Register>::Wrapper::WrapperType::ThreadWrapper:
        p = new (std::nothrow) typename DAddingMethods<Register>::ThreadWrapper;
        break;
    default:
        p = new (std::nothrow) typename DAddingMethods<Register>::Wrapper;
        break;
    }

    if(!p)
        return nullptr;

    if(!p->read(obj))
    {
        delete p;
        p = nullptr;
        LOG_ERROR("Invalid format of json file.");
    }

    return p;
}
template DAddingMethods<Registers_x86>::Wrapper *DJsonParser::loadInjectDescription<Registers_x86>(QString name);
template DAddingMethods<Registers_x64>::Wrapper *DJsonParser::loadInjectDescription<Registers_x64>(QString name);

/**
 * @brief DJsonParser::saveIncjectionDescription zapisuje referencje obiekt wskazywany przez injection do pliku w formacie .json
 * @param injection referencja obiektu do zapisu
 * @param name nazwa metody
 * @return true w przypadku powodzenia false w p.p.
 */
bool DJsonParser::saveIncjectDescription(QString name, DAddingMethods<Registers_x86>::Wrapper &injection)
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
