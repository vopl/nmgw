#pragma once

#include <sstream>
#include <iostream>
#include <string_view>
#include <system_error>
#include "timeProvider.hpp"

namespace logger
{
    class Stream
    {
        std::ostream& _out;
        std::stringstream _buf;

    public:
        Stream(std::ostream& out, std::string_view level, std::string_view identity)
            : _out{out}
        {
            auto putOne = [&](bool needSpace, std::string_view str) -> bool
            {
                if(str.empty())
                {
                    return false;
                }

                if(needSpace)
                    _buf << ' ';
                _buf << str;
                return true;
            };

            bool needSpace = false;
            needSpace |= putOne(needSpace, timeProvidedAsString());
            needSpace |= putOne(needSpace, level);
            needSpace |= putOne(needSpace, identity);
            _buf << ": ";
        }

        ~Stream()
        {
            _buf << '\n';
            std::string str{_buf.str()};
            _out.write(str.data(), str.size());
            _out.flush();
        }

        Stream& operator<<(const std::error_code& ec)
        {
            if(ec)
                _buf << '[' << ec.message() << ", " << ec.category().name() << '.' << ec.value() << ']';
            else
                _buf << "[ok]";
            return *this;
        }

        Stream& operator<<(const std::error_condition& ec)
        {
            if(ec)
                _buf << '[' << ec.message() << ", " << ec.category().name() << '.' << ec.value() << ']';
            else
                _buf << "[ok]";
            return *this;
        }

        template <class T> Stream& operator<<(const T& v)
        {
            _buf << v;
            return *this;
        }
    };

}
