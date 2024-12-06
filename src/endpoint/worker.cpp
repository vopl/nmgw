#include "worker.hpp"

namespace endpoint
{
    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    std::shared_ptr<asio2::iopool> worker()
    {
        static std::shared_ptr<asio2::iopool> g = std::make_shared<asio2::iopool>(1);
        return g;
    }
}
