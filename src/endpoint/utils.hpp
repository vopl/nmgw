#pragma once

#include <QString>
#include <QByteArray>

namespace endpoint::utils
{
    QByteArray qtReadAllFile(QString path);
}
