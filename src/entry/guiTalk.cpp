#include "guiTalk.hpp"
#include <QCoreApplication>

namespace entry
{
    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    GuiTalk::GuiTalk()
    {
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    GuiTalk::~GuiTalk()
    {
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    void GuiTalk::closing()
    {
        QCoreApplication::quit();
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    QString GuiTalk::getRendezvousHost() const
    {
        return _rendezvousHost;
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    void GuiTalk::setRendezvousHost(const QString& value)
    {
        if(_rendezvousHost != value)
        {
            _rendezvousHost = value;
            rendezvousHostChanged();
        }
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    QString GuiTalk::getRendezvousPort() const
    {
        return _rendezvousPort;
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    void GuiTalk::setRendezvousPort(const QString& value)
    {
        if(_rendezvousPort != value)
        {
            _rendezvousPort = value;
            rendezvousPortChanged();
        }
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    QString GuiTalk::getRendezvousConnectivity() const
    {
        return _rendezvousConnectivity;
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    void GuiTalk::setRendezvousConnectivity(const QString& value)
    {
        if(_rendezvousConnectivity != value)
        {
            _rendezvousConnectivity = value;
            rendezvousConnectivityChanged();
        }
    }
}
