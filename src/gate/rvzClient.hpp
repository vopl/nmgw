#pragma once

#include <asio2/rpc/rpcs_client.hpp>

namespace gate
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

        void onSocks5(std::function<int()>);
        void onInput(std::function<void(int, std::string)>);
        void onClosed(std::function<void(int)>);

        void output(int, std::string);
        void close(int);

    private:
        asio2::rpcs_client _rpcsClient;

        std::vector<std::function<void()>>                  _onConnect;
        std::vector<std::function<void()>>                  _onDisconnect;
        std::vector<std::function<int()>>                   _onSocks5;
        std::vector<std::function<void(int, std::string)>>  _onInput;
        std::vector<std::function<void(int)>>               _onClosed;
    };
}
