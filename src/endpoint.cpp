#include <asio2/rpc/rpcs_client.hpp>
#include <asio2/proxy/socks5_session.hpp>
#include <logger.hpp>
#include <filesystem>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QSettings>
#include <QFile>
#include "endpoint/worker.hpp"
#include "endpoint/rvzClient.hpp"
#include "endpoint/guiGate.hpp"
#include "endpoint/utils.hpp"


using namespace endpoint;
namespace fs = std::filesystem;


struct Socks5 : asio2::socks5_session_t<Socks5>
{
    static std::shared_ptr<asio2::io_t> _io;
    static std::atomic<asio2::detail::state_t> _fakeServerState;
    static asio2::detail::session_mgr_t<Socks5> _fakeSessionsHolder;
    static asio2::detail::listener_t _fakeListener;

    static std::shared_ptr<Socks5> create(std::shared_ptr<asio2::io_t> io)
    {
        asio::local::stream_protocol::socket clientSide{io->context()};
        // std::shared_ptr<asio::local::stream_protocol::socket> sessionSide{std::make_shared<asio::local::stream_protocol::socket>(io.context())};
        // asio::local::connect_pair(clientSide, *sessionSide);

        std::shared_ptr<asio::ip::tcp::socket> sessionSide{std::make_shared<asio::ip::tcp::socket>(io->context())};


        std::shared_ptr<Socks5> socks5{std::make_shared<Socks5>(std::move(io))};

        socks5->_clientSide = std::move(clientSide);

        return socks5;
    }

    Socks5(std::shared_ptr<asio2::io_t> io)
        : socks5_session_t{_fakeSessionsHolder, _fakeListener, io, asio2::detail::tcp_frame_size, asio2::detail::max_buffer_size}
        , _clientSide{io->context()}
    {
    }

    void write(std::string output)
    {
        _outputBuf.emplace_back(std::move(output));
        assert(0);
        //pump();
    }

    void close()
    {
        destroy();

        asio::error_code ec{};
        _clientSide.shutdown(asio::socket_base::shutdown_both, ec);
        _clientSide.close(ec);
    }

    void startRead()
    {
        _clientSide.async_read_some(asio::buffer(_inputBuf), [this, keep = shared_from_this()](asio::error_code ec, std::size_t size)
        {
            if(ec)
                return;

            assert(size <= _inputBuf.size());
            if(_onRead)
                _onRead(std::string{_inputBuf.begin(), _inputBuf.begin() + size});
            startRead();
        });
    }

    asio::local::stream_protocol::socket _clientSide;
    std::array<char, 1024> _inputBuf;
    std::deque<std::string> _outputBuf;
    std::function<void(std::string)> _onRead;
};
std::shared_ptr<asio2::io_t> Socks5::_io = worker()->get();
std::atomic<asio2::detail::state_t> Socks5::_fakeServerState = asio2::detail::state_t::started;
asio2::detail::session_mgr_t<Socks5> Socks5::_fakeSessionsHolder{_io, _fakeServerState};
asio2::detail::listener_t Socks5::_fakeListener;

using Socks5Ptr = std::shared_ptr<Socks5>;

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

    std::map<int, Socks5Ptr> socks5s;
    int socks5IdGen{};

    QGuiApplication app(argc, argv);
    QCoreApplication::setOrganizationName("vopl");
    QCoreApplication::setOrganizationDomain("vopl");
    QCoreApplication::setApplicationName("nmgw");

    GuiGate guiGate;

    QSettings settings;

    guiGate.setRendezvousHost(settings.value("rendezvousHost", "127.0.0.1").toString());
    guiGate.setRendezvousPort(settings.value("rendezvousPort", "8011").toString());

    RvzClient rvzClient;

    auto targetRvzClient = [&]
    {
        rvzClient.target(
                    guiGate.getRendezvousHost().toStdString(),
                    guiGate.getRendezvousPort().toStdString());
    };

    auto onRendezvousHostPortChanged = [&]
    {
        settings.setValue("rendezvousHost", guiGate.getRendezvousHost());
        settings.setValue("rendezvousPort", guiGate.getRendezvousPort());
        targetRvzClient();
    };

    QObject::connect(&guiGate, &GuiGate::rendezvousHostChanged, onRendezvousHostPortChanged);
    QObject::connect(&guiGate, &GuiGate::rendezvousPortChanged, onRendezvousHostPortChanged);


    rvzClient.onConnect([&]()
    {
        if (asio2::get_last_error())
        {
            QMetaObject::invokeMethod(&guiGate, [&]{guiGate.setRendezvousConnectivity(QString{"err, %1"}.arg(QString::fromLocal8Bit(asio2::get_last_error().message())));});
            return;
        }

        QMetaObject::invokeMethod(&guiGate, [&]{guiGate.setRendezvousConnectivity("ok");});
    });

    rvzClient.onDisconnect([&]
    {
        for(auto& [_, socks5] : socks5s)
            socks5->close();
        socks5s = {};

        socks5IdGen = {};
        QMetaObject::invokeMethod(&guiGate, [&]{guiGate.setRendezvousConnectivity("none");});
    });

    rvzClient.onSocks5([&]
    {
        socks5IdGen++;

        Socks5Ptr socks5 = Socks5::create(worker()->get());
        socks5->_onRead = [socks5IdGen, &rvzClient](std::string input)
        {
            rvzClient.output(socks5IdGen, std::move(input));
        };
        socks5->startRead();
        socks5s[socks5IdGen] = std::move(socks5);
        return socks5IdGen;
    });

    rvzClient.onClose([&](int socks5Id)
    {
        socks5s.erase(socks5Id);
    });

    rvzClient.onInput([&](int socks5Id, std::string data)
    {
        auto iter = socks5s.find(socks5Id);
        if(socks5s.end() == iter)
        {
            rvzClient.closed(socks5Id);
            return;
        }
        const Socks5Ptr& socks5 = iter->second;
        socks5->write(std::move(data));
    });

    targetRvzClient();

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("cppGate", &guiGate);

    const QUrl url{"qrc:/src/endpoint/gui.qml"};
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [url](QObject *obj, const QUrl &objUrl)
                            {
                                if (!obj && url == objUrl)
                                    QCoreApplication::exit(-1);
                            }, Qt::QueuedConnection
    );
    engine.load(url);

    return app.exec();
}
