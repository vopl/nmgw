#pragma once

#include <asio2/proxy/socks5_server.hpp>
#include "session.hpp"

namespace gate::socks5
{
    class Server
    {
    public:
        Server();
        ~Server();

        void start();
        void stop();

        SessionPtr open();
        SessionPtr get(Session::key_type sessionId);
        bool output(Session::key_type sessionId, std::string data);
        void close(Session::key_type sessionId);

        void onInput(std::function<void(int, std::string)>);
        void onClosed(std::function<void(int)>);

    private:
        std::atomic<asio2::detail::state_t>     _state;
        asio2::detail::session_mgr_t<Session>   _sessionsMgr;
        asio2::detail::listener_t               _listener;

        std::vector<std::function<void(int, std::string)>>  _onInput;
        std::vector<std::function<void(int)>>               _onClosed;
    };
}
