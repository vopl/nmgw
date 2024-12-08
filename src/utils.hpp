#pragma once

#include <QString>
#include <QByteArray>
#include <asio2/base/iopool.hpp>

namespace utils
{
    QByteArray qtReadAllFile(QString path);
    std::shared_ptr<asio2::iopool> asio2Worker();
}
