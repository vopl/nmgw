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
        void actualizeGate(std::string gateId);

        void start();
        void stop();

        void subscribeOnConnect(std::function<void(asio::error_code)>);
        void subscribeOnDisconnect(std::function<void(asio::error_code)>);

    public:
        void socks5Open(std::function<void(int)>);

        void subscribeOnSock5Input(int id, std::function<void(std::string)>);
        void subscribeOnSock5Closed(int id, std::function<void()>);

        void sock5Output(int id, std::string data);
        void sock5Close(int id);

    private:
        void activateOnSock5Input(int id, std::string data);
        void activateOnSock5Closed(int id);

    private:
        asio2::rpcs_client _rpcsClient;
        bool        _started{};
        std::string _rendezvousHost;
        std::string _rendezvousPort;
        std::string _entryId;
        std::string _gateId;

        std::vector<std::function<void(asio::error_code)>> _onConnect;
        std::vector<std::function<void(asio::error_code)>> _onDisconnect;

        std::map<int, std::function<void(std::string)>>     _onSock5Input;
        std::map<int, std::function<void()>>                _onSock5Closed;
    };
}
