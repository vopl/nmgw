#pragma once

#ifdef QT_CORE_LIB
#   include <QString>
#   include <QByteArray>
#   include <QFile>
#   include <logger.hpp>
#endif

#include <asio2/base/iopool.hpp>

namespace utils
{
    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
#ifdef QT_CORE_LIB
    inline QByteArray qtReadAllFile(QString path)
    {
        QFile file{path};
        if(!file.open(QFile::ReadOnly))
        {
            LOGE("unable to open for reading: " << path.toStdString() << " [" << file.errorString().toStdString() << "]");
            return {};
        }
        return file.readAll();
    }
#endif

    std::string readAllFile(const std::string& path);
    std::shared_ptr<asio2::iopool> asio2Worker();

    inline static constexpr std::size_t initBufferSize  = 32768;
    inline static constexpr std::size_t maxBufferSize   = (std::numeric_limits<std::size_t>::max)();

    inline static constexpr std::chrono::milliseconds transactionTimeout    = std::chrono::minutes{2};
    inline static constexpr std::chrono::milliseconds silenceTimeout        = std::chrono::hours{1};

    template <class Channel>
    void setupTimeouts(Channel& channel)
    {
        if constexpr(requires {channel->set_keep_alive(true)                   ;}) channel->set_keep_alive(true);
        if constexpr(requires {channel->set_silence_timeout(silenceTimeout)    ;}) channel->set_silence_timeout(silenceTimeout);
        if constexpr(requires {channel->set_default_timeout(transactionTimeout);}) channel->set_default_timeout(transactionTimeout);
        if constexpr(requires {channel->set_timeout(transactionTimeout)        ;}) channel->set_timeout(transactionTimeout);

        if constexpr(requires {channel.set_keep_alive(true)                   ;}) channel.set_keep_alive(true);
        if constexpr(requires {channel.set_silence_timeout(silenceTimeout)    ;}) channel.set_silence_timeout(silenceTimeout);
        if constexpr(requires {channel.set_default_timeout(transactionTimeout);}) channel.set_default_timeout(transactionTimeout);
        if constexpr(requires {channel.set_timeout(transactionTimeout)        ;}) channel.set_timeout(transactionTimeout);
    }
}
