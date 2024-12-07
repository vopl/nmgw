#include "session.hpp"
#include "../rvzClient.hpp"
#include <cassert>

namespace entry::socks5
{
    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    Session::Session(asio2::detail::session_mgr_t<Session>& sessionMgr,
                     asio2::detail::listener_t&             listener,
                     const std::shared_ptr<asio2::io_t>&    rwio,
                     std::size_t                            init_buf_size,
                     std::size_t                            max_buf_size)
        : Base{sessionMgr, listener, rwio, init_buf_size, max_buf_size}
    {
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    Session::~Session()
    {
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    void Session::setRvzClient(RvzClient* rvzClient)
    {
        assert(!_rvzClient);
        _rvzClient = rvzClient;
        downstreamLogickTick();
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    void Session::output(std::string_view data)
    {
        if(!downstreamWorks())
        {
            _output.emplace_back(data);
            return;
        }

        while(!_output.empty())
        {
            _rvzClient->sock5Output(_downstreamId, std::move(_output.front()));
            _output.pop_front();
        }
        _rvzClient->sock5Output(_downstreamId, std::string{data});
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    void Session::downstreamLogickTick()
    {
        if(!_rvzClient)
        {
            _downStreamState = {};
            _downstreamId = {};
            _output = {};
            return;
        }

        switch(_downStreamState)
        {
        case DownstreamState::null:
            _rvzClient->socks5Open([this](int downstreamId)
            {
                _downstreamId = downstreamId;
                _downStreamState = DownstreamState::work;

                _rvzClient->subscribeOnSock5Input(_downstreamId, [this](std::string data)
                {
                    async_send(std::move(data));
                });

                _rvzClient->subscribeOnSock5Closed(_downstreamId, [this]
                {
                    stop();
                });
            });
            _downStreamState = DownstreamState::opening;
            break;
        case DownstreamState::opening:
            break;
        case DownstreamState::work:
            break;
        default:
            std::unreachable();
        }
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    bool Session::downstreamWorks() const
    {
        return DownstreamState::work == _downStreamState;
    }

}
