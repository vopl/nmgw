#pragma once

#include <asio2/rpc/rpcs_client.hpp>
#include "../common.hpp"

namespace gate
{
    class RvzClient
    {
    public:
        RvzClient();
        ~RvzClient();

        void actualizeRendezvous(std::string host, std::string port);
        void actualizeGate(common::GateId gateId);

        void start();
        void stop();

        void subscribeOnConnect(std::function<void(asio::error_code)>);
        void subscribeOnDisconnect(std::function<void(asio::error_code)>);

        void subscribeOnSocks5Open(std::function<void(common::Socks5Id)>);
        void subscribeOnSocks5Input(std::function<void(common::Socks5Id, std::string)>);
        void subscribeOnSocks5Closed(std::function<void(common::Socks5Id)>);

        void socks5Output(common::Socks5Id, std::string);
        void socks5Close(common::Socks5Id);

    private:
        asio2::rpcs_client  _rpcsClient;
        bool                _started{};
        std::string         _rendezvousHost;
        std::string         _rendezvousPort;
        common::GateId      _gateId;

        std::vector<std::function<void(asio::error_code)>>              _onConnect;
        std::vector<std::function<void(asio::error_code)>>              _onDisconnect;
        std::vector<std::function<void(common::Socks5Id)>>              _onSocks5Open;
        std::vector<std::function<void(common::Socks5Id, std::string)>> _onSocks5Input;
        std::vector<std::function<void(common::Socks5Id)>>              _onSocks5Closed;
    };
}
