#include "rvzClient.hpp"
#include "../utils.hpp"
#include <logger.hpp>
#include <cassert>

namespace std
{
    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    template <class T>
    std::ostream& operator<<(std::ostream& out, const std::vector<T>& vec)
    {
        out << '[';
        bool afterFirst = false;
        for(const T& e : vec)
        {
            if(afterFirst)
                out << ',';
            else
                afterFirst = true;

            out << e;
        }
        out << ']';
        return out;
    }
}

namespace entry
{
    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    RvzClient::RvzClient()
        : _rpcsClient{utils::initBufferSize, utils::maxBufferSize, *utils::asio2Worker()}
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

            if(_entryId)
            {
                _rpcsClient.async_call([entryId = _entryId]
                {
                    asio::error_code ec = asio2::get_last_error();
                    LOGI("rvz-client call entry-intro " << entryId << " " << ec);
                }, "entry-intro", _entryId);
            }

            _rpcsClient.start_timer("ping", std::chrono::seconds{60}, [this]
            {
                if(!_rpcsClient.is_started())
                    return;

                _rpcsClient.async_call([]
                {
                    asio::error_code ec = asio2::get_last_error();
                    LOGI("rvz-client call ping" << " " << ec);
                }, "ping");
            });
        });

        _rpcsClient.bind_disconnect([this]
        {
            asio::error_code ec = asio2::get_last_error();
            LOGI("rvz-client on disconnect " << ec);
            for(const auto& cb: _onDisconnect)
                cb(ec);
        });

        _rpcsClient.bind("pong", []
        {
            asio::error_code ec = asio2::get_last_error();
            LOGI("rvz-client on pong " << " " << ec);
        });

        _rpcsClient.bind("gate-list", [this](std::vector<common::GateId> gateIds)
        {
            asio::error_code ec = asio2::get_last_error();
            LOGI("rvz-client on gate-list " << gateIds << " " << ec);
            for(const auto& cb: _onGateList)
                cb(gateIds);
        });

        _rpcsClient.bind("socks5-close", [this](common::Socks5Id socks5Id)
        {
            asio::error_code ec = asio2::get_last_error();
            LOGI("rvz-client on socks5-close " << socks5Id << " " << ec);
            activateOnSocks5Closed(socks5Id);
        });

        _rpcsClient.bind("socks5-traf", [&](common::Socks5Id socks5Id, std::string data)
        {
            asio::error_code ec = asio2::get_last_error();
            LOGI("rvz-client on socks5-traf " << socks5Id << " " << data.size() << " bytes " << ec);
            activateOnSocks5Input(socks5Id, std::move(data));
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
                _rpcsClient.async_call([entryId = _entryId]
                {
                    asio::error_code ec = asio2::get_last_error();
                    LOGI("rvz-client call entry-intro " << entryId << " " << ec);
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
        _rpcsClient.async_call([this, completition, gateId](common::Socks5Id socks5Id)
        {
            asio::error_code ec = asio2::get_last_error();
            LOGI("rvz-client call entry-socks5-open " << gateId << " -> " << socks5Id << " " << ec);

            if(ec)
            {
                if(socks5Id != common::Socks5Id{})
                {
                    activateOnSocks5Closed(socks5Id);
                    socks5Close(socks5Id);
                }
            }

            completition(socks5Id);
        }, "entry-socks5-open", gateId);
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    void RvzClient::subscribeOnSocks5Input(common::Socks5Id socks5Id, std::function<void(std::string)> cb)
    {
        auto [iter, inserted] = _onSocks5Input.emplace(socks5Id, std::move(cb));
        assert(inserted);
        (void)iter;
        (void)inserted;
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    void RvzClient::subscribeOnSocks5Closed(common::Socks5Id socks5Id, std::function<void()> cb)
    {
        auto [iter, inserted] = _onSocks5Closed.emplace(socks5Id, std::move(cb));
        assert(inserted);
        (void)iter;
        (void)inserted;
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    void RvzClient::socks5Output(common::Socks5Id socks5Id, std::string data)
    {
        std::size_t dataSize = data.size();
        _rpcsClient.async_call([this, socks5Id, dataSize]()
        {
            asio::error_code ec = asio2::get_last_error();
            LOGI("rvz-client call entry-socks5-traf " << socks5Id << " " << dataSize << " bytes " << ec);

            if(ec)
            {
                activateOnSocks5Closed(socks5Id);
                socks5Close(socks5Id);
            }
        }, "entry-socks5-traf", socks5Id, std::move(data));
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    void RvzClient::socks5Close(common::Socks5Id socks5Id)
    {
        _onSocks5Input.erase(socks5Id);
        _onSocks5Closed.erase(socks5Id);
        _rpcsClient.async_call([socks5Id]()
        {
            asio::error_code ec = asio2::get_last_error();
            LOGI("rvz-client call entry-socks5-close " << socks5Id << " " << ec);
        }, "entry-socks5-close", socks5Id);
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    void RvzClient::activateOnSocks5Input(common::Socks5Id socks5Id, std::string data)
    {
        auto iter = _onSocks5Input.find(socks5Id);
        if(_onSocks5Input.end() == iter)
            socks5Close(socks5Id);
        else
            iter->second(std::move(data));
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    void RvzClient::activateOnSocks5Closed(common::Socks5Id socks5Id)
    {
        _onSocks5Input.erase(socks5Id);

        auto iter = _onSocks5Closed.find(socks5Id);
        if(_onSocks5Closed.end() != iter)
        {
            auto cb = std::move(iter->second);
            _onSocks5Closed.erase(iter);
            cb();
        }
    }
}
