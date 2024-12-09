#pragma once

#include <asio2/tcp/tcp_session.hpp>
#include "../../common.hpp"

namespace entry
{
    class RvzClient;
}

namespace entry::socks5
{
    class Session
        : public asio2::tcp_session_t<Session>
    {
        using Base = asio2::tcp_session_t<Session>;

    public:
        Session(asio2::detail::session_mgr_t<Session>&  sessionMgr,
                asio2::detail::listener_t&              listener,
                const std::shared_ptr<asio2::io_t>&     rwio,
                std::size_t                             init_buf_size,
                std::size_t                             max_buf_size);
        ~Session();

        void setRvzClient(RvzClient* rvzClient);
        void setGateId(const common::GateId& gateId);
        void processOutput(std::string_view data);
        void processClose();

    private:
        void downstreamLogickTick();
        bool downstreamWorks() const;

    private:
        enum class DownstreamState
        {
            null,
            opening,
            work,
            fail,
        };

    private:
        RvzClient*              _rvzClient{};
        common::GateId          _gateId;
        DownstreamState         _downStreamState{};
        common::Socks5Id        _downstreamId{};
        static constexpr int    _reopenTimerId = 288451;

        std::deque<std::string> _output;
    };

    using SessionPtr = std::shared_ptr<Session>;
}
