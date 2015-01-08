#include "dsettings.h"

const QString DSettings::file_name = "settings.json";

DSettings::DSettings()
{
    loaded = load();
}

bool DSettings::load()
{
    QFile f(file_name);

    if(!f.open(QFile::ReadOnly))
        return false;

    QByteArray data = f.readAll();
    f.close();

    QJsonParseError e;
    QJsonDocument doc = QJsonDocument::fromJson(data, &e);
    if(e.error != QJsonParseError::NoError)
        return false;

    QJsonObject settings = doc.object();

    nasmPath = settings["nasm_path"].toString();
    ndisasmPath = settings["ndisasm_path"].toString();
    descriptionsPath_x86 = settings["desc_x86_path"].toString();
    descriptionsPath_x64 = settings["desc_x64_path"].toString();

    return true;
}

const QString DSettings::getNasmPath() const
{
    return nasmPath;
}

const QString DSettings::getNdisasmPath() const
{
    return ndisasmPath;
}

bool DSettings::save()
{
    QFile f(file_name);

    if(!f.open(QFile::WriteOnly | QFile::Truncate))
        return false;

    QJsonObject settings;

    settings["nasm_path"] = nasmPath;
    settings["ndisasm_path"] = ndisasmPath;
    settings["desc_x86_path"] = descriptionsPath_x86;
    settings["desc_x64_path"] = descriptionsPath_x64;

    QJsonDocument doc(settings);
    if(f.write(doc.toJson()) == -1)
    {
        f.close();
        return false;
    }

    f.close();
    loaded = true;

    return true;
}

void DSettings::setNasmPath(QString nasm_path)
{
    nasmPath = nasm_path;
}

void DSettings::setNdisasmPath(QString ndisasm_path)
{
    ndisasmPath = ndisasm_path;
}

template <>
const QString DSettings::getDescriptionsPath<Registers_x86>() const
{
    return descriptionsPath_x86;
}

template <>
const QString DSettings::getDescriptionsPath<Registers_x64>() const
{
    return descriptionsPath_x64;
}

template <>
void DSettings::setDescriptionsPath<Registers_x86>(QString desc_path)
{
    descriptionsPath_x86 = desc_path;
}

template <>
void DSettings::setDescriptionsPath<Registers_x64>(QString desc_path)
{
    descriptionsPath_x64 = desc_path;
}
