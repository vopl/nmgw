#include "session.hpp"
#include "../rvzClient.hpp"
#include <logger.hpp>
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
    void Session::setGateId(const common::GateId& gateId)
    {
        if (_gateId != gateId)
        {
            if(_downstreamId != common::Socks5Id{})
            {
                _rvzClient->socks5Close(_downstreamId);
                _downstreamId = {};
            }
            _downStreamState = {};
            _output = {};

            _gateId = gateId;
            downstreamLogickTick();
        }
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    void Session::processOutput(std::string_view data)
    {
        if(!downstreamWorks())
        {
            if(!data.empty())
                _output.emplace_back(data);
            return;
        }

        while(!_output.empty())
        {
            _rvzClient->socks5Output(_downstreamId, std::move(_output.front()));
            _output.pop_front();
        }

        if(!data.empty())
            _rvzClient->socks5Output(_downstreamId, std::string{data});
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    void Session::processClose()
    {
        if(_downstreamId != common::Socks5Id{})
        {
            _rvzClient->socks5Close(_downstreamId);
            _downstreamId = {};
        }
        _downStreamState = {};
        _output = {};
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    void Session::downstreamLogickTick()
    {
        if(!_rvzClient || !_gateId)
        {
            _downStreamState = {};
            _downstreamId = {};
            return;
        }

        switch(_downStreamState)
        {
        case DownstreamState::null:
            _rvzClient->socks5Open(_gateId, [this, holder = shared_from_this()](common::Socks5Id downstreamId)
            {
                _downstreamId = downstreamId;
                if(downstreamId != common::Socks5Id{})
                {
                    _downStreamState = DownstreamState::work;

                    _rvzClient->subscribeOnSocks5Input(_downstreamId, [this, holder = shared_from_this()](std::string data)
                    {
                        if(!is_started())
                            return;

                        std::size_t dataSize = data.size();
                        async_send(std::move(data), [this, dataSize]
                        {
                            asio::error_code ec = asio2::get_last_error();
                            LOGI("tcp-to-socks5 session " << remote_address() << ":" << remote_port() << " send " << dataSize << " bytes " << ec);
                        });
                    });

                    _rvzClient->subscribeOnSocks5Closed(_downstreamId, [this, holder = shared_from_this()]
                    {
                        if(!is_started())
                            return;

                        stop();
                    });

                    processOutput();
                }
                else
                {
                    _downStreamState = DownstreamState::fail;
                    start_timer(_reopenTimerId, std::chrono::seconds{1}, 1, [this]{ downstreamLogickTick();});
                }
            });
            _downStreamState = DownstreamState::opening;
            break;
        case DownstreamState::opening:
            break;
        case DownstreamState::work:
            break;
        case DownstreamState::fail:
            _downStreamState = DownstreamState::null;
            start_timer(_reopenTimerId, std::chrono::seconds{1}, 1, [this]{ downstreamLogickTick();});
            break;
        default:
            std::unreachable();
        }
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    bool Session::downstreamWorks() const
    {
        return DownstreamState::work == _downStreamState && _downstreamId != common::Socks5Id{};
    }
}
