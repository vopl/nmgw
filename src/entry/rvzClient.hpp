#pragma once

#include <asio2/rpc/rpcs_client.hpp>

namespace entry
{
    class RvzClient
    {
    public:
        RvzClient();
        ~RvzClient();

        void actualizeRendezvous(std::string host, std::string port);
        void actualizeEntry(std::string entryId);

        void start();
        void stop();

        void subscribeOnConnect(std::function<void(asio::error_code)>);
        void subscribeOnDisconnect(std::function<void(asio::error_code)>);

        void subscribeOnGateList(std::function<void(std::vector<std::string>)>);
    public:
        void socks5Open(const std::string& gateId, std::function<void(int)>);

        void subscribeOnSocks5Input(int id, std::function<void(std::string)>);
        void subscribeOnSocks5Closed(int id, std::function<void()>);

        void socks5Output(int id, std::string data);
        void socks5Close(int id);

    private:
        void activateOnSocks5Input(int id, std::string data);
        void activateOnSocks5Closed(int id);

    private:
        asio2::rpcs_client _rpcsClient;
        bool        _started{};
        std::string _rendezvousHost;
        std::string _rendezvousPort;
        std::string _entryId;

        std::vector<std::function<void(asio::error_code)>> _onConnect;
        std::vector<std::function<void(asio::error_code)>> _onDisconnect;

        std::vector<std::function<void(std::vector<std::string>)>>  _onGateList;

        std::map<int, std::function<void(std::string)>>     _onSocks5Input;
        std::map<int, std::function<void()>>                _onSocks5Closed;
    };
}
