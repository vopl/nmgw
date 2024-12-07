#include "server.hpp"
#include "../worker.hpp"

namespace entry::socks5
{
    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    Server::Server()
        : asio2::tcp_server_impl_t<Server, Session>{asio2::detail::tcp_frame_size, asio2::detail::max_buffer_size, *worker()}
    {
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    Server::~Server()
    {
        stop();
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    void Server::start()
    {
        assert(!"not impl");
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    void Server::stop()
    {
        //assert(!"not impl");
    }
}
