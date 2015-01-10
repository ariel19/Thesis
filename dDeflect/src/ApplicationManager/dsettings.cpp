#include "dsettings.h"

const QString DSettings::file_name = "settings.json";

DSettings::DSettings()
{
    _loaded = load();
}

bool DSettings::load()
{
    QFile f(file_name);

    if(!f.open(QFile::ReadOnly))
    {
        LOG_WARN("Settings file not found.");
        return false;
    }

    QByteArray data = f.readAll();
    f.close();

    QJsonParseError e;
    QJsonDocument doc = QJsonDocument::fromJson(data, &e);
    if(e.error != QJsonParseError::NoError)
    {
        LOG_ERROR("Invalid settings file format!");
        return false;
    }

    QJsonObject settings = doc.object();

    nasmPath = settings["nasm_path"].toString();
    ndisasmPath = settings["ndisasm_path"].toString();
    descriptionsPath_x86 = settings["desc_x86_path"].toString();
    descriptionsPath_x64 = settings["desc_x64_path"].toString();
    upxPath = settings["upx_path"].toString();

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

const QString DSettings::getUpxPath() const
{
    return upxPath;
}

bool DSettings::save()
{
    QFile f(file_name);

    if(!f.open(QFile::WriteOnly | QFile::Truncate))
    {
        LOG_ERROR("Saving the configuration to file failed.");
        return false;
    }

    QJsonObject settings;

    settings["nasm_path"] = nasmPath;
    settings["ndisasm_path"] = ndisasmPath;
    settings["desc_x86_path"] = descriptionsPath_x86;
    settings["desc_x64_path"] = descriptionsPath_x64;
    settings["upx_path"] = upxPath;

    QJsonDocument doc(settings);
    if(f.write(doc.toJson()) == -1)
    {
        f.close();
        LOG_ERROR("Saving the configuration to file failed.");
        return false;
    }

    f.close();
    _loaded = true;

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

void DSettings::setUpxPath(QString upx_path)
{
    upxPath = upx_path;
}

bool DSettings::loaded()
{
    return _loaded;
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
