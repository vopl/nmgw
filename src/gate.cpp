#include <logger.hpp>
#include <filesystem>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QSettings>
#include "gate/rvzClient.hpp"
#include "gate/socks5/server.hpp"
#include "gate/guiTalk.hpp"
#include "utils.hpp"
#include <cstring>

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

    QGuiApplication app(argc, argv);
    QCoreApplication::setOrganizationName("vopl");
    QCoreApplication::setOrganizationDomain("vopl");
    QCoreApplication::setApplicationName("nmgw-gate");

    gate::GuiTalk guiTalk;

    QSettings settings;

    guiTalk.setRendezvousHost(settings.value("rendezvousHost", "127.0.0.1").toString());
    guiTalk.setRendezvousPort(settings.value("rendezvousPort", "28938").toString());

    gate::RvzClient rvzClient;
    gate::socks5::Server socks5Server;

    auto startRvzClient = [&]
    {
        utils::asio2Worker()->post([&, host=guiTalk.getRendezvousHost().toStdString(), port=guiTalk.getRendezvousPort().toStdString()]
        {
            rvzClient.start(std::move(host), std::move(port));
        });
    };

    QObject::connect(&guiTalk, &gate::GuiTalk::applyStateRequested, [&]
    {
        settings.setValue("rendezvousHost", guiTalk.getRendezvousHost());
        settings.setValue("rendezvousPort", guiTalk.getRendezvousPort());
        startRvzClient();
    });

    rvzClient.onConnect([&]()
    {
        if (asio::error_code ec = asio2::get_last_error())
        {
            socks5Server.stop();
            QMetaObject::invokeMethod(&guiTalk, [&, txt=QString::fromLocal8Bit(ec.message())]{guiTalk.setRendezvousConnectivity(txt);});
            return;
        }

        socks5Server.start();
        QMetaObject::invokeMethod(&guiTalk, [&]{guiTalk.setRendezvousConnectivity("ok");});
    });

    rvzClient.onDisconnect([&]
    {
        socks5Server.stop();
        QMetaObject::invokeMethod(&guiTalk, [&]{guiTalk.setRendezvousConnectivity("none");});
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
    asio::signal_set signalset(utils::asio2Worker()->get_context());
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
    engine.rootContext()->setContextProperty("cppTalk", &guiTalk);
    engine.load("qrc:/src/gate/gui.qml");

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    int exitCode = app.exec();

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    rvzClient.stop();
    socks5Server.stop();
    asio::error_code ec;
    signalset.cancel(ec);

    utils::asio2Worker()->stop();
    LOGI("done");
    return exitCode;
}
