#pragma once

#include <cstdint>
#include <string>
#include <ostream>

namespace common
{
    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    struct Socks5Id
    {
        std::uint64_t _value{};

        explicit operator bool() const
        {
            return !!_value;
        }

        bool operator!() const
        {
            return !operator bool();
        }

        friend auto operator<=>(const Socks5Id&, const Socks5Id&) = default;
        friend bool operator==(const Socks5Id&, const Socks5Id&) = default;

        template <class Archive>
        void serialize(Archive & ar)
        {
            ar(_value);
        }

        friend std::ostream& operator<<(std::ostream& out, const Socks5Id& v)
        {
            return out << v._value;
        }
    };

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    struct EntryId
    {
        std::string _value{};

        explicit operator bool() const
        {
            return !_value.empty();
        }

        bool operator!() const
        {
            return !operator bool();
        }

        friend auto operator<=>(const EntryId&, const EntryId&) = default;
        friend bool operator==(const EntryId&, const EntryId&) = default;

        template <class Archive>
        void serialize(Archive & ar)
        {
            ar(_value);
        }

        friend std::ostream& operator<<(std::ostream& out, const EntryId& v)
        {
            return out << v._value;
        }
    };

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    struct GateId
    {
        std::string _value{};

        explicit operator bool() const
        {
            return !_value.empty();
        }

        bool operator!() const
        {
            return !operator bool();
        }

        friend auto operator<=>(const GateId&, const GateId&) = default;
        friend bool operator==(const GateId&, const GateId&) = default;

        template <class Archive>
        void serialize(Archive & ar)
        {
            ar(_value);
        }

        friend std::ostream& operator<<(std::ostream& out, const GateId& v)
        {
            return out << v._value;
        }
    };
}
