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
        void setGateId(const common::GateId& gateId);
        void start();
        void stop();

    private:
        RvzClient*      _rvzClient{};
        common::GateId  _gateId;
    };
}
