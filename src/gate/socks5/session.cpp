#include "session.hpp"
#include "../../utils.hpp"
#include <logger.hpp>

namespace gate::socks5
{
    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    Session::Session()
        : asio2::socks5_tcp_client_t<Session>{asio2::detail::tcp_frame_size, asio2::detail::max_buffer_size, utils::asio2Worker()->get()}
    {
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    Session::~Session()
    {
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    void Session::output(std::string data)
    {
        if(is_started())
            send(std::move(data));
    }
}
