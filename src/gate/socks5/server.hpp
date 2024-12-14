#pragma once

#include <asio2/proxy/socks5_server.hpp>
#include <asio2/proxy/socks5_client.hpp>
#include "../../common.hpp"

namespace gate::socks5
{
    class Server
    {
        using ImplClient = asio2::socks5_tcp_client;
        using ImplClientPtr = std::shared_ptr<ImplClient>;

    public:
        Server();
        ~Server();

        void start();
        void stop();

        void open(common::Socks5Id socks5Id);
        bool output(common::Socks5Id socks5Id, std::string data);
        void close(common::Socks5Id socks5Id);
        void closeAll();

        void subscribeOnInput(std::function<void(common::Socks5Id, std::string)>);
        void subscribeOnClosed(std::function<void(common::Socks5Id)>);

    private:
        struct ImplSession : asio2::socks5_session_t<ImplSession>
        {
            using socks5_session_t::socks5_session_t;
            using socks5_session_t::back_client_;
        };
        using ImplSessionPtr = std::shared_ptr<ImplSession>;
        using ImplServer = asio2::socks5_server_t<ImplSession>;

        ImplServer                                  _implServer;
        std::map<common::Socks5Id, ImplClientPtr>   _implClients;

        std::vector<std::function<void(common::Socks5Id, std::string)>>  _onInput;
        std::vector<std::function<void(common::Socks5Id)>>               _onClosed;
    };
}
