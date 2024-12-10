#include "server.hpp"
#include "../utils.hpp"
#include <logger.hpp>
#include <cassert>

namespace std
{
    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    template <class I>
    I begin(const std::pair<I, I>& range)
    {
        return range.first;
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    template <class I>
    I end(const std::pair<I, I>& range)
    {
        return range.second;
    }

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

namespace rendezvous
{
    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    Server::Server()
        : _rpcsServer{utils::initBufferSize, utils::maxBufferSize, *utils::asio2Worker()}
    {
        _rpcsServer.set_verify_mode(asio::ssl::verify_peer | asio::ssl::verify_fail_if_no_peer_cert);
        _rpcsServer.set_cert_buffer(
                    utils::readAllFile("../etc/ca.crt"),
                    utils::readAllFile("../etc/server.crt"),
                    utils::readAllFile("../etc/server.key"),
                    utils::readAllFile("../etc/server.pswd"));
        asio::error_code ec = asio2::get_last_error();
        LOGI("set cert " << ec);

        _rpcsServer.set_dh_buffer(utils::readAllFile("../etc/dh2048.pem"));
        ec = asio2::get_last_error();
        LOGI("set dh params " << ec);

        _rpcsServer.bind_start([&]
        {
            asio::error_code ec = asio2::get_last_error();
            LOGI("rvz-server on start: " << _rpcsServer.listen_address() << ":" << _rpcsServer.listen_port() << " " << ec);
        });

        _rpcsServer.bind_stop([&]
        {
            asio::error_code ec = asio2::get_last_error();
            LOGI("rvz-server on stop " << _rpcsServer.listen_address() << ":" << _rpcsServer.listen_port() << " " << ec);
        });

        _rpcsServer.bind_accept([this](const SessionPtr& session)
        {
            asio::error_code ec = asio2::get_last_error();
            LOGI("rvz-server on accept " << session->remote_address() << ":" << session->remote_port() << " " << ec);
        });

        _rpcsServer.bind_connect([this](const SessionPtr& session)
        {
            asio::error_code ec = asio2::get_last_error();
            LOGI("rvz-server on connect " << session->remote_address() << ":" << session->remote_port() << " " << ec);

            Client client{};
            client._session = session;
            _clients.insert(std::move(client));
        });

        _rpcsServer.bind_disconnect([this](const SessionPtr& session)
        {
            asio::error_code ec = asio2::get_last_error();
            LOGI("rvz-server on disconnect " << session->remote_address() << ":" << session->remote_port() << " " << ec);

            {
                auto& idx = _clients.get<ClientBySession>();
                auto iter = idx.find(session);
                if(idx.end() != iter)
                {
                    Client prev = *iter;
                    idx.erase(iter);
                    if(prev._gateId)
                        distributeGateList();
                }
            }
            {
                auto& idx = _socks5s.get<Socks5ByEntrySession>();
                auto range = idx.equal_range(session);
                for(auto iter : range)
                {
                    iter._gateSession->async_call([session=iter._gateSession, id=iter._id]
                    {
                        asio::error_code ec = asio2::get_last_error();
                        LOGI("rvz-server call socks5-close " << session->remote_address() << ":" << session->remote_port() << " " << id << " " << ec);
                    }, "socks5-close", iter._id);
                }
                idx.erase(range.first, range.second);
            }
            {
                auto& idx = _socks5s.get<Socks5ByGateSession>();
                auto range = idx.equal_range(session);
                for(auto iter : range)
                    iter._entrySession->async_call([session=iter._entrySession, id=iter._id]
                    {
                        asio::error_code ec = asio2::get_last_error();
                        LOGI("rvz-server call socks5-close " << session->remote_address() << ":" << session->remote_port() << " " << id << " " << ec);
                    }, "socks5-close", iter._id);
                idx.erase(range.first, range.second);
            }
        });

        _rpcsServer.bind("ping", [this](const SessionPtr& session)
        {
            asio::error_code ec = asio2::get_last_error();
            LOGI("rvz-server on ping: " << session->remote_address() << ":" << session->remote_port() << " " << ec);

            session->async_call([session]
            {
                asio::error_code ec = asio2::get_last_error();
                LOGI("rvz-server call pong " << session->remote_address() << ":" << session->remote_port() << " " << ec);
            }, "pong");
        });

        _rpcsServer.bind("gate-intro", [this](const SessionPtr& session, common::GateId gateId)
        {
            asio::error_code ec = asio2::get_last_error();
            LOGI("rvz-server on gate-intro: " << session->remote_address() << ":" << session->remote_port() << " " << gateId << " " << ec);

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
            asio::error_code ec = asio2::get_last_error();
            LOGI("rvz-server on entry-intro: " << session->remote_address() << ":" << session->remote_port() << " " << entryId << " " << ec);

            auto& idx = _clients.get<ClientBySession>();
            auto iter = idx.find(session);
            if(idx.end() != iter)
            {
                Client prev = *iter;
                if(prev._entryId != entryId)
                    idx.modify(iter, [&](Client& next){ next._entryId = entryId; });
            }

            if(entryId)
                distributeGateList(session);
        });

        _rpcsServer.bind("entry-socks5-open", [this](const SessionPtr& entrySession, common::GateId gateId)
        {
            asio::error_code ec = asio2::get_last_error();
            LOGI("rvz-server on entry-socks5-open: " << entrySession->remote_address() << ":" << entrySession->remote_port() << " " << gateId << " " << ec);

            auto& idx = _clients.get<ClientByGateId>();
            auto iter = idx.find(gateId);
            if(idx.end() == iter)
                return common::Socks5Id{};

            ++_socks5IdGen._value;
            common::Socks5Id socks5Id{_socks5IdGen};

            SessionPtr gateSession = iter->_session;
            _socks5s.emplace(Socks5{entrySession, gateSession, socks5Id});

            gateSession->async_call([this, entrySession, gateSession, socks5Id]
            {
                asio::error_code ec = asio2::get_last_error();
                LOGI("rvz-server call socks5-open: " << gateSession->remote_address() << ":" << gateSession->remote_port() << " " << socks5Id << " " << ec);
                if(ec)
                {
                    _socks5s.get<Socks5ById>().erase(socks5Id);
                    if(entrySession->is_started())
                    {
                        entrySession->async_call([entrySession, socks5Id]
                        {
                            asio::error_code ec = asio2::get_last_error();
                            LOGI("rvz-server call socks5-close: " << entrySession->remote_address() << ":" << entrySession->remote_port() << " " << socks5Id << " " << ec);
                        }, "socks5-close", socks5Id);
                    }
                }
            }, "socks5-open", socks5Id);

            return socks5Id;
        });

        _rpcsServer.bind("entry-socks5-close", [this](const SessionPtr& entrySession, common::Socks5Id socks5Id)
        {
            asio::error_code ec = asio2::get_last_error();
            LOGI("rvz-server on entry-socks5-close: " << entrySession->remote_address() << ":" << entrySession->remote_port() << " " << socks5Id << " " << ec);

            auto& idx = _socks5s.get<Socks5ById>();
            auto iter = idx.find(socks5Id);
            if(idx.end() == iter)
                return;

            if(iter->_entrySession != entrySession)
            {
                LOGW("entry-socks5-close with non-associated socks5");
                return;
            }

            SessionPtr gateSession = iter->_gateSession;
            _socks5s.get<Socks5ById>().erase(socks5Id);
            gateSession->async_call([gateSession, socks5Id]
            {
                asio::error_code ec = asio2::get_last_error();
                LOGI("rvz-server call socks5-close: " << gateSession->remote_address() << ":" << gateSession->remote_port() << " " << socks5Id << " " << ec);
            }, "socks5-close", socks5Id);
        });

        _rpcsServer.bind("entry-socks5-traf", [&](const SessionPtr& entrySession, common::Socks5Id socks5Id, std::string data)
        {
            asio::error_code ec = asio2::get_last_error();
            LOGI("rvz-server on entry-socks5-traf: " << entrySession->remote_address() << ":" << entrySession->remote_port() << " " << socks5Id << " " << data.size() << " bytes " << ec);

            auto& idx = _socks5s.get<Socks5ById>();
            auto iter = idx.find(socks5Id);
            if(idx.end() == iter)
                return;

            if(iter->_entrySession != entrySession)
            {
                LOGW("entry-socks5-traf with non-associated socks5");
                return;
            }

            std::size_t dataSize = data.size();
            SessionPtr gateSession = iter->_gateSession;
            gateSession->async_call([gateSession, socks5Id, dataSize]
            {
                asio::error_code ec = asio2::get_last_error();
                LOGI("rvz-server call socks5-traf: " << gateSession->remote_address() << ":" << gateSession->remote_port() << " " << socks5Id << " " << dataSize << " bytes " << ec);
            }, "socks5-traf", socks5Id, std::move(data));
        });

        _rpcsServer.bind("gate-socks5-close", [&](const SessionPtr& gateSession, common::Socks5Id socks5Id)
        {
            asio::error_code ec = asio2::get_last_error();
            LOGI("rvz-server on gate-socks5-close: " << gateSession->remote_address() << ":" << gateSession->remote_port() << " " << socks5Id << " " << ec);

            auto& idx = _socks5s.get<Socks5ById>();
            auto iter = idx.find(socks5Id);
            if(idx.end() == iter)
                return;

            if(iter->_gateSession != gateSession)
            {
                LOGW("gate-socks5-close with non-associated socks5");
                return;
            }

            SessionPtr entrySession = iter->_entrySession;
            _socks5s.get<Socks5ById>().erase(socks5Id);
            entrySession->async_call([entrySession, socks5Id]
            {
                asio::error_code ec = asio2::get_last_error();
                LOGI("rvz-server call socks5-close: " << entrySession->remote_address() << ":" << entrySession->remote_port() << " " << socks5Id << " " << ec);
            }, "socks5-close", socks5Id);

        });

        _rpcsServer.bind("gate-socks5-traf", [&](const SessionPtr& gateSession, common::Socks5Id socks5Id, std::string data)
        {
            asio::error_code ec = asio2::get_last_error();
            LOGI("rvz-server on gate-socks5-traf: " << gateSession->remote_address() << ":" << gateSession->remote_port() << " " << socks5Id << " " << data.size() << " bytes " << ec);

            auto& idx = _socks5s.get<Socks5ById>();
            auto iter = idx.find(socks5Id);
            if(idx.end() == iter)
                return;

            if(iter->_gateSession != gateSession)
            {
                LOGW("gate-socks5-traf with non-associated socks5");
                return;
            }

            std::size_t dataSize = data.size();
            SessionPtr entrySession = iter->_entrySession;
            entrySession->async_call([entrySession, socks5Id, dataSize]
            {
                asio::error_code ec = asio2::get_last_error();
                LOGI("rvz-server call socks5-traf: " << entrySession->remote_address() << ":" << entrySession->remote_port() << " " << socks5Id << " " << dataSize << " bytes " << ec);
            }, "socks5-traf", socks5Id, std::move(data));
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
            if(iter->_gateId)
                gateIds.emplace_back(iter->_gateId);

        for(auto iter=_clients.begin(); iter!=_clients.end(); ++iter)
        {
            if(!iter->_entryId)
                continue;

            if(target && target!=iter->_session)
                continue;

            if(iter->_session->is_started())
            {
                iter->_session->async_call([entrySession=iter->_session, gateIds]
                {
                    asio::error_code ec = asio2::get_last_error();
                    LOGI("rvz-server call gate-list: " << entrySession->remote_address() << ":" << entrySession->remote_port() << " " << gateIds << " " << ec);
                }, "gate-list", gateIds);
            }
        }
    }
}
