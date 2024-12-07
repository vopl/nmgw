#pragma once

#include <asio2/rpc/rpcs_server.hpp>

namespace rendezvous
{
    class Server
    {
    public:
        Server();
        ~Server();

        void start();
        void stop();

        // void onConnect(std::function<void()>);
        // void onDisconnect(std::function<void()>);

        // void onSocks5(std::function<int()>);
        // void onInput(std::function<void(int, std::string)>);
        // void onClosed(std::function<void(int)>);

        // void output(int, std::string);
        // void close(int);

    private:
        asio2::rpcs_server _rpcsServer;

        // std::vector<std::function<void()>>                  _onConnect;
        // std::vector<std::function<void()>>                  _onDisconnect;
        // std::vector<std::function<int()>>                   _onSocks5;
        // std::vector<std::function<void(int, std::string)>>  _onInput;
        // std::vector<std::function<void(int)>>               _onClosed;
    };
}
