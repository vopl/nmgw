#include <logger.hpp>
#include <filesystem>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QSettings>
#include "endpoint/rvzClient.hpp"
#include "endpoint/socks5/server.hpp"
#include "endpoint/guiGate.hpp"
#include "endpoint/worker.hpp"
#include <cstring>

namespace ep = endpoint;
namespace fs = std::filesystem;


// struct Socks5 : asio2::socks5_session_t<Socks5>
// {
//     static std::shared_ptr<asio2::io_t> _io;
//     static std::atomic<asio2::detail::state_t> _fakeServerState;
//     static asio2::detail::session_mgr_t<Socks5> _fakeSessionsHolder;
//     static asio2::detail::listener_t _fakeListener;

//     static std::shared_ptr<Socks5> create(std::shared_ptr<asio2::io_t> io)
//     {
//         asio::local::stream_protocol::socket clientSide{io->context()};
//         // std::shared_ptr<asio::local::stream_protocol::socket> sessionSide{std::make_shared<asio::local::stream_protocol::socket>(io.context())};
//         // asio::local::connect_pair(clientSide, *sessionSide);

//         std::shared_ptr<asio::ip::tcp::socket> sessionSide{std::make_shared<asio::ip::tcp::socket>(io->context())};


//         std::shared_ptr<Socks5> socks5{std::make_shared<Socks5>(std::move(io))};

//         socks5->_clientSide = std::move(clientSide);

//         return socks5;
//     }

//     Socks5(std::shared_ptr<asio2::io_t> io)
//         : socks5_session_t{_fakeSessionsHolder, _fakeListener, io, asio2::detail::tcp_frame_size, asio2::detail::max_buffer_size}
//         , _clientSide{io->context()}
//     {
//     }

//     void write(std::string output)
//     {
//         _outputBuf.emplace_back(std::move(output));
//         assert(0);
//         //pump();
//     }

//     void close()
//     {
//         destroy();

//         asio::error_code ec{};
//         _clientSide.shutdown(asio::socket_base::shutdown_both, ec);
//         _clientSide.close(ec);
//     }

//     void startRead()
//     {
//         _clientSide.async_read_some(asio::buffer(_inputBuf), [this, keep = shared_from_this()](asio::error_code ec, std::size_t size)
//         {
//             if(ec)
//                 return;

//             assert(size <= _inputBuf.size());
//             if(_onRead)
//                 _onRead(std::string{_inputBuf.begin(), _inputBuf.begin() + size});
//             startRead();
//         });
//     }

//     asio::local::stream_protocol::socket _clientSide;
//     std::array<char, 1024> _inputBuf;
//     std::deque<std::string> _outputBuf;
//     std::function<void(std::string)> _onRead;
// };
// std::shared_ptr<asio2::io_t> Socks5::_io = worker()->get();
// std::atomic<asio2::detail::state_t> Socks5::_fakeServerState = asio2::detail::state_t::started;
// asio2::detail::session_mgr_t<Socks5> Socks5::_fakeSessionsHolder{_io, _fakeServerState};
// asio2::detail::listener_t Socks5::_fakeListener;

// using Socks5Ptr = std::shared_ptr<Socks5>;

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

    QGuiApplication app(argc, argv);
    QCoreApplication::setOrganizationName("vopl");
    QCoreApplication::setOrganizationDomain("vopl");
    QCoreApplication::setApplicationName("nmgw");

    ep::GuiGate guiGate;

    QSettings settings;

    guiGate.setRendezvousHost(settings.value("rendezvousHost", "127.0.0.1").toString());
    guiGate.setRendezvousPort(settings.value("rendezvousPort", "28938").toString());

    ep::RvzClient rvzClient;
    ep::socks5::Server socks5Server;

    auto startRvzClient = [&]
    {
        rvzClient.start(
                    guiGate.getRendezvousHost().toStdString(),
                    guiGate.getRendezvousPort().toStdString());
    };

    auto onRendezvousHostPortChanged = [&]
    {
        settings.setValue("rendezvousHost", guiGate.getRendezvousHost());
        settings.setValue("rendezvousPort", guiGate.getRendezvousPort());
        startRvzClient();
    };

    QObject::connect(&guiGate, &ep::GuiGate::rendezvousHostChanged, onRendezvousHostPortChanged);
    QObject::connect(&guiGate, &ep::GuiGate::rendezvousPortChanged, onRendezvousHostPortChanged);


    rvzClient.onConnect([&]()
    {
        if (asio::error_code ec = asio2::get_last_error())
        {
            socks5Server.stop();
            QMetaObject::invokeMethod(&guiGate, [&, txt=QString::fromLocal8Bit(ec.message())]{guiGate.setRendezvousConnectivity(txt);});
            return;
        }

        socks5Server.start();
        QMetaObject::invokeMethod(&guiGate, [&]{guiGate.setRendezvousConnectivity("ok");});
    });

    rvzClient.onDisconnect([&]
    {
        socks5Server.stop();
        QMetaObject::invokeMethod(&guiGate, [&]{guiGate.setRendezvousConnectivity("none");});
    });

    rvzClient.onSocks5([&]
    {
        return (int)socks5Server.open()->key();
    });

    rvzClient.onClosed([&](int socks5Id)
    {
        socks5Server.close(socks5Id);
    });

    rvzClient.onInput([&](int socks5Id, std::string data)
    {
        if(!socks5Server.output(socks5Id, std::move(data)))
            rvzClient.close(socks5Id);
    });

    socks5Server.onInput([&](int socks5Id, std::string data)
    {
        rvzClient.output(socks5Id, std::move(data));
    });

    socks5Server.onClosed([&](int socks5Id)
    {
        rvzClient.close(socks5Id);
    });

    startRvzClient();

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    asio::signal_set signalset(ep::worker()->get_context());
    signalset.add(SIGINT);
    signalset.add(SIGTERM);
    signalset.async_wait([&](const asio::error_code& ec, int signo)
    {
        if (ec)
            return;
        LOGI("SIG"<<sigabbrev_np(signo)<<": "<<ec);
        app.quit();
    });

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("cppGate", &guiGate);
    engine.load("qrc:/src/endpoint/gui.qml");

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    ep::worker()->start();
    int exitCode = app.exec();

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    rvzClient.stop();
    socks5Server.stop();
    asio::error_code ec;
    signalset.cancel(ec);
    ep::worker()->stop();

    LOGI("done");
    return exitCode;
}
