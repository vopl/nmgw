#include "server.hpp"
#include "../rvzClient.hpp"
#include "../../utils.hpp"
#include <logger.hpp>

namespace entry::socks5
{
    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    Server::Server()
        : Base{utils::initBufferSize, utils::maxBufferSize, *utils::asio2Worker()}
    {
        bind_start([&]
        {
            asio::error_code ec = asio2::get_last_error();
            LOGI("tcp-to-socks5 server on start " << listen_address() << ":" << listen_port() << " " << ec);
        });

        bind_stop([&]
        {
            asio::error_code ec = asio2::get_last_error();
            LOGI("tcp-to-socks5 server on stop " << ec);
        });

        bind_accept([this](const std::shared_ptr<Session>& session)
        {
            asio::error_code ec = asio2::get_last_error();
            LOGI("tcp-to-socks5 session " << session->remote_address() << ":" << session->remote_port() << " on accept " << ec);
        });

        bind_connect([this](const std::shared_ptr<Session>& session)
        {
            asio::error_code ec = asio2::get_last_error();
            LOGI("tcp-to-socks5 session " << session->remote_address() << ":" << session->remote_port() << " on connect " << session->remote_address() << ":" << session->remote_port() << " " << ec);
            assert(_rvzClient);
            session->setRvzClient(_rvzClient);
            session->setGateId(_gateId);
        });

        bind_disconnect([this](const std::shared_ptr<Session>& session)
        {
            asio::error_code ec = asio2::get_last_error();
            LOGI("tcp-to-socks5 session " << session->remote_address() << ":" << session->remote_port() << " on disconnect " << session->remote_address() << ":" << session->remote_port() << " " << ec);
            session->processClose();
        });

        bind_recv([this](const std::shared_ptr<Session>& session, std::string_view data)
        {
            asio::error_code ec = asio2::get_last_error();
            LOGI("tcp-to-socks5 session " << session->remote_address() << ":" << session->remote_port() << " on recv " << session->remote_address() << ":" << session->remote_port() << " " << data.size() << " bytes " << ec);
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
        sessions().for_each([&](const SessionPtr& session)
        {
            session->setRvzClient(_rvzClient);
        });
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    void Server::setGateId(const common::GateId& gateId)
    {
        _gateId = gateId;
        sessions().for_each([&](const SessionPtr& session)
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
