#pragma once

#include <asio2/rpc/rpcs_server.hpp>
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/composite_key.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/sequenced_index.hpp>

namespace rendezvous
{
    namespace bmi = boost::multi_index;

    class Server
    {
        using SessionPtr = std::shared_ptr<asio2::rpcs_session>;
    public:
        Server();
        ~Server();

        void start();
        void stop();

    private:
        void distributeGateList(const SessionPtr& target = {});

    private:
        asio2::rpcs_server _rpcsServer;

    private:
        struct Client
        {
            SessionPtr  _session;
            std::string _entryId;
            std::string _gateId;
        };

        using ClientBySession = bmi::member<Client, SessionPtr , &Client::_session>;
        using ClientByEntryId = bmi::member<Client, std::string, &Client::_entryId>;
        using ClientByGateId  = bmi::member<Client, std::string, &Client::_gateId>;

        using Clients = bmi::multi_index_container<
            Client,
            bmi::indexed_by<
                bmi::ordered_unique    <bmi::tag<ClientBySession>, ClientBySession>,
                bmi::ordered_non_unique<bmi::tag<ClientByEntryId>, ClientByEntryId>,
                bmi::ordered_non_unique<bmi::tag<ClientByGateId >, ClientByGateId >
            >
        >;

        Clients _clients;

    private:
        struct Socks5
        {
            SessionPtr  _entrySession;
            SessionPtr  _gateSession;
            int         _id{};
        };

        using Socks5ByEntrySession = bmi::member<Socks5, SessionPtr, &Socks5::_entrySession>;
        using Socks5ByGateSession  = bmi::member<Socks5, SessionPtr, &Socks5::_gateSession>;
        using Socks5ById           = bmi::member<Socks5, int       , &Socks5::_id>;
        using Socks5ByEntrySessionAndId = bmi::composite_key<Socks5, Socks5ByEntrySession, Socks5ById>;
        using Socks5ByGateSessionAndId  = bmi::composite_key<Socks5, Socks5ByGateSession, Socks5ById>;

        using Socks5s = bmi::multi_index_container<
            Socks5,
            bmi::indexed_by<
                bmi::ordered_non_unique<bmi::tag<Socks5ByEntrySession>, Socks5ByEntrySession>,
                bmi::ordered_non_unique<bmi::tag<Socks5ByGateSession >, Socks5ByGateSession >,
                bmi::ordered_non_unique<bmi::tag<Socks5ById          >, Socks5ById          >,

                bmi::ordered_non_unique<bmi::tag<Socks5ByEntrySessionAndId>, Socks5ByEntrySessionAndId>,
                bmi::ordered_non_unique<bmi::tag<Socks5ByGateSessionAndId >, Socks5ByGateSessionAndId >
            >
        >;

        Socks5s _socks5s;
    };
}
