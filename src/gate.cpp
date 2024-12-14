#include <logger.hpp>
#include <filesystem>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QSettings>
#include <QRandomGenerator>
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
            LOGE("unable to determine current directory " << ec);
            return EXIT_FAILURE;
        }

        fs::path executableDir = executablePath.parent_path();
        fs::current_path(executableDir, ec);
        if(ec)
        {
            LOGE("unable to set current directory to " << executableDir << " " << ec);
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

    if(settings.value("gateId").isNull())
    {
        QRandomGenerator gen{QRandomGenerator::securelySeeded()};
        std::string value;
        for(std::size_t i{}; i<32; ++i)
            value += (char)gen.bounded('a', 'z');
        settings.setValue("gateId", QString::fromLocal8Bit(value));
    }
    if(settings.value("rendezvousHost").isNull())
        settings.setValue("rendezvousHost", "185.185.40.120");
    if(settings.value("rendezvousPort").isNull())
        settings.setValue("rendezvousPort", "28938");

    guiTalk.setGateId(settings.value("gateId").toString());
    guiTalk.setRendezvousHost(settings.value("rendezvousHost").toString());
    guiTalk.setRendezvousPort(settings.value("rendezvousPort").toString());

    gate::RvzClient rvzClient;
    gate::socks5::Server socks5Server;

    auto actualizeRvzClient = [&]
    {
        utils::asio2Worker()->post([&,
                                   host=settings.value("rendezvousHost").toString().toStdString(),
                                   port=settings.value("rendezvousPort").toString().toStdString(),
                                   gateId=settings.value("gateId").toString().toStdString()]
        {
            rvzClient.actualizeGate(common::GateId{gateId});
            rvzClient.actualizeRendezvous(std::move(host), std::move(port));
        });
    };
    actualizeRvzClient();

    QObject::connect(&guiTalk, &gate::GuiTalk::applyStateRequested, [&]
    {
        settings.setValue("gateId", guiTalk.getGateId());
        settings.setValue("rendezvousHost", guiTalk.getRendezvousHost());
        settings.setValue("rendezvousPort", guiTalk.getRendezvousPort());
        actualizeRvzClient();
    });

    rvzClient.subscribeOnConnect([&](asio::error_code ec)
    {
        if (ec)
        {
            socks5Server.stop();
            QMetaObject::invokeMethod(&guiTalk, [&, txt=QString::fromLocal8Bit(ec.message())]{guiTalk.setRendezvousConnectivity(txt);});
            return;
        }

        socks5Server.start();
        QMetaObject::invokeMethod(&guiTalk, [&]{guiTalk.setRendezvousConnectivity("ok");});
    });

    rvzClient.subscribeOnDisconnect([&](asio::error_code /*ec*/)
    {
        socks5Server.stop();
        QMetaObject::invokeMethod(&guiTalk, [&]{guiTalk.setRendezvousConnectivity("none");});
    });

    rvzClient.subscribeOnSocks5Open([&](common::Socks5Id socks5Id)
    {
        socks5Server.open(socks5Id);
    });

    rvzClient.subscribeOnSocks5Closed([&](common::Socks5Id socks5Id)
    {
        socks5Server.close(socks5Id);
    });

    rvzClient.subscribeOnSocks5Input([&](common::Socks5Id socks5Id, std::string data)
    {
        if(!socks5Server.output(socks5Id, std::move(data)))
            rvzClient.socks5Close(socks5Id);
    });

    socks5Server.subscribeOnInput([&](common::Socks5Id socks5Id, std::string data)
    {
        rvzClient.socks5Output(socks5Id, std::move(data));
    });

    socks5Server.subscribeOnClosed([&](common::Socks5Id socks5Id)
    {
        rvzClient.socks5Close(socks5Id);
    });

    rvzClient.start();

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    asio::signal_set signalset(utils::asio2Worker()->get_context());
    signalset.add(SIGINT);
    signalset.add(SIGTERM);
    signalset.async_wait([&](const asio::error_code& ec, int signo)
    {
        if (ec)
            return;
        LOGI("signal " << strsignal(signo) << " " << ec);
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
