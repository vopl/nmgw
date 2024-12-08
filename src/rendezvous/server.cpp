#include "server.hpp"
#include "../utils.hpp"
#include <logger.hpp>
#include <cassert>

namespace rendezvous
{
    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    Server::Server()
        : _rpcsServer{asio2::detail::tcp_frame_size, asio2::detail::max_buffer_size, *utils::asio2Worker()}
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

        _rpcsServer.bind_accept([this](const SessionPtr& session)
        {
            LOGI("rvz-server accept: " << session->remote_address() << ":" << session->remote_port() << ", " << asio2::get_last_error());
        });

        _rpcsServer.bind_connect([this](const SessionPtr& session)
        {
            LOGI("rvz-server connect: " << session->remote_address() << ":" << session->remote_port() << ", " << asio2::get_last_error());

            Client client{};
            client._session = session;
            _clients.insert(std::move(client));
        });

        _rpcsServer.bind_disconnect([this](const SessionPtr& session)
        {
            LOGI("rvz-server disconnect: " << session->remote_address() << ":" << session->remote_port() << ", " << asio2::get_last_error());

            auto& idx = _clients.get<ClientBySession>();
            auto iter = idx.find(session);
            if(idx.end() != iter)
            {
                Client prev = *iter;
                idx.erase(iter);
                if(!prev._gateId.empty())
                    distributeGateList();
            }
        });

        _rpcsServer.bind("gate-intro", [this](const SessionPtr& session, std::string gateId)
        {
            LOGI("rvz-server gate-intro: " << session->remote_address() << ":" << session->remote_port() << ", " << gateId);

            auto& idx = _clients.get<ClientBySession>();
            auto iter = idx.find(session);
            if(idx.end() != iter)
            {
                Client prev = *iter;
                if(prev._gateId != gateId)
                {
                    idx.modify(iter, [&](Client& next){ next._gateId = gateId; });
                    distributeGateList();
                }
            }
        });

        _rpcsServer.bind("entry-intro", [this](const SessionPtr& session, std::string entryId)
        {
            LOGI("rvz-server entry-intro: " << session->remote_address() << ":" << session->remote_port() << ", " << entryId);

            auto& idx = _clients.get<ClientBySession>();
            auto iter = idx.find(session);
            if(idx.end() != iter)
            {
                Client prev = *iter;
                if(prev._entryId != entryId)
                    idx.modify(iter, [&](Client& next){ next._entryId = entryId; });
            }

            if(!entryId.empty())
                distributeGateList(session);
        });

        _rpcsServer.bind("entry-sock5-open", [this](const SessionPtr& session, std::string gateId)
        {
            assert(0);
            return 0;
        });

        _rpcsServer.bind("entry-sock5-close", [this](const SessionPtr& session, int socks5Id)
        {
            assert(0);
        });

        _rpcsServer.bind("entry-sock5-traf", [&](const SessionPtr& session, int socks5Id, std::string data)
        {
            assert(0);
        });

        _rpcsServer.bind("gate-sock5-close", [&](const SessionPtr& session, int socks5Id)
        {
            assert(0);
        });

        _rpcsServer.bind("gate-sock5-traf", [&](const SessionPtr& session, int socks5Id, std::string data)
        {
            assert(0);
        });
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

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    void Server::distributeGateList(const SessionPtr& target)
    {
        std::vector<std::string> gateIds;
        for(auto iter=_clients.begin(); iter!=_clients.end(); ++iter)
            if(!iter->_gateId.empty())
                gateIds.emplace_back(iter->_gateId);

        for(auto iter=_clients.begin(); iter!=_clients.end(); ++iter)
        {
            if(target && target!=iter->_session)
                continue;

            if(iter->_session->is_started())
                iter->_session->async_call("gate-list", gateIds);
        }
    }
}
