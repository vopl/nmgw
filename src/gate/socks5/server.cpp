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
        asio::error_code ec = asio2::get_last_error();
        LOGI("socks5 server start " << ec);
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    void Server::stop()
    {
        for(auto& [socks5Id, session] : _implSessions)
        {

        }
        _implSessions.clear();
        if(_implServer.is_started())
        {
            _implServer.stop();
            asio::error_code ec = asio2::get_last_error();
            LOGI("socks5 server stop " << ec);
        }
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    void Server::open(common::Socks5Id socks5Id)
    {
        auto [iter, inserted] = _implSessions.emplace(
                                    socks5Id,
                                    std::make_shared<Session>());
        assert(inserted);
        SessionPtr session = iter->second;

        auto onClose = [socks5Id, this]
        {
            if(_implSessions.erase(socks5Id))
            {
                for(const auto& onClosed: _onClosed)
                    onClosed(socks5Id);
            }
        };

        session->bind_connect([socks5Id, this, session, onClose]()
        {
            asio::error_code ec = asio2::get_last_error();
            LOGI("socks5 session " << socks5Id << " on connect " << ec);

            if(ec)
            {
                onClose();
                return;
            }

            session->bind_recv([socks5Id, this, onClose](std::string_view data)
            {
                asio::error_code ec = asio2::get_last_error();
                LOGI("socks5 session " << socks5Id << " on recv " << data.size() << " bytes " << ec);

                if(ec)
                    return;

                for(const auto& onInput: _onInput)
                    onInput(socks5Id, std::string{data});
            });
        });

        session->bind_disconnect([socks5Id, this, session, onClose]()
        {
            asio::error_code ec = asio2::get_last_error();
            LOGI("socks5 session " << socks5Id << " on disconnect " << ec);
            onClose();
        });

        session->async_start("127.9.9.19", 17052);
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    bool Server::output(common::Socks5Id socks5Id, std::string data)
    {
        auto iter = _implSessions.find(socks5Id);
        if(_implSessions.end() == iter)
            return false;

        std::size_t dataSize = data.size();
        iter->second->async_send(std::move(data), [socks5Id, dataSize]
        {
            asio::error_code ec = asio2::get_last_error();
            LOGI("socks5 session " << socks5Id << " sent " << dataSize << " bytes " << ec);
        });
        return true;
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    void Server::close(common::Socks5Id socks5Id)
    {
        auto iter = _implSessions.find(socks5Id);
        if(_implSessions.end() == iter)
            return;

        SessionPtr session = iter->second;
        _implSessions.erase(iter);

        if(session->is_started())
            session->stop();

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
