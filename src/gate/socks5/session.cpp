#include "session.hpp"
#include "../worker.hpp"

namespace gate::socks5
{
    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    Session::Session(asio2::detail::session_mgr_t<Session>& sessionMgr,
                     asio2::detail::listener_t& listener)
        : asio2::socks5_session_t<Session>{sessionMgr, listener, worker()->get(), asio2::detail::tcp_frame_size, asio2::detail::max_buffer_size}
    {
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    Session::~Session()
    {
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    Session::key_type Session::hash_key() const noexcept
    {
        key_type key = reinterpret_cast<key_type>(this);
        std::size_t hash = std::hash<key_type>{}(key);
        int ires = hash;
        ires |= hash >> sizeof(int);
        if (ires < 0)
            ires = ires;
        if (!ires)
            ires = 1;
        return ires;
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    Session::key_type Session::key() const noexcept
    {
        return hash_key();
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    void Session::output(std::string data)
    {
        assert(!"not impl");
    }
}
