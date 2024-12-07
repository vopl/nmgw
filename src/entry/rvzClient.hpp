#pragma once

#include <asio2/rpc/rpcs_client.hpp>

namespace entry
{
    class RvzClient
    {
    public:
        RvzClient();
        ~RvzClient();

        void start(std::string_view host, std::string_view port);
        void stop();

        void subscribeOnConnect(std::function<void()>);
        void subscribeOnDisconnect(std::function<void()>);

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

        std::vector<std::function<void()>>                  _onConnect;
        std::vector<std::function<void()>>                  _onDisconnect;

        std::map<int, std::function<void(std::string)>>     _onSock5Input;
        std::map<int, std::function<void()>>                _onSock5Closed;
    };
}
