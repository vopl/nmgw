#pragma once
#include <asio2/base/iopool.hpp>

namespace rendezvous
{
    std::shared_ptr<asio2::iopool> worker();
}
