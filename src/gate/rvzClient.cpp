#include "rvzClient.hpp"
#include "../utils.hpp"
#include <logger.hpp>
#include <cassert>

namespace gate
{
    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    RvzClient::RvzClient()
        : _rpcsClient{asio2::detail::tcp_frame_size, asio2::detail::max_buffer_size, *utils::asio2Worker()}
    {
        _rpcsClient.set_default_timeout(std::chrono::seconds(5));

        _rpcsClient.set_verify_mode(asio::ssl::verify_peer | asio::ssl::verify_fail_if_no_peer_cert);
        _rpcsClient.set_cert_buffer(
                    utils::qtReadAllFile(":/etc/ca.crt").toStdString(),
                    utils::qtReadAllFile(":/etc/client.crt").toStdString(),
                    utils::qtReadAllFile(":/etc/client.key").toStdString(),
                    utils::qtReadAllFile(":/etc/client.pswd").toStdString());
        asio::error_code ec = asio2::get_last_error();
        LOGI("set cert " << ec);

        _rpcsClient.set_dh_buffer(utils::qtReadAllFile(":/etc/dh2048.pem").toStdString());
        ec = asio2::get_last_error();
        LOGI("set dh params " << ec);

        _rpcsClient.bind_connect([this]
        {
            asio::error_code ec = asio2::get_last_error();
            LOGI("rvz-client on connect " << ec);
            for(const auto& cb: _onConnect)
                cb(ec);

            if(ec)
                return;

            if(_gateId)
            {
                _rpcsClient.async_call([gateId = _gateId]()
                {
                    asio::error_code ec = asio2::get_last_error();
                    LOGI("rvz-client call gate-intro " << gateId << " " << ec);
                }, "gate-intro", _gateId);
            }
        });

        _rpcsClient.bind_disconnect([this]
        {
            asio::error_code ec = asio2::get_last_error();
            LOGI("rvz-client on disconnect " << ec);
            for(const auto& cb: _onDisconnect)
                cb(ec);
        });

        _rpcsClient.bind("socks5-open", [this](common::Socks5Id socks5Id)
        {
            asio::error_code ec = asio2::get_last_error();
            LOGI("rvz-client on socks5-open " << socks5Id << " " << ec);
            for(const auto& cb: _onSocks5Open)
                cb(socks5Id);
        });

        _rpcsClient.bind("socks5-close", [this](common::Socks5Id socks5Id)
        {
            asio::error_code ec = asio2::get_last_error();
            LOGI("rvz-client on socks5-close " << socks5Id << " " << ec);
            for(const auto& cb: _onSocks5Closed)
                cb(socks5Id);
        });

        _rpcsClient.bind("socks5-traf", [&](common::Socks5Id socks5Id, std::string data)
        {
            asio::error_code ec = asio2::get_last_error();
            LOGI("rvz-client on socks5-traf " << socks5Id << " " << data.size() << " bytes " << ec);
            for(const auto& cb: _onSocks5Input)
                cb(socks5Id, data);
        });
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    RvzClient::~RvzClient()
    {
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    void RvzClient::actualizeRendezvous(std::string host, std::string port)
    {
        if (_rendezvousHost != host || _rendezvousPort != port)
        {
            _rendezvousHost = std::move(host);
            _rendezvousPort = std::move(port);
            if (_started)
            {
                _rpcsClient.post([&]
                {
                    _rpcsClient.stop();
                    if(!_rendezvousHost.empty() && !_rendezvousPort.empty())
                        _rpcsClient.async_start(_rendezvousHost, _rendezvousPort);
                });
            }
        }
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    void RvzClient::actualizeGate(common::GateId gateId)
    {
        if(_gateId != gateId)
        {
            _gateId = std::move(gateId);
            if(_rpcsClient.is_started())
            {
                _rpcsClient.async_call([gateId=_gateId]()
                {
                    asio::error_code ec = asio2::get_last_error();
                    LOGI("rvz-client call gate-intro " << gateId << " "<< ec);
                }, "gate-intro", _gateId);
            }
        }
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    void RvzClient::start()
    {
        if(!_started)
        {
            _started = true;
            if(!_rendezvousHost.empty() && !_rendezvousPort.empty())
                _rpcsClient.async_start(_rendezvousHost, _rendezvousPort);
        }
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    void RvzClient::stop()
    {
        _started = false;
        _rpcsClient.post([&]
        {
            _rpcsClient.stop();
        });
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    void RvzClient::subscribeOnConnect(std::function<void(asio::error_code)> cb)
    {
        _onConnect.emplace_back(std::move(cb));
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    void RvzClient::subscribeOnDisconnect(std::function<void(asio::error_code)> cb)
    {
        _onDisconnect.emplace_back(std::move(cb));
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    void RvzClient::subscribeOnSocks5Open(std::function<void(common::Socks5Id)> cb)
    {
        _onSocks5Open.emplace_back(std::move(cb));
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    void RvzClient::subscribeOnSocks5Input(std::function<void(common::Socks5Id, std::string)> cb)
    {
        _onSocks5Input.emplace_back(std::move(cb));
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    void RvzClient::subscribeOnSocks5Closed(std::function<void(common::Socks5Id)> cb)
    {
        _onSocks5Closed.emplace_back(std::move(cb));
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    void RvzClient::socks5Output(common::Socks5Id socks5Id, std::string data)
    {
        std::size_t dataSize = data.size();
        _rpcsClient.async_call([socks5Id, dataSize]
        {
            asio::error_code ec = asio2::get_last_error();
            LOGI("rvz-client call gate-socks5-traf " << socks5Id << " " << dataSize << " bytes "<< ec);
        }, "gate-socks5-traf", socks5Id, std::move(data));
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    void RvzClient::socks5Close(common::Socks5Id socks5Id)
    {
        _rpcsClient.async_call([socks5Id]
        {
            asio::error_code ec = asio2::get_last_error();
            LOGI("rvz-client call gate-socks5-close " << socks5Id << " "<< ec);
        }, "gate-socks5-close", socks5Id);
    }
}
