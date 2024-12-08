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
            if(ec)
                return;

            _rpcsClient.async_call([this]()
            {
                LOGI("rvz-client call entry-intro: " << asio2::get_last_error());
            }, "entry-intro", "blabla-i-am-entry");

            _rpcsClient.async_call([this](std::vector<std::string> geteIds)
            {
                LOGI("rvz-client call entry-get-gates: " << asio2::get_last_error());
                for(const std::string& gateId : geteIds)
                    LOGI("gateId: " << gateId);
                //TODO:  store gate identifiers
            }, "entry-get-gates");

            for(const auto& cb: _onConnect)
                cb();
        });

        _rpcsClient.bind_disconnect([this]
        {
            LOGI("rvz-client disconnect: " << asio2::get_last_error());
            for(const auto& cb: _onDisconnect)
                cb();
        });

        _rpcsClient.bind("sock5-close", [this](int id)
        {
            activateOnSock5Closed(id);
        });

        _rpcsClient.bind("sock5-traf", [&](int id, std::string data)
        {
            activateOnSock5Input(id, std::move(data));
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
    void RvzClient::subscribeOnConnect(std::function<void()> cb)
    {
        _onConnect.emplace_back(std::move(cb));
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    void RvzClient::subscribeOnDisconnect(std::function<void()> cb)
    {
        _onDisconnect.emplace_back(std::move(cb));
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    void RvzClient::socks5Open(std::function<void(int)> completition)
    {
        _rpcsClient.async_call([this, completition](int id)
        {
            if(const asio::error_code ec = asio2::get_last_error())
            {
                LOGI("rvz-client call entry-socks5-open: " << ec);
                activateOnSock5Closed(id);
                close(id);
                return;
            }
            completition(id);
        }, "entry-socks5-open", "");//TODO: provide gate identifier
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    void RvzClient::subscribeOnSock5Input(int id, std::function<void(std::string)> cb)
    {
        auto [iter, inserted] = _onSock5Input.emplace(id, std::move(cb));
        assert(inserted);
        (void)iter;
        (void)inserted;
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    void RvzClient::subscribeOnSock5Closed(int id, std::function<void()> cb)
    {
        auto [iter, inserted] = _onSock5Closed.emplace(id, std::move(cb));
        assert(inserted);
        (void)iter;
        (void)inserted;
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    void RvzClient::sock5Output(int id, std::string data)
    {
        _rpcsClient.async_call([this, id]()
        {
            if(const asio::error_code ec = asio2::get_last_error())
            {
                LOGI("rvz-client call entry-sock5-traf: " << ec);
                activateOnSock5Closed(id);
                close(id);
            }
        }, "entry-sock5-traf", id, std::move(data));
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    void RvzClient::sock5Close(int id)
    {
        _onSock5Input.erase(id);
        _onSock5Closed.erase(id);
        _rpcsClient.async_call([this]()
        {
            if(const asio::error_code ec = asio2::get_last_error())
                LOGI("rvz-client call entry-sock5-close: " << ec);
        }, "entry-sock5-close", id);
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    void RvzClient::activateOnSock5Input(int id, std::string data)
    {
        auto iter = _onSock5Input.find(id);
        if(_onSock5Input.end() == iter)
            close(id);
        else
            iter->second(std::move(data));
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    void RvzClient::activateOnSock5Closed(int id)
    {
        _onSock5Input.erase(id);

        auto iter = _onSock5Closed.find(id);
        if(_onSock5Closed.end() != iter)
        {
            auto cb = std::move(iter->second);
            _onSock5Closed.erase(iter);
            cb();
        }
    }

}
