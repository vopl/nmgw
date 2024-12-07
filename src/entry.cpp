#include <logger.hpp>
#include <filesystem>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QSettings>
#include "entry/rvzClient.hpp"
#include "entry/socks5/server.hpp"
#include "entry/guiTalk.hpp"
#include "entry/worker.hpp"
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

    entry::worker()->start();

    QGuiApplication app(argc, argv);
    QCoreApplication::setOrganizationName("vopl");
    QCoreApplication::setOrganizationDomain("vopl");
    QCoreApplication::setApplicationName("nmgw");

    entry::GuiTalk guiTalk;

    QSettings settings;

    guiTalk.setRendezvousHost(settings.value("rendezvousHost", "127.0.0.1").toString());
    guiTalk.setRendezvousPort(settings.value("rendezvousPort", "28938").toString());

    entry::RvzClient rvzClient;
    entry::socks5::Server socks5Server;
    socks5Server.setRvzClient(&rvzClient);

    auto startRvzClient = [&]
    {
        rvzClient.start(
                    guiTalk.getRendezvousHost().toStdString(),
                    guiTalk.getRendezvousPort().toStdString());
    };

    auto onRendezvousHostPortChanged = [&]
    {
        settings.setValue("rendezvousHost", guiTalk.getRendezvousHost());
        settings.setValue("rendezvousPort", guiTalk.getRendezvousPort());
        startRvzClient();
    };

    QObject::connect(&guiTalk, &entry::GuiTalk::rendezvousHostChanged, onRendezvousHostPortChanged);
    QObject::connect(&guiTalk, &entry::GuiTalk::rendezvousPortChanged, onRendezvousHostPortChanged);


    rvzClient.subscribeOnConnect([&]()
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

    rvzClient.subscribeOnDisconnect([&]
    {
        socks5Server.stop();
        QMetaObject::invokeMethod(&guiTalk, [&]{guiTalk.setRendezvousConnectivity("none");});
    });

    startRvzClient();

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    asio::signal_set signalset(entry::worker()->get_context());
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
    engine.load("qrc:/src/entry/gui.qml");

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    int exitCode = app.exec();

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    rvzClient.stop();
    socks5Server.stop();
    asio::error_code ec;
    signalset.cancel(ec);


    entry::worker()->stop();
    LOGI("done");
    return exitCode;
}
