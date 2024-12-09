#pragma once

#include <asio2/proxy/socks5_client.hpp>

namespace gate::socks5
{
    class Session
        : public asio2::socks5_tcp_client_t<Session>
    {
    public:
        Session();
        ~Session();

        void output(std::string data);

    private:

    };

    using SessionPtr = std::shared_ptr<Session>;
}
