
#include "rendezvous/server.hpp"
#include "utils.hpp"
#include <logger.hpp>
#include <filesystem>

namespace rvz = rendezvous;
namespace fs = std::filesystem;

int main(int argc, char* argv[])
{
    //set current path near executable
    {
        std::error_code ec;
        fs::path executablePath = fs::canonical(argv[0], ec);
        if(ec)
        {
            LOGE("unable to determine current directory: "<<ec);
            return EXIT_FAILURE;
        }

        fs::path executableDir = executablePath.parent_path();
        fs::current_path(executableDir, ec);
        if(ec)
        {
            LOGE("unable to set current directory to "<<executableDir<<": "<<ec);
            return EXIT_FAILURE;
        }
    }

    utils::asio2Worker()->start();

    rvz::Server rvzServer;
    rvzServer.start();

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    std::promise<void> sigPromise;
    std::future<void> sigFuture = sigPromise.get_future();
    asio::signal_set signalset(utils::asio2Worker()->get_context());
    signalset.add(SIGINT);
    signalset.add(SIGTERM);
    signalset.async_wait([sigPromise = std::move(sigPromise)](const asio::error_code& ec, int signo) mutable
    {
        LOGI("SIG"<<sigabbrev_np(signo)<<": "<<ec);
        sigPromise.set_value();
    });

    sigFuture.wait();
    rvzServer.stop();

    utils::asio2Worker()->stop();
    LOGI("done");
    return 0;
}
