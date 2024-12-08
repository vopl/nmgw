#include "server.hpp"
#include "../rvzClient.hpp"
#include "../../utils.hpp"
#include <logger.hpp>

namespace entry::socks5
{
    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    Server::Server()
        : Base{asio2::detail::tcp_frame_size, asio2::detail::max_buffer_size, *utils::asio2Worker()}
    {
        bind_start([&]
        {
            asio::error_code ec = asio2::get_last_error();
            LOGI("tcp-to-socks5 server start: " << listen_address() << ":" << listen_port() << ", " << ec);
        });

        bind_stop([&]
        {
            asio::error_code ec = asio2::get_last_error();
            LOGI("tcp-to-socks5 server stop: " << listen_address() << ":" << listen_port() << ", " << ec);
        });

        bind_accept([this](const std::shared_ptr<Session>& session)
        {
            asio::error_code ec = asio2::get_last_error();
            LOGI("tcp-to-socks5 server accept: " << session->remote_address() << ":" << session->remote_port() << ", " << ec);
        });

        bind_connect([this](const std::shared_ptr<Session>& session)
        {
            asio::error_code ec = asio2::get_last_error();
            LOGI("tcp-to-socks5 server connect: " << session->remote_address() << ":" << session->remote_port() << ", " << ec);
            assert(_rvzClient);
            session->setRvzClient(_rvzClient);
            session->setGateId(_gateId);
        });

        bind_disconnect([this](const std::shared_ptr<Session>& session)
        {
            asio::error_code ec = asio2::get_last_error();
            LOGI("tcp-to-socks5 server disconnect: " << session->remote_address() << ":" << session->remote_port() << ", " << ec);
            session->processClose();
        });

        bind_recv([this](const std::shared_ptr<Session>& session, std::string_view data)
        {
            session->processOutput(data);
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
        this->sessions().for_each([&](const SessionPtr& session)
        {
            session->setRvzClient(_rvzClient);
        });
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    void Server::setGateId(const std::string& gateId)
    {
        _gateId = gateId;
        this->sessions().for_each([&](const SessionPtr& session)
        {
            session->setGateId(_gateId);
        });
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    void Server::start()
    {
        Base::start("0.0.0.0", 1080);
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    void Server::stop()
    {
        Base::post([&]
        {
            Base::stop();
        });
    }
}
