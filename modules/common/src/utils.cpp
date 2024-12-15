#include "utils.hpp"
#include <logger.hpp>
#include <fstream>

namespace utils
{
    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    std::string readAllFile(const std::string& path)
    {
        std::ifstream ifs{path.c_str(), std::ios::in | std::ios::binary | std::ios::ate};
        if(!ifs)
        {
            LOGE("unable to open for reading: " << path);
            return {};
        }

        std::ifstream::pos_type fileSize = ifs.tellg();
        ifs.seekg(0, std::ios::beg);

        std::string res;
        res.resize(fileSize);
        ifs.read(res.data(), fileSize);

        return res;
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    namespace
    {
        std::shared_ptr<asio2::iopool> g_asio2Worker = std::make_shared<asio2::iopool>(1);
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    std::shared_ptr<asio2::iopool> asio2Worker()
    {
        return g_asio2Worker;
    }
}
