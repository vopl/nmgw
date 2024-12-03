#pragma once

#include "logger/stream.hpp"
#include "logger/timeProvider.hpp"

#if !defined(LoggerIdentity)
#   if defined(ModuleName)
#       define LoggerIdentity ModuleName
#   elif defined(UnitName)
#       define LoggerIdentity UnitName
#   else
#       define LoggerIdentity ""
#   endif
#endif

#   define LOGF(...) logger::Stream{std::cout, "FTL", LoggerIdentity} << __VA_ARGS__
#   define LOGE(...) logger::Stream{std::cout, "ERR", LoggerIdentity} << __VA_ARGS__
#   define LOGW(...) logger::Stream{std::cout, "WRN", LoggerIdentity} << __VA_ARGS__
#   define LOGI(...) logger::Stream{std::cout, "INF", LoggerIdentity} << __VA_ARGS__
#   define LOGD(...) logger::Stream{std::cout, "DBG", LoggerIdentity} << __VA_ARGS__
#   define LOGT(...) logger::Stream{std::cout, "TRC", LoggerIdentity} << __VA_ARGS__
