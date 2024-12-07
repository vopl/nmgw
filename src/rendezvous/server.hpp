#pragma once

#include <asio2/rpc/rpcs_server.hpp>
#include <boost/multi_index_container.hpp>

namespace rendezvous
{
    class Server
    {
    public:
        Server();
        ~Server();

        void start();
        void stop();

    private:
        asio2::rpcs_server _rpcsServer;

        struct Entry
        {
            asio2::rpcs_session _session;
            std::string _id;
        };

        //boost::multi_index_container

        struct Gate
        {
            asio2::rpcs_session _session;
            std::string _id;
        };

        struct Sock5
        {
            asio2::rpcs_session _entrySession;
            asio2::rpcs_session _gateSession;
            int                 _sock5Id;
        };
    };
}
