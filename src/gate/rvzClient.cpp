#include "rvzClient.hpp"
#include "../utils.hpp"
#include <logger.hpp>
#include <cassert>

namespace gate
{
    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    RvzClient::RvzClient()
    {
        _rpcsClient.set_default_timeout(std::chrono::seconds(5));

        _rpcsClient.set_verify_mode(asio::ssl::verify_peer | asio::ssl::verify_fail_if_no_peer_cert);
        _rpcsClient.set_cert_buffer(
                    utils::qtReadAllFile(":/etc/ca.crt").toStdString(),
                    utils::qtReadAllFile(":/etc/client.crt").toStdString(),
                    utils::qtReadAllFile(":/etc/client.key").toStdString(),
                    utils::qtReadAllFile(":/etc/client.pswd").toStdString());
        if(const asio::error_code ec = asio2::get_last_error())
            LOGE("set cert: " << ec);

        _rpcsClient.set_dh_buffer(utils::qtReadAllFile(":/etc/dh2048.pem").toStdString());
        if(const asio::error_code ec = asio2::get_last_error())
            LOGE("set dh params: " << ec);

        _rpcsClient.bind_connect([this]
        {
            LOGI("connect: " << asio2::get_last_error());
            for(const auto& cb: _onConnect)
                cb();
        });

        _rpcsClient.bind_disconnect([this]
        {
            LOGI("disconnect: " << asio2::get_last_error());
            for(const auto& cb: _onDisconnect)
                cb();
        });

        _rpcsClient.bind("sock5", [this]
        {
            assert(_onSocks5.size() < 2);

            int res = -1;
            for(const auto& cb: _onSocks5)
                res = cb();
            return res;
        });

        _rpcsClient.bind("close", [this](int socks5Id)
        {
            for(const auto& cb: _onClosed)
                cb(socks5Id);
        });

        _rpcsClient.bind("traf", [&](int socks5Id, std::string data)
        {
            for(const auto& cb: _onInput)
                cb(socks5Id, data);
        });
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    RvzClient::~RvzClient()
    {
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    void RvzClient::start(std::string_view host, std::string_view port)
    {
        if (_rpcsClient.get_host() != host || _rpcsClient.get_port() != port)
        {
            _rpcsClient.stop();
            if(!host.empty() && !port.empty())
                _rpcsClient.async_start(std::string{host}, std::string{port});
        }
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    void RvzClient::stop()
    {
        _rpcsClient.stop();
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    void RvzClient::onConnect(std::function<void()> cb)
    {
        _onConnect.emplace_back(std::move(cb));
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    void RvzClient::onDisconnect(std::function<void()> cb)
    {
        _onDisconnect.emplace_back(std::move(cb));
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    void RvzClient::onSocks5(std::function<int()> cb)
    {
        _onSocks5.emplace_back(std::move(cb));
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    void RvzClient::onInput(std::function<void(int, std::string)> cb)
    {
        _onInput.emplace_back(std::move(cb));
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    void RvzClient::onClosed(std::function<void(int)> cb)
    {
        _onClosed.emplace_back(std::move(cb));
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    void RvzClient::output(int, std::string)
    {
        assert(!"not impl");
        // client.send("write id data")
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    void RvzClient::close(int)
    {
        assert(!"not impl");
        // client.send("closed id")
    }
}
