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

        void onConnect(std::function<void()>);
        void onDisconnect(std::function<void()>);

    public:
        void socks5(std::function<void(int)>);

        void onInput(int id, std::function<void(std::string)>);
        void onClosed(int id, std::function<void()>);

        void output(int id, std::string data);
        void close(int id);

    private:
        void activateOnInput(int id, std::string data);
        void activateOnClosed(int id);

    private:
        asio2::rpcs_client _rpcsClient;

        std::vector<std::function<void()>>                  _onConnect;
        std::vector<std::function<void()>>                  _onDisconnect;

        std::map<int, std::function<void(std::string)>>     _onInput;
        std::map<int, std::function<void()>>                _onClosed;
    };
}
