#pragma once

#include <asio2/proxy/socks5_server.hpp>
#include <asio2/proxy/socks5_client.hpp>
#include "../../common.hpp"

namespace gate::socks5
{
    class Server
    {
        using Session = asio2::socks5_tcp_client;
        using SessionPtr = std::shared_ptr<Session>;

    public:
        Server();
        ~Server();

        void start();
        void stop();

        void open(common::Socks5Id socks5Id);
        bool output(common::Socks5Id socks5Id, std::string data);
        void close(common::Socks5Id socks5Id);

        void subscribeOnInput(std::function<void(common::Socks5Id, std::string)>);
        void subscribeOnClosed(std::function<void(common::Socks5Id)>);

    private:
        asio2::socks5_server _implServer;
        std::map<common::Socks5Id, SessionPtr> _implSessions;

        std::vector<std::function<void(common::Socks5Id, std::string)>>  _onInput;
        std::vector<std::function<void(common::Socks5Id)>>               _onClosed;
    };
}
