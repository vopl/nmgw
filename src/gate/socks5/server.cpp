#include "server.hpp"
#include "../../utils.hpp"
#include <logger.hpp>

namespace gate::socks5
{
    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    Server::Server()
        : _implServer{utils::initBufferSize, utils::maxBufferSize, *utils::asio2Worker()}
    {
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    Server::~Server()
    {
        stop();
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    void Server::start()
    {
        _implServer.bind_accept([](const ImplSessionPtr& session)
        {
            asio::error_code ec = asio2::get_last_error();
            LOGI("socks5 server on accept " << ec);

            if(!ec)
            {
                utils::setupTimeouts(session);

                asio2::socks5::options opts;
                opts.set_methods(asio2::socks5::method::anonymous);
                session->set_socks5_options(std::move(opts));
            }
        });

        _implServer.bind_connect([](const ImplSessionPtr& session)
        {
            asio::error_code ec = asio2::get_last_error();
            LOGI("socks5 server on connect " << ec);
        });

        _implServer.bind_disconnect([](const ImplSessionPtr& /*session*/)
        {
            asio::error_code ec = asio2::get_last_error();
            LOGI("socks5 server on disconnect " << ec);

        });

        _implServer.bind_socks5_handshake([](const ImplSessionPtr& /*session*/)
        {
            asio::error_code ec = asio2::get_last_error();
            LOGI("socks5 server on handshake " << ec);
        });

        _implServer.bind_start([]()
        {
            asio::error_code ec = asio2::get_last_error();
            LOGI("socks5 server on start " << ec);
        });

        _implServer.bind_stop([]()
        {
            asio::error_code ec = asio2::get_last_error();
            LOGI("socks5 server on stop " << ec);
        });

        _implServer.start("127.9.9.19", 17052);
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    void Server::stop()
    {
        _implServer.post([this]
        {
            auto implClients = std::move(_implClients);
            for(auto& [socks5Id, client] : implClients)
            {
                if(client->is_started())
                    client->stop();
            }

            if(_implServer.is_started())
                _implServer.stop();
        });
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    void Server::open(common::Socks5Id socks5Id)
    {
        auto [iter, inserted] = _implClients.emplace(
                                    socks5Id,
                                    std::make_shared<ImplClient>(utils::initBufferSize, utils::maxBufferSize, *utils::asio2Worker()));
        assert(inserted);
        ImplClientPtr client = iter->second;
        utils::setupTimeouts(client);

        auto onClose = [socks5Id, this]
        {
            if(_implClients.erase(socks5Id))
            {
                for(const auto& onClosed: _onClosed)
                    onClosed(socks5Id);
            }
        };

        client->bind_connect([socks5Id, this, client, onClose]()
        {
            asio::error_code ec = asio2::get_last_error();
            LOGI("socks5 client " << socks5Id << " on connect " << ec);

            if(ec)
            {
                onClose();
                return;
            }

            client->bind_recv([socks5Id, this, onClose](std::string_view data)
            {
                asio::error_code ec = asio2::get_last_error();
                LOGI("socks5 client " << socks5Id << " on recv " << data.size() << " bytes " << ec);

                if(ec)
                    return;

                for(const auto& onInput: _onInput)
                    onInput(socks5Id, std::string{data});
            });
        });

        client->bind_disconnect([socks5Id, this, client, onClose]()
        {
            asio::error_code ec = asio2::get_last_error();
            LOGI("socks5 client " << socks5Id << " on disconnect " << ec);
            onClose();
        });

        client->async_start("127.9.9.19", 17052);
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    bool Server::output(common::Socks5Id socks5Id, std::string data)
    {
        auto iter = _implClients.find(socks5Id);
        if(_implClients.end() == iter)
            return false;

        std::size_t dataSize = data.size();
        iter->second->async_send(std::move(data), [socks5Id, dataSize]
        {
            asio::error_code ec = asio2::get_last_error();
            LOGI("socks5 client " << socks5Id << " sent " << dataSize << " bytes " << ec);
        });
        return true;
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    void Server::close(common::Socks5Id socks5Id)
    {
        auto iter = _implClients.find(socks5Id);
        if(_implClients.end() == iter)
            return;

        ImplClientPtr implClient = iter->second;
        _implClients.erase(iter);

        if(implClient->is_started())
            implClient->stop();

        for(const auto& onClosed: _onClosed)
            onClosed(socks5Id);
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    void Server::closeAll()
    {
        while(!_implClients.empty())
            close(_implClients.begin()->first);
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    void Server::subscribeOnInput(std::function<void(common::Socks5Id, std::string)> cb)
    {
        _onInput.emplace_back(std::move(cb));
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    void Server::subscribeOnClosed(std::function<void(common::Socks5Id)> cb)
    {
        _onClosed.emplace_back(std::move(cb));
    }
}
