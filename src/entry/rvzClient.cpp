#include "rvzClient.hpp"
#include "worker.hpp"
#include "../utils.hpp"
#include <logger.hpp>
#include <cassert>

namespace entry
{
    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    RvzClient::RvzClient()
        : _rpcsClient{asio2::detail::tcp_frame_size, asio2::detail::max_buffer_size, *worker()}
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
            LOGI("rvz-client connect: " << asio2::get_last_error());
            for(const auto& cb: _onConnect)
                cb();
        });

        _rpcsClient.bind_disconnect([this]
        {
            LOGI("rvz-client disconnect: " << asio2::get_last_error());
            for(const auto& cb: _onDisconnect)
                cb();
        });

        _rpcsClient.bind("close", [this](int id)
        {
            activateOnClosed(id);
        });

        _rpcsClient.bind("traf", [&](int id, std::string data)
        {
            activateOnInput(id, std::move(data));
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
    void RvzClient::socks5(std::function<void(int)> completition)
    {
        _rpcsClient.async_call([this, completition](int id)
        {
            if(const asio::error_code ec = asio2::get_last_error())
            {
                LOGI("rvz-client call socks5: " << ec);
                activateOnClosed(id);
                close(id);
                return;
            }
            completition(id);
        }, "socks5");
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    void RvzClient::onInput(int id, std::function<void(std::string)> cb)
    {
        auto [iter, inserted] = _onInput.emplace(id, std::move(cb));
        assert(inserted);
        (void)iter;
        (void)inserted;
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    void RvzClient::onClosed(int id, std::function<void()> cb)
    {
        auto [iter, inserted] = _onClosed.emplace(id, std::move(cb));
        assert(inserted);
        (void)iter;
        (void)inserted;
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    void RvzClient::output(int id, std::string data)
    {
        _rpcsClient.async_call([this, id]()
        {
            if(const asio::error_code ec = asio2::get_last_error())
            {
                LOGI("rvz-client call traf: " << ec);
                activateOnClosed(id);
                close(id);
            }
        }, "traf", id, std::move(data));
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    void RvzClient::close(int id)
    {
        _onInput.erase(id);
        _onClosed.erase(id);
        _rpcsClient.async_call([this]()
        {
            if(const asio::error_code ec = asio2::get_last_error())
                LOGI("rvz-client call close: " << ec);
        }, "close", id);
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    void RvzClient::activateOnInput(int id, std::string data)
    {
        auto iter = _onInput.find(id);
        if(_onInput.end() == iter)
            close(id);
        else
            iter->second(std::move(data));
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    void RvzClient::activateOnClosed(int id)
    {
        auto iter = _onClosed.find(id);
        if(_onClosed.end() != iter)
        {
            auto cb = std::move(iter->second);
            _onClosed.erase(iter);
            cb();
        }
    }

}
