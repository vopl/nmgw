#pragma once

#include <asio2/rpc/rpcs_client.hpp>

namespace gate
{
    class RvzClient
    {
    public:
        RvzClient();
        ~RvzClient();

        void actualizeRendezvous(std::string host, std::string port);
        void actualizeGate(std::string gateId);

        void start();
        void stop();

        void subscribeOnConnect(std::function<void(asio::error_code)>);
        void subscribeOnDisconnect(std::function<void(asio::error_code)>);

        void subscribeOnSocks5Open(std::function<int()>);
        void subscribeOnSocks5Input(std::function<void(int, std::string)>);
        void subscribeOnSocks5Closed(std::function<void(int)>);

        void socks5Output(int, std::string);
        void socks5Close(int);

    private:
        asio2::rpcs_client _rpcsClient;
        bool        _started{};
        std::string _rendezvousHost;
        std::string _rendezvousPort;
        std::string _gateId;

        std::vector<std::function<void(asio::error_code)>>  _onConnect;
        std::vector<std::function<void(asio::error_code)>>  _onDisconnect;
        std::vector<std::function<int()>>                   _onSocks5Open;
        std::vector<std::function<void(int, std::string)>>  _onSocks5Input;
        std::vector<std::function<void(int)>>               _onSocks5Closed;
    };
}
