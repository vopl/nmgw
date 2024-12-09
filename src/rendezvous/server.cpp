#include "server.hpp"
#include "../utils.hpp"
#include <logger.hpp>
#include <cassert>

namespace std
{
    template <class I>
    I begin(const std::pair<I, I>& range)
    {
        return range.first;
    }

    template <class I>
    I end(const std::pair<I, I>& range)
    {
        return range.second;
    }
}

namespace rendezvous
{
    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    Server::Server()
        : _rpcsServer{asio2::detail::tcp_frame_size, asio2::detail::max_buffer_size, *utils::asio2Worker()}
    {
        _rpcsServer.set_verify_mode(asio::ssl::verify_peer | asio::ssl::verify_fail_if_no_peer_cert);
        _rpcsServer.set_cert_buffer(
                    utils::readAllFile("../etc/ca.crt"),
                    utils::readAllFile("../etc/server.crt"),
                    utils::readAllFile("../etc/server.key"),
                    utils::readAllFile("../etc/server.pswd"));
        if(const asio::error_code ec = asio2::get_last_error())
            LOGE("set cert: " << ec);

        _rpcsServer.set_dh_buffer(utils::readAllFile("../etc/dh2048.pem"));
        if(const asio::error_code ec = asio2::get_last_error())
            LOGE("set dh params: " << ec);

        _rpcsServer.bind_start([&]
        {
            asio::error_code ec = asio2::get_last_error();
            LOGI("rvz-server start: " << _rpcsServer.listen_address() << ":" << _rpcsServer.listen_port() << ", " << ec);
        });

        _rpcsServer.bind_stop([&]
        {
            asio::error_code ec = asio2::get_last_error();
            LOGI("rvz-server stop: " << _rpcsServer.listen_address() << ":" << _rpcsServer.listen_port() << ", " << ec);
        });

        _rpcsServer.bind_accept([this](const SessionPtr& session)
        {
            asio::error_code ec = asio2::get_last_error();
            LOGI("rvz-server accept: " << session->remote_address() << ":" << session->remote_port() << ", " << ec);
        });

        _rpcsServer.bind_connect([this](const SessionPtr& session)
        {
            asio::error_code ec = asio2::get_last_error();
            LOGI("rvz-server connect: " << session->remote_address() << ":" << session->remote_port() << ", " << ec);

            Client client{};
            client._session = session;
            _clients.insert(std::move(client));
        });

        _rpcsServer.bind_disconnect([this](const SessionPtr& session)
        {
            asio::error_code ec = asio2::get_last_error();
            LOGI("rvz-server disconnect: " << session->remote_address() << ":" << session->remote_port() << ", " << ec);

            {
                auto& idx = _clients.get<ClientBySession>();
                auto iter = idx.find(session);
                if(idx.end() != iter)
                {
                    Client prev = *iter;
                    idx.erase(iter);
                    if(prev._gateId != common::GateId{})
                        distributeGateList();
                }
            }
            {
                auto& idx = _socks5s.get<Socks5ByEntrySession>();
                auto range = idx.equal_range(session);
                for(auto iter : range)
                    iter._gateSession->async_call("socks5-close", iter._id);
                idx.erase(range.first, range.second);
            }
            {
                auto& idx = _socks5s.get<Socks5ByGateSession>();
                auto range = idx.equal_range(session);
                for(auto iter : range)
                    iter._entrySession->async_call("socks5-close", iter._id);
                idx.erase(range.first, range.second);
            }
        });

        _rpcsServer.bind("gate-intro", [this](const SessionPtr& session, common::GateId gateId)
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

        _rpcsServer.bind("entry-intro", [this](const SessionPtr& session, common::EntryId entryId)
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

            if(entryId != common::EntryId{})
                distributeGateList(session);
        });

        _rpcsServer.bind("entry-socks5-open", [this](const SessionPtr& session, common::GateId gateId)
        {
            auto& idx = _clients.get<ClientByGateId>();
            auto iter = idx.find(gateId);
            if(idx.end() == iter)
                return common::Socks5Id{};

            ++_socks5IdGen._value;
            common::Socks5Id socks5Id{_socks5IdGen};

            _socks5s.emplace(Socks5{session, iter->_session, socks5Id});

            iter->_session->async_call([this, session, socks5Id]
            {
                asio::error_code ec = asio2::get_last_error();
                if(ec)
                {
                    _socks5s.get<Socks5ById>().erase(socks5Id);
                    if(session->is_started())
                        session->async_call("socks5-close", socks5Id);
                }
            }, "socks5-open", socks5Id);

            return socks5Id;
        });

        _rpcsServer.bind("entry-socks5-close", [this](const SessionPtr& session, common::Socks5Id socks5Id)
        {
            auto& idx = _socks5s.get<Socks5ById>();
            auto iter = idx.find(socks5Id);
            if(idx.end() == iter)
                return;

            if(iter->_entrySession != session)
            {
                LOGW("entry-socks5-close with non-associated socks5");
                return;
            }

            _socks5s.get<Socks5ById>().erase(socks5Id);
            iter->_gateSession->async_call("socks5-close", socks5Id);
        });

        _rpcsServer.bind("entry-socks5-traf", [&](const SessionPtr& session, common::Socks5Id socks5Id, std::string data)
        {
            auto& idx = _socks5s.get<Socks5ById>();
            auto iter = idx.find(socks5Id);
            if(idx.end() == iter)
                return;

            if(iter->_entrySession != session)
            {
                LOGW("entry-socks5-traf with non-associated socks5");
                return;
            }

            iter->_gateSession->async_call("socks5-traf", socks5Id, std::move(data));
        });

        _rpcsServer.bind("gate-socks5-close", [&](const SessionPtr& session, common::Socks5Id socks5Id)
        {
            auto& idx = _socks5s.get<Socks5ById>();
            auto iter = idx.find(socks5Id);
            if(idx.end() == iter)
                return;

            if(iter->_entrySession != session)
            {
                LOGW("gate-socks5-close with non-associated socks5");
                return;
            }

            _socks5s.get<Socks5ById>().erase(socks5Id);
            iter->_entrySession->async_call("socks5-close", socks5Id);
        });

        _rpcsServer.bind("gate-socks5-traf", [&](const SessionPtr& session, common::Socks5Id socks5Id, std::string data)
        {
            auto& idx = _socks5s.get<Socks5ById>();
            auto iter = idx.find(socks5Id);
            if(idx.end() == iter)
                return;

            if(iter->_entrySession != session)
            {
                LOGW("gate-socks5-traf with non-associated socks5");
                return;
            }

            iter->_entrySession->async_call("socks5-traf", socks5Id, std::move(data));
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
        _rpcsServer.post([&]
        {
            _rpcsServer.stop();
        });
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    void Server::distributeGateList(const SessionPtr& target)
    {
        std::vector<common::GateId> gateIds;
        for(auto iter=_clients.begin(); iter!=_clients.end(); ++iter)
            if(iter->_gateId != common::GateId{})
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
