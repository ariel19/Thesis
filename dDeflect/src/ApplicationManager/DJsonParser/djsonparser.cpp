#include "djsonparser.h"

#include <ApplicationManager/dlogger.h>

/**
 * @brief DJsonParser::getInjectionRead zwraca wskaźnik do wczytanej metody
 * @return wskaźnik do wczytanej metody
 */
Wrapper<Registers_x86> *DJsonParser::getInjectionRead() const
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

DJsonParser::DJsonParser(QString path) : m_path(path) {}

 bool DJsonParser::loadSourceCodeDescription(QString name, SourceCodeDescription &scd) {
    QFile loadFile(m_path + name);

    if (!loadFile.open(QIODevice::ReadOnly)) {
        LOG_ERROR("Cannot open json file.");
        return false;
    }

    QByteArray saveData = loadFile.readAll();

    QJsonDocument loadDoc(QJsonDocument::fromJson(saveData));
    QJsonObject obj = loadDoc.object();

    return scd.read(obj);
}

/**
 * @brief DJsonParser::loadInjectDescription tworzy obiekt InjectDescription deserializując go z z injectDescription.json
 * @param name nazwa metody
 * @return zwraca true w razie powodzenia false w przeciwnym wypadku
 */
template <typename Register>
Wrapper<Register> *DJsonParser::loadInjectDescription(QString name)
{
    QFile loadFile(m_path + name);

    if (!loadFile.open(QIODevice::ReadOnly)) {
        LOG_ERROR("Cannot open json file.");
        return nullptr;
    }

    QByteArray saveData = loadFile.readAll();

    QJsonDocument loadDoc(QJsonDocument::fromJson(saveData));
    QJsonObject obj = loadDoc.object();

    Wrapper<Register> *p = nullptr;
    QString typeStr = obj["type"].toString();
    typename Wrapper<Register>::WrapperType type = Wrapper<Register>::wrapperTypes[typeStr];

    switch(type)
    {
    case Wrapper<Register>::WrapperType::OepWrapper:
        p = new (std::nothrow) OEPWrapper<Register>;
        break;
    case Wrapper<Register>::WrapperType::TrampolineWrapper:
        p = new (std::nothrow)  TrampolineWrapper<Register>;
        break;
    case Wrapper<Register>::WrapperType::ThreadWrapper:
        p = new (std::nothrow)  ThreadWrapper<Register>;
        break;
    default:
        p = new (std::nothrow) Wrapper<Register>;
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
template Wrapper<Registers_x86> *DJsonParser::loadInjectDescription<Registers_x86>(QString name);
template Wrapper<Registers_x64> *DJsonParser::loadInjectDescription<Registers_x64>(QString name);

/**
 * @brief DJsonParser::saveIncjectionDescription zapisuje referencje obiekt wskazywany przez injection do pliku w formacie .json
 * @param injection referencja obiektu do zapisu
 * @param name nazwa metody
 * @return true w przypadku powodzenia false w p.p.
 */
bool DJsonParser::saveIncjectDescription(QString name, Wrapper<Registers_x86> &injection)
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
