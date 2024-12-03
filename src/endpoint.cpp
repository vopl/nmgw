#include <asio2/rpc/rpcs_client.hpp>
#include <logger.hpp>
#include <filesystem>

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

    std::string_view host = "127.0.0.1";
    std::string_view port = "8011";

    asio2::rpcs_client client;

    // set default rpc call timeout
    client.set_default_timeout(std::chrono::seconds(5));

    //------------------------------------------------------------------------------
    // beacuse the server did not specify the verify_fail_if_no_peer_cert flag, so
    // our client may not load the ssl certificate.
    //------------------------------------------------------------------------------
    client.set_verify_mode(asio::ssl::verify_peer | asio::ssl::verify_fail_if_no_peer_cert);
    client.set_cert_file(
        "../etc/ca.crt",
        "../etc/client.crt",
        "../etc/client.key",
        "123456");
    client.set_dh_file("../etc/dh2048.pem");

    client.bind_connect([&]()
    {
        LOGI("connect: " << asio2::get_last_error());

        if (asio2::get_last_error())
            return;

        // the type of the callback's second parameter is auto, so you have to specify
        // the return type in the template function like 'async_call<int>'
        int x = std::rand(), y = std::rand();
        client.async_call<int>([x, y](auto v)
        {
            asio2::ignore_unused(x, y);
            if (!asio2::get_last_error())
            {
                ASIO2_ASSERT(v == x + y);
            }
            LOGI("sum1: " << v << ", " << asio2::get_last_error());
        }, "add", x, y);

    }).bind_disconnect([]()
    {
        LOGI("disconnect: " << asio2::get_last_error());
    });

    client.bind("sub", [](int a, int b) { return a - b; });

    client.async_start(host, port);

    client.start_timer("timer_id1", std::chrono::milliseconds(500), [&]()
    {
        std::string s1;
        s1 += '<';
        for (int i = 100 + std::rand() % (100); i > 0; i--)
        {
            s1 += (char)((std::rand() % 26) + 'a');
        }

        std::string s2;
        for (int i = 100 + std::rand() % (100); i > 0; i--)
        {
            s2 += (char)((std::rand() % 26) + 'a');
        }
        s2 += '>';

        client.async_call([s1, s2](std::string v)
        {
            if (!asio2::get_last_error())
            {
                ASIO2_ASSERT(v == s1 + s2);
            }
            LOGI("cat: " << v << ", " << asio2::get_last_error());

        }, "cat", s1, s2);
    });

    while (std::getchar() != '\n');

    return 0;
}
