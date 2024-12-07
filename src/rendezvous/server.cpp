#include "server.hpp"
#include "worker.hpp"
#include "../utils.hpp"
#include <logger.hpp>
#include <cassert>

namespace rendezvous
{
    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    Server::Server()
        : _rpcsServer{asio2::detail::tcp_frame_size, asio2::detail::max_buffer_size, *worker()}
    {
        _rpcsServer.set_verify_mode(asio::ssl::verify_peer | asio::ssl::verify_fail_if_no_peer_cert);
        _rpcsServer.set_cert_buffer(
                    utils::qtReadAllFile(":/etc/ca.crt").toStdString(),
                    utils::qtReadAllFile(":/etc/server.crt").toStdString(),
                    utils::qtReadAllFile(":/etc/server.key").toStdString(),
                    utils::qtReadAllFile(":/etc/server.pswd").toStdString());
        if(const asio::error_code ec = asio2::get_last_error())
            LOGE("set cert: " << ec);

        _rpcsServer.set_dh_buffer(utils::qtReadAllFile(":/etc/dh2048.pem").toStdString());
        if(const asio::error_code ec = asio2::get_last_error())
            LOGE("set dh params: " << ec);

        _rpcsServer.bind_start([&]()
        {
            LOGI("rvz-server start: " << _rpcsServer.listen_address() << ":" << _rpcsServer.listen_port() << ", " << asio2::get_last_error());
        });

        _rpcsServer.bind_stop([&]()
        {
            LOGI("rvz-server stop: " << _rpcsServer.listen_address() << ":" << _rpcsServer.listen_port() << ", " << asio2::get_last_error());
        });

        _rpcsServer.bind_accept([this](const std::shared_ptr<asio2::rpcs_session>& session)
        {
            LOGI("rvz-server accept: " << session->remote_address() << ":" << session->remote_port() << ", " << asio2::get_last_error());
        });

        _rpcsServer.bind_connect([this](const std::shared_ptr<asio2::rpcs_session>& session)
        {
            LOGI("rvz-server connect: " << session->remote_address() << ":" << session->remote_port() << ", " << asio2::get_last_error());
        });

        _rpcsServer.bind_disconnect([this](const std::shared_ptr<asio2::rpcs_session>& session)
        {
            LOGI("rvz-server disconnect: " << session->remote_address() << ":" << session->remote_port() << ", " << asio2::get_last_error());
        });

        // _rpcsServer.bind("sock5", [this]
        // {
        //     assert(_onSocks5.size() < 2);

        //     int res = -1;
        //     for(const auto& cb: _onSocks5)
        //         res = cb();
        //     return res;
        // });

        // _rpcsServer.bind("close", [this](int socks5Id)
        // {
        //     for(const auto& cb: _onClosed)
        //         cb(socks5Id);
        // });

        // _rpcsServer.bind("traf", [&](int socks5Id, std::string data)
        // {
        //     for(const auto& cb: _onInput)
        //         cb(socks5Id, data);
        // });
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    Server::~Server()
    {
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    void Server::start()
    {
        _rpcsServer.start("0.0.0.0", 28938);
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    void Server::stop()
    {
        _rpcsServer.stop();
    }

    // /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    // void RvzClient::onConnect(std::function<void()> cb)
    // {
    //     _onConnect.emplace_back(std::move(cb));
    // }

    // /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    // void RvzClient::onDisconnect(std::function<void()> cb)
    // {
    //     _onDisconnect.emplace_back(std::move(cb));
    // }

    // /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    // void RvzClient::onSocks5(std::function<int()> cb)
    // {
    //     _onSocks5.emplace_back(std::move(cb));
    // }

    // /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    // void RvzClient::onInput(std::function<void(int, std::string)> cb)
    // {
    //     _onInput.emplace_back(std::move(cb));
    // }

    // /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    // void RvzClient::onClosed(std::function<void(int)> cb)
    // {
    //     _onClosed.emplace_back(std::move(cb));
    // }

    // /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    // void RvzClient::output(int, std::string)
    // {
    //     assert(!"not impl");
    //     // client.send("write id data")
    // }

    // /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    // void RvzClient::close(int)
    // {
    //     assert(!"not impl");
    //     // client.send("closed id")
    // }
}
