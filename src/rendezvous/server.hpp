#pragma once

#include <asio2/rpc/rpcs_server.hpp>
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/member.hpp>
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
        struct Sock5
        {
            SessionPtr  _entrySession;
            SessionPtr  _gateSession;
            int         _id{};
        };

        using Sock5ByEntrySession = bmi::member<Sock5, SessionPtr, &Sock5::_entrySession>;
        using Sock5ByGateSession  = bmi::member<Sock5, SessionPtr, &Sock5::_gateSession>;
        using Sock5ById           = bmi::member<Sock5, int       , &Sock5::_id>;

        using Sock5s = bmi::multi_index_container<
            Sock5,
            bmi::indexed_by<
                bmi::ordered_non_unique<bmi::tag<Sock5ByEntrySession>, Sock5ByEntrySession>,
                bmi::ordered_non_unique<bmi::tag<Sock5ByGateSession >, Sock5ByGateSession >,
                bmi::ordered_non_unique<bmi::tag<Sock5ById          >, Sock5ById          >
            >
        >;

        Sock5s _sock5s;
    };
}
