#pragma once
#include <QObject>

namespace gate
{
    class GuiTalk
        : public QObject
    {
        Q_OBJECT
        Q_PROPERTY(QString gateId                 READ getGateId                 WRITE setGateId                 NOTIFY  gateIdChanged                FINAL)
        Q_PROPERTY(QString rendezvousHost         READ getRendezvousHost         WRITE setRendezvousHost         NOTIFY rendezvousHostChanged         FINAL)
        Q_PROPERTY(QString rendezvousPort         READ getRendezvousPort         WRITE setRendezvousPort         NOTIFY rendezvousPortChanged         FINAL)
        Q_PROPERTY(QString rendezvousConnectivity READ getRendezvousConnectivity WRITE setRendezvousConnectivity NOTIFY rendezvousConnectivityChanged FINAL)

    public:
        GuiTalk();
        ~GuiTalk();

    public:
        Q_INVOKABLE void closing();

    public:
        QString getGateId() const;
        void setGateId(const QString& value);

        QString getRendezvousHost() const;
        void setRendezvousHost(const QString& value);

        QString getRendezvousPort() const;
        void setRendezvousPort(const QString& value);

        QString getRendezvousConnectivity() const;
        void setRendezvousConnectivity(const QString& value);

    signals:
        void applyStateRequested();
        void gateIdChanged();
        void rendezvousHostChanged();
        void rendezvousPortChanged();
        void rendezvousConnectivityChanged();

    private:
        QString _gateId;
        QString _rendezvousHost;
        QString _rendezvousPort;
        QString _rendezvousConnectivity;
    };
}
