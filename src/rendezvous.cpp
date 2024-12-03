
#include <asio2/rpc/rpcs_server.hpp>
#include <logger.hpp>
#include <filesystem>

namespace fs = std::filesystem;

int add(int a, int b)
{
    return a + b;
}

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

    std::string_view host = "0.0.0.0";
    std::string_view port = "8011";

    asio2::rpcs_server server;

    // use file for cert
    server.set_cert_file(
        "../etc/ca.crt",
        "../etc/server.crt",
        "../etc/server.key",
        "123456");
    server.set_dh_file("../etc/dh2048.pem");
    server.set_verify_mode(asio::ssl::verify_peer | asio::ssl::verify_fail_if_no_peer_cert);

    if (asio2::get_last_error())
        LOGE("load cert files failed: " << asio2::get_last_error());

    server.bind_connect([&](auto & session_ptr)
    {
        LOGI("client enter : " <<
            session_ptr->remote_address() << ":" << session_ptr->remote_port() <<
            session_ptr->local_address() << ":" << session_ptr->local_port());

        session_ptr->async_call([](int v)
        {
            if (!asio2::get_last_error())
            {
                ASIO2_ASSERT(v == 15 - 6);
            }
            LOGI("sub: " << v << ", " << asio2::get_last_error());
        }, std::chrono::seconds(10), "sub", 15, 6);

    }).bind_start([&]()
    {
        LOGI("start rpcs server: " << server.listen_address() << ":" << server.listen_port() << ", " << asio2::get_last_error());
    });

    server.bind("add", add);

    server.bind("cat", [&](std::shared_ptr<asio2::rpcs_session>& session_ptr,
        const std::string& a, const std::string& b)
    {
        int x = std::rand(), y = std::rand();
        // Nested call rpc function in business function is ok.
        session_ptr->async_call([x, y](int v)
        {
            asio2::ignore_unused(x, y);
            if (!asio2::get_last_error())
            {
                ASIO2_ASSERT(v == x - y);
            }
            LOGI("sub: " << v << ", " << asio2::get_last_error());
        }, "sub", x, y);

        return a + b;
    });

    server.start(host, port);
    server.wait_signal(SIGINT, SIGTERM);

    LOGI("done");
    return 0;
}
