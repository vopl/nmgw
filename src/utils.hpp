#pragma once

#ifdef QT_CORE_LIB
#   include <QString>
#   include <QByteArray>
#endif

#include <asio2/base/iopool.hpp>

namespace utils
{
#ifdef QT_CORE_LIB
    QByteArray qtReadAllFile(QString path);
#endif

    std::string readAllFile(const std::string& path);
    std::shared_ptr<asio2::iopool> asio2Worker();
}
