#include "rvzClient.hpp"
#include "../utils.hpp"
#include <logger.hpp>
#include <cassert>

namespace entry
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
        if(const asio::error_code ec = asio2::get_last_error())
            LOGE("set cert: " << ec);

        _rpcsClient.set_dh_buffer(utils::qtReadAllFile(":/etc/dh2048.pem").toStdString());
        if(const asio::error_code ec = asio2::get_last_error())
            LOGE("set dh params: " << ec);

        _rpcsClient.bind_connect([this]
        {
            asio::error_code ec = asio2::get_last_error();
            LOGI("rvz-client connect: " << ec);
            for(const auto& cb: _onConnect)
                cb(ec);

            if(ec)
                return;

            if(_entryId != common::EntryId{})
            {
                _rpcsClient.async_call([this]
                {
                    asio::error_code ec = asio2::get_last_error();
                    LOGI("rvz-client call entry-intro: " << ec);
                }, "entry-intro", _entryId);
            }
        });

        _rpcsClient.bind_disconnect([this]
        {
            asio::error_code ec = asio2::get_last_error();
            LOGI("rvz-client disconnect: " << ec);
            for(const auto& cb: _onDisconnect)
                cb(ec);
        });

        _rpcsClient.bind("gate-list", [this](std::vector<common::GateId> gateIds)
        {
            for(const auto& cb: _onGateList)
                cb(gateIds);
        });

        _rpcsClient.bind("socks5-close", [this](common::Socks5Id id)
        {
            activateOnSocks5Closed(id);
        });

        _rpcsClient.bind("socks5-traf", [&](common::Socks5Id id, std::string data)
        {
            activateOnSocks5Input(id, std::move(data));
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
    void RvzClient::actualizeEntry(common::EntryId entryId)
    {
        if(_entryId != entryId)
        {
            _entryId = std::move(entryId);
            if(_rpcsClient.is_started())
            {
                _rpcsClient.async_call([this]
                {
                    asio::error_code ec = asio2::get_last_error();
                    LOGI("rvz-client call entry-intro: " << ec);
                }, "entry-intro", _entryId);
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
    void RvzClient::subscribeOnGateList(std::function<void(std::vector<common::GateId>)> cb)
    {
        _onGateList.emplace_back(std::move(cb));
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    void RvzClient::socks5Open(const common::GateId& gateId, std::function<void(common::Socks5Id)> completition)
    {
        _rpcsClient.async_call([this, completition](common::Socks5Id id)
        {
            if(const asio::error_code ec = asio2::get_last_error())
            {
                LOGI("rvz-client call entry-socks5-open: " << ec);
                if(id != common::Socks5Id{})
                {
                    activateOnSocks5Closed(id);
                    socks5Close(id);
                }
            }

            completition(id);
        }, "entry-socks5-open", gateId);
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    void RvzClient::subscribeOnSocks5Input(common::Socks5Id id, std::function<void(std::string)> cb)
    {
        auto [iter, inserted] = _onSocks5Input.emplace(id, std::move(cb));
        assert(inserted);
        (void)iter;
        (void)inserted;
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    void RvzClient::subscribeOnSocks5Closed(common::Socks5Id id, std::function<void()> cb)
    {
        auto [iter, inserted] = _onSocks5Closed.emplace(id, std::move(cb));
        assert(inserted);
        (void)iter;
        (void)inserted;
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    void RvzClient::socks5Output(common::Socks5Id id, std::string data)
    {
        _rpcsClient.async_call([this, id]()
        {
            if(const asio::error_code ec = asio2::get_last_error())
            {
                LOGI("rvz-client call entry-socks5-traf: " << ec);
                activateOnSocks5Closed(id);
                socks5Close(id);
            }
        }, "entry-socks5-traf", id, std::move(data));
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    void RvzClient::socks5Close(common::Socks5Id id)
    {
        _onSocks5Input.erase(id);
        _onSocks5Closed.erase(id);
        _rpcsClient.async_call([this]()
        {
            if(const asio::error_code ec = asio2::get_last_error())
                LOGI("rvz-client call entry-socks5-close: " << ec);
        }, "entry-socks5-close", id);
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    void RvzClient::activateOnSocks5Input(common::Socks5Id id, std::string data)
    {
        auto iter = _onSocks5Input.find(id);
        if(_onSocks5Input.end() == iter)
            socks5Close(id);
        else
            iter->second(std::move(data));
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    void RvzClient::activateOnSocks5Closed(common::Socks5Id id)
    {
        _onSocks5Input.erase(id);

        auto iter = _onSocks5Closed.find(id);
        if(_onSocks5Closed.end() != iter)
        {
            auto cb = std::move(iter->second);
            _onSocks5Closed.erase(iter);
            cb();
        }
    }

}
