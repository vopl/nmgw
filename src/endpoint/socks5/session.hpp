#pragma once

#include <asio2/proxy/socks5_session.hpp>

namespace endpoint::socks5
{
    class Session
        : public asio2::socks5_session_t<Session>
    {
    public:
        Session(asio2::detail::session_mgr_t<Session>& sessionMgr,
                asio2::detail::listener_t& listener);
        ~Session();

        key_type hash_key() const noexcept;
        key_type key() const noexcept;

        void output(std::string data);

    private:

    };

    using SessionPtr = std::shared_ptr<Session>;
}
