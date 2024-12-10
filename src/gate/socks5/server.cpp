#include "server.hpp"
#include "../../utils.hpp"
#include <logger.hpp>

namespace gate::socks5
{
    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    Server::Server()
    {
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    Server::~Server()
    {
        stop();
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    void Server::start()
    {
        _implServer.start("127.9.9.19", 17052);
        if(asio::error_code ec = asio2::get_last_error())
        {
            LOGE("socks5 server start: " << ec);
            return;
        }
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    void Server::stop()
    {
        _implSessions.clear();
        _implServer.stop();
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    SessionPtr Server::open(common::Socks5Id socks5Id)
    {
        auto [iter, inserted] = _implSessions.emplace(
                                    socks5Id,
                                    std::make_shared<Session>());
        assert(inserted);
        SessionPtr session = iter->second;

        auto processOnClose = [socks5Id, this]
        {
            _implSessions.erase(socks5Id);
            for(const auto& onClosed: _onClosed)
                onClosed(socks5Id);
        };

        session->async_start("127.9.9.19", 17052);
        session->bind_connect([socks5Id, this, session, processOnClose]()
        {
            if(asio::error_code ec = asio2::get_last_error())
            {
                LOGE("socks5 tcp client connect: " << ec);
                processOnClose();
                return;
            }

            session->bind_recv([socks5Id, this, processOnClose](std::string_view data)
            {
                LOGD("recv " << data.size());
                if(asio::error_code ec = asio2::get_last_error())
                {
                    LOGE("socks5 tcp client read: " << ec);
                    processOnClose();
                    return;
                }

                for(const auto& onInput: _onInput)
                    onInput(socks5Id, std::string{data});
            });
        });

        return iter->second;
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    SessionPtr Server::get(common::Socks5Id socks5Id)
    {
        auto iter = _implSessions.find(socks5Id);
        if(_implSessions.end() == iter)
            return {};
        return iter->second;
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    bool Server::output(common::Socks5Id socks5Id, std::string data)
    {
        auto iter = _implSessions.find(socks5Id);
        if(_implSessions.end() == iter)
            return false;

        LOGD("send " << data.size());
        iter->second->async_send(std::move(data));
        return true;
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    void Server::close(common::Socks5Id socks5Id)
    {
        auto iter = _implSessions.find(socks5Id);
        if(_implSessions.end() == iter)
            return;

        iter->second->stop();
        _implSessions.erase(iter);

        for(const auto& onClosed: _onClosed)
            onClosed(socks5Id);
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    void Server::subscribeOnInput(std::function<void(common::Socks5Id, std::string)> cb)
    {
        _onInput.emplace_back(std::move(cb));
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    void Server::subscribeOnClosed(std::function<void(common::Socks5Id)> cb)
    {
        _onClosed.emplace_back(std::move(cb));
    }
}
