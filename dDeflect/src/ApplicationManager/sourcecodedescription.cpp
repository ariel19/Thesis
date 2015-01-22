#include "sourcecodedescription.h"

#include <QJsonObject>
#include <QJsonArray>

void SourceCodeDescription::copy(SourceCodeDescription *new_scd)
{
    if(!new_scd)
        return;

    new_scd->m_name = m_name;
    new_scd->description = description;
    new_scd->path = path;
    new_scd->headers = headers;
    new_scd->sys_type = sys_type;
}

bool SourceCodeDescription::read(const QJsonObject &json) {
    // name

    m_name = json["name"].toString();

    // description
    description = json["description"].toString();

    // path
    path = json["path"].toString();

    // headers
    QJsonArray _headers = json["headers"].toArray();
    foreach (auto h, _headers)
        headers.append(h.toString());

    // system type
    QString system_type = json["system"].toString();
    if (system_type.contains("linux"))
        sys_type = SystemType::Linux;
    else if (system_type.contains("windows"))
        sys_type = SystemType::Windows;
    else return false;

    return true;
}

QString SourceCodeDescription::name() const { return m_name; }
