#pragma once

#include <asio2/tcp/tcp_server.hpp>
#include "session.hpp"

namespace entry::socks5
{
    class Server
        : public asio2::tcp_server_impl_t<Server, Session>
    {
        using Base = asio2::tcp_server_impl_t<Server, Session>;

    public:
        Server();
        ~Server();

        void setRvzClient(RvzClient* rvzClient);
        void setGateId(const std::string& gateId);
        void start();
        void stop();

        // SessionPtr open();
        // SessionPtr get(Session::key_type sessionId);
        // bool output(Session::key_type sessionId, std::string data);
        // void close(Session::key_type sessionId);

        // void onInput(std::function<void(int, std::string)>);
        // void onClosed(std::function<void(int)>);

    private:
        RvzClient*  _rvzClient{};
        std::string _gateId;
    };
}
