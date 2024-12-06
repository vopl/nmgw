#include "utils.hpp"
#include <QFile>
#include <logger.hpp>

namespace endpoint::utils
{
    QByteArray qtReadAllFile(QString path)
    {
        QFile file{path};
        if(!file.open(QFile::ReadOnly))
        {
            LOGE("unable to open for reading: " << path.toStdString() << ", " << file.errorString().toStdString());
            return {};
        }
        return file.readAll();
    }
}
