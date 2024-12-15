#pragma once

#include <asio2/rpc/rpcs_client.hpp>
#include <common.hpp>

namespace entry
{
    class RvzClient
    {
    public:
        RvzClient();
        ~RvzClient();

        void actualizeRendezvous(std::string host, std::string port);
        void actualizeEntry(common::EntryId entryId);

        void start();
        void stop();

        void subscribeOnConnect(std::function<void(asio::error_code)>);
        void subscribeOnDisconnect(std::function<void(asio::error_code)>);

        void subscribeOnGateList(std::function<void(std::vector<common::GateId>)>);
    public:
        void socks5Open(const common::GateId& gateId, std::function<void(common::Socks5Id)>);

        void subscribeOnSocks5Input(common::Socks5Id socks5Id, std::function<void(std::string)>);
        void subscribeOnSocks5Closed(common::Socks5Id socks5Id, std::function<void()>);

        void socks5Output(common::Socks5Id socks5Id, std::string data);
        void socks5Close(common::Socks5Id socks5Id);

    private:
        void activateOnSocks5Input(common::Socks5Id socks5Id, std::string data);
        void activateOnSocks5Closed(common::Socks5Id socks5Id);

    private:
        asio2::rpcs_client  _rpcsClient;
        bool                _started{};
        std::string         _rendezvousHost;
        std::string         _rendezvousPort;
        common::EntryId     _entryId;

        std::vector<std::function<void(asio::error_code)>> _onConnect;
        std::vector<std::function<void(asio::error_code)>> _onDisconnect;

        std::vector<std::function<void(std::vector<common::GateId>)>>   _onGateList;

        std::map<common::Socks5Id, std::function<void(std::string)>>    _onSocks5Input;
        std::map<common::Socks5Id, std::function<void()>>               _onSocks5Closed;
    };
}
