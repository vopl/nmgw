#include "server.hpp"
#include "../../utils.hpp"
#include <logger.hpp>

namespace entry::socks5
{
    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    Server::Server()
        : Base{asio2::detail::tcp_frame_size, asio2::detail::max_buffer_size, *utils::asio2Worker()}
    {
        bind_start([&]()
        {
            LOGI("tcp-to-socks5 server start: " << listen_address() << ":" << listen_port() << ", " << asio2::get_last_error());
        });

        bind_stop([&]()
        {
            LOGI("tcp-to-socks5 server stop: " << listen_address() << ":" << listen_port() << ", " << asio2::get_last_error());
        });

        bind_accept([this](const std::shared_ptr<Session>& session)
        {
            LOGI("tcp-to-socks5 server accept: " << session->remote_address() << ":" << session->remote_port() << ", " << asio2::get_last_error());
        });

        bind_connect([this](const std::shared_ptr<Session>& session)
        {
            LOGI("tcp-to-socks5 server connect: " << session->remote_address() << ":" << session->remote_port() << ", " << asio2::get_last_error());
            assert(_rvzClient);
            session->setRvzClient(_rvzClient);
        });

        bind_disconnect([this](const std::shared_ptr<Session>& session)
        {
            LOGI("tcp-to-socks5 server disconnect: " << session->remote_address() << ":" << session->remote_port() << ", " << asio2::get_last_error());
        });

        bind_recv([this](const std::shared_ptr<Session>& session, std::string_view data)
        {
            session->output(data);
        });
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    Server::~Server()
    {
        stop();
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    void Server::setRvzClient(RvzClient* rvzClient)
    {
        assert(!_rvzClient);
        _rvzClient = rvzClient;
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    void Server::start()
    {
        Base::start("0.0.0.0", 1080);
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    void Server::stop()
    {
        Base::stop();
    }
}
