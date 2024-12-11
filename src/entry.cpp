#include <logger.hpp>
#include <filesystem>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QSettings>
#include <QRandomGenerator>
#include "entry/rvzClient.hpp"
#include "entry/socks5/server.hpp"
#include "entry/guiTalk.hpp"
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
    QCoreApplication::setApplicationName("nmgw-entry");

    entry::GuiTalk guiTalk;

    QSettings settings;

    if(settings.value("entryId").isNull())
    {
        QRandomGenerator gen{QRandomGenerator::securelySeeded()};
        std::string value;
        for(std::size_t i{}; i<32; ++i)
            value += (char)gen.bounded('a', 'z');
        settings.setValue("entryId", QString::fromLocal8Bit(value));
    }
    if(settings.value("rendezvousHost").isNull())
        settings.setValue("rendezvousHost", "127.0.0.1");
    if(settings.value("rendezvousPort").isNull())
        settings.setValue("rendezvousPort", "28938");

    guiTalk.setEntryId(settings.value("entryId").toString());
    guiTalk.setGateId(settings.value("gateId").toString());
    guiTalk.setRendezvousHost(settings.value("rendezvousHost").toString());
    guiTalk.setRendezvousPort(settings.value("rendezvousPort").toString());

    entry::RvzClient rvzClient;
    entry::socks5::Server socks5Server;
    socks5Server.setRvzClient(&rvzClient);

    auto actualizeRvzClient = [&]
    {
        utils::asio2Worker()->post([&,
                                   host=settings.value("rendezvousHost").toString().toStdString(),
                                   port=settings.value("rendezvousPort").toString().toStdString(),
                                   entryId=settings.value("entryId").toString().toStdString(),
                                   gateId=settings.value("gateId").toString().toStdString()]
        {
            rvzClient.actualizeEntry(common::EntryId{entryId});
            rvzClient.actualizeRendezvous(std::move(host), std::move(port));
            socks5Server.setGateId(common::GateId{gateId});
        });
    };
    actualizeRvzClient();

    QObject::connect(&guiTalk, &entry::GuiTalk::applyStateRequested, [&]
    {
        settings.setValue("entryId", guiTalk.getEntryId());
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

    rvzClient.subscribeOnGateList([&](std::vector<common::GateId> gateIds_)
    {
        QStringList gateIds;
        for(const common::GateId& gateId : gateIds_)
            gateIds.emplaceBack(QString::fromStdString(gateId._value));

        QMetaObject::invokeMethod(&guiTalk, [&, gateIds]{guiTalk.setGateIds(gateIds);});
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
#ifdef ANDROID
        auto SIG = signo >= _NSIG ? "UNKNOWN" : sys_siglist[signo];
#else
        auto SIG = sigabbrev_np(signo);
#endif
        LOGI("SIG" << SIG << " " << ec);
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

    utils::asio2Worker()->stop();
    LOGI("done");
    return exitCode;
}
