#include "session.hpp"
#include "../../utils.hpp"

namespace gate::socks5
{
    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    Session::Session(asio2::detail::session_mgr_t<Session>& sessionMgr,
                     asio2::detail::listener_t& listener)
        : asio2::socks5_session_t<Session>{sessionMgr, listener, utils::asio2Worker()->get(), asio2::detail::tcp_frame_size, asio2::detail::max_buffer_size}
    {
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    Session::~Session()
    {
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    void Session::output(std::string data)
    {
        assert(!"not impl");
    }
}
