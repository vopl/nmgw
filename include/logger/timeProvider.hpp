#pragma once

#include "api.hpp"
#include <chrono>
#include <string>

namespace logger
{
    using TimeProvider = std::chrono::system_clock::time_point(*)();

    TimeProvider API_LOGGER setTimeProvider(TimeProvider);
    std::string_view API_LOGGER timeProvidedAsString();
}
