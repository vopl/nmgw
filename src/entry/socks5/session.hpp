#pragma once

#include <asio2/tcp/tcp_session.hpp>

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
        void output(std::string_view data);

    private:
        void downstreamLogickTick();
        bool downstreamWorks() const;

    private:
        enum class DownstreamState
        {
            null,
            opening,
            work,
        };

    private:
        RvzClient*      _rvzClient{};
        DownstreamState _downStreamState{};
        int             _downstreamId{};

        std::deque<std::string> _output;
    };

    using SessionPtr = std::shared_ptr<Session>;
}
