#pragma once

#include <asio2/tcp/tcp_session.hpp>

namespace entry::socks5
{
    class Session
        : public asio2::tcp_session_t<Session>
    {
    public:
        Session(asio2::detail::session_mgr_t<Session>& sessionMgr,
                asio2::detail::listener_t& listener);
        ~Session();

        void output(std::string data);

    private:
    };

    using SessionPtr = std::shared_ptr<Session>;
}
