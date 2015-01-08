#ifndef DSETTINGS_H
#define DSETTINGS_H

#include <QString>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <core/file_types/codedefines.h>

class DSettings
{
private:
    DSettings();
    DSettings(const DSettings &);

    const static QString file_name;

    QString descriptionsPath_x64;
    QString descriptionsPath_x86;
    QString ndisasmPath;
    QString nasmPath;

    bool loaded;

    bool load();

public:
    static DSettings &getSettings()
    {
        static DSettings s;
        return s;
    }

    const QString getNasmPath() const;
    const QString getNdisasmPath() const;
    template <typename Register>
    const QString getDescriptionsPath() const;

    bool save();

    void setNasmPath(QString nasm_path);
    void setNdisasmPath(QString ndisasm_path);
    template <typename Register>
    void setDescriptionsPath(QString desc_path);

};

#endif // DSETTINGS_H
