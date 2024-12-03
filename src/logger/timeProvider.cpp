#include <logger/timeProvider.hpp>
#include <utility>
#include <array>
#include <ctime>

namespace logger
{
    namespace
    {
        TimeProvider g_timeProvider{ []{ return std::chrono::system_clock::now(); } };

        char g_buf[64] {};

        struct LastState
        {
            std::chrono::system_clock::time_point _timeProvided {};
            std::chrono::seconds _secs {};

            std::string_view _strToSecs {};
            std::string_view _str {};
        } g_lastState {};
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    TimeProvider API_LOGGER setTimeProvider(TimeProvider tp)
    {
        return std::exchange(g_timeProvider, tp);
    }

    namespace
    {
        constexpr std::array<char, 1000*3> thousand = []
        {
            std::array<char, 1000*3> res;
            for(std::size_t i{}; i<res.size()/3; ++i)
            {
                res[i*3+0] = '0' + i/100%10;
                res[i*3+1] = '0' + i/10%10;
                res[i*3+2] = '0' + i/1%10;
            }
            return res;
        }();
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    std::string_view API_LOGGER timeProvidedAsString()
    {
        if(!g_timeProvider)
        {
            return {};
        }

        std::chrono::system_clock::time_point now = g_timeProvider();
        if(now == g_lastState._timeProvided)
        {
            return g_lastState._str;
        }

        g_lastState._timeProvided = now;

        std::chrono::microseconds time_since_epoch = std::chrono::duration_cast<std::chrono::microseconds>(now.time_since_epoch());

        std::size_t end;
        std::chrono::seconds secs = std::chrono::duration_cast<std::chrono::seconds>(time_since_epoch);
        if(secs == g_lastState._secs)
        {
            end = g_lastState._strToSecs.size();
        }
        else
        {
            std::time_t t = secs.count();
            struct tm tm;
#ifdef _WIN32
            localtime_s(&tm, &t);
#else
            localtime_r(&t, &tm);
#endif
            end = std::strftime(g_buf, sizeof(g_buf), "%Y-%m-%d %H:%M:%S", &tm);
            g_lastState._strToSecs = std::string_view{g_buf, end};
        }

        std::size_t msecs = static_cast<std::size_t>((time_since_epoch - std::chrono::duration_cast<std::chrono::microseconds>(secs)).count());

        g_buf[end+0] = '.';

        auto d3 = [&](int mult, char* buf)
        {
            int idx = msecs/mult % 1000;
            buf[0] = thousand[idx*3+0];
            buf[1] = thousand[idx*3+1];
            buf[2] = thousand[idx*3+2];
        };
        d3(1000, g_buf+end+1);
        d3(1, g_buf+end+4);

        g_buf[end+7] = 0;
        end += 7;

        g_lastState._str = std::string_view{g_buf, end};
        return g_lastState._str;
    }
}
