#pragma once
#include <QObject>

namespace endpoint
{
    class GuiGate
        : public QObject
    {
        Q_OBJECT
        Q_PROPERTY(QString rendezvousHost         READ getRendezvousHost         WRITE setRendezvousHost         NOTIFY rendezvousHostChanged         FINAL)
        Q_PROPERTY(QString rendezvousPort         READ getRendezvousPort         WRITE setRendezvousPort         NOTIFY rendezvousPortChanged         FINAL)
        Q_PROPERTY(QString rendezvousConnectivity READ getRendezvousConnectivity WRITE setRendezvousConnectivity NOTIFY rendezvousConnectivityChanged FINAL)

    public:
        GuiGate();
        ~GuiGate();

    public:
        Q_INVOKABLE void closing();

    public:
        QString getRendezvousHost() const;
        void setRendezvousHost(const QString& value);

        QString getRendezvousPort() const;
        void setRendezvousPort(const QString& value);

        QString getRendezvousConnectivity() const;
        void setRendezvousConnectivity(const QString& value);

    signals:
        void rendezvousHostChanged();
        void rendezvousPortChanged();
        void rendezvousConnectivityChanged();


    private:
        QString _rendezvousHost;
        QString _rendezvousPort;
        QString _rendezvousConnectivity;
    };
}
