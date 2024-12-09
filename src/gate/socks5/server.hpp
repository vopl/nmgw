#pragma once

#include <asio2/proxy/socks5_server.hpp>
#include "session.hpp"
#include "../../common.hpp"

namespace gate::socks5
{
    class Server
    {
    public:
        Server();
        ~Server();

        void start();
        void stop();

        SessionPtr open(common::Socks5Id socks5Id);
        SessionPtr get(common::Socks5Id socks5Id);
        bool output(common::Socks5Id socks5Id, std::string data);
        void close(common::Socks5Id socks5Id);

        void subscribeOnInput(std::function<void(common::Socks5Id, std::string)>);
        void subscribeOnClosed(std::function<void(common::Socks5Id)>);

    private:
        std::atomic<asio2::detail::state_t>     _state;
        asio2::detail::session_mgr_t<Session>   _sessionsMgr;
        asio2::detail::listener_t               _listener;

        std::vector<std::function<void(common::Socks5Id, std::string)>>  _onInput;
        std::vector<std::function<void(common::Socks5Id)>>               _onClosed;
    };
}
