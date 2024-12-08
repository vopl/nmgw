#include "utils.hpp"
#include <QFile>
#include <logger.hpp>

namespace utils
{
    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
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

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    namespace
    {
        std::shared_ptr<asio2::iopool> g_asio2Worker = std::make_shared<asio2::iopool>(1);
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    std::shared_ptr<asio2::iopool> asio2Worker()
    {
        return g_asio2Worker;
    }
}
