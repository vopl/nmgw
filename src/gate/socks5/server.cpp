#include "server.hpp"
#include "../../utils.hpp"

namespace gate::socks5
{
    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    Server::Server()
        : _state{asio2::detail::state_t::stopped}
        , _sessionsMgr{utils::asio2Worker()->get(), _state}
        , _listener{}
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
        _state.store(asio2::detail::state_t::started);
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    void Server::stop()
    {
        _state.store(asio2::detail::state_t::stopping);
        _sessionsMgr.dispatch([this]
        {
            _sessionsMgr.for_each([](const SessionPtr& session)
            {
                session->stop();
            });
        });

        //assert(!"not impl");
        // for(auto& [_, socks5] : socks5s)
        //     socks5->close();
        // socks5s = {};

        // socks5IdGen = {};

    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    SessionPtr Server::open()
    {
        assert(!"not impl");
        // socks5IdGen++;

        // Socks5Ptr socks5 = Socks5::create(utils::asio2Worker()->get());
        // socks5->_onRead = [socks5IdGen, &rvzClient](std::string input)
        // {
        //     rvzClient.output(socks5IdGen, std::move(input));
        // };
        // socks5->startRead();
        // socks5s[socks5IdGen] = std::move(socks5);
        // return socks5IdGen;

    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    SessionPtr Server::get(Session::key_type sessionId)
    {
        return _sessionsMgr.find(sessionId);
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    bool Server::output(Session::key_type sessionId, std::string data)
    {
        gate::socks5::SessionPtr session = get(sessionId);
        if(!session)
            return false;
        session->output(std::move(data));
        return true;
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    void Server::close(Session::key_type sessionId)
    {
        assert(!"not impl");
        // socks5s.erase(socks5Id);
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    void Server::onInput(std::function<void(int, std::string)> cb)
    {
        _onInput.emplace_back(std::move(cb));
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    void Server::onClosed(std::function<void(int)> cb)
    {
        _onClosed.emplace_back(std::move(cb));
    }

}
