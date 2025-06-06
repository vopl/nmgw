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
    QString GuiTalk::getEntryId() const
    {
        return _entryId;
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    void GuiTalk::setEntryId(const QString& value)
    {
        if(_entryId != value)
        {
            _entryId = value;
            entryIdChanged();
        }
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    QString GuiTalk::getGateId() const
    {
        return _gateId;
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    void GuiTalk::setGateId(const QString& value)
    {
        if(_gateId != value)
        {
            _gateId = value;
            gateIdChanged();
        }
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

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    QStringList GuiTalk::getGateIds() const
    {
        return _gateIds;
    }

    /////////0/////////1/////////2/////////3/////////4/////////5/////////6/////////7
    void GuiTalk::setGateIds(const QStringList& value)
    {
        if(_gateIds != value)
        {
            _gateIds = value;
            gateIdsChanged();
        }
    }
}
