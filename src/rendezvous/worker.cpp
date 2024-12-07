#include "worker.hpp"

namespace rendezvous
{
    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    std::shared_ptr<asio2::iopool> worker()
    {
#if BUILD_TYPE_DEBUG
        std::size_t concurrency = 1;
#else
        std::size_t concurrency = asio2::detail::default_concurrency();
#endif
        static std::shared_ptr<asio2::iopool> g = std::make_shared<asio2::iopool>(concurrency);
        return g;
    }
}
