#include "guiGate.hpp"

namespace endpoint
{
    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    GuiGate::GuiGate()
    {
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    GuiGate::~GuiGate()
    {
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    QString GuiGate::getRendezvousHost() const
    {
        return _rendezvousHost;
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    void GuiGate::setRendezvousHost(const QString& value)
    {
        if(_rendezvousHost != value)
        {
            _rendezvousHost = value;
            rendezvousHostChanged();
        }
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    QString GuiGate::getRendezvousPort() const
    {
        return _rendezvousPort;
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    void GuiGate::setRendezvousPort(const QString& value)
    {
        if(_rendezvousPort != value)
        {
            _rendezvousPort = value;
            rendezvousPortChanged();
        }
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    QString GuiGate::getRendezvousConnectivity() const
    {
        return _rendezvousConnectivity;
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    void GuiGate::setRendezvousConnectivity(const QString& value)
    {
        if(_rendezvousConnectivity != value)
        {
            _rendezvousConnectivity = value;
            rendezvousConnectivityChanged();
        }
    }
}
