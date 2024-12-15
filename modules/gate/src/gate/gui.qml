import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Window {
    width: 411
    height: 731
    visible: true
    title: qsTr("nmgw gate")

    ColumnLayout {
        anchors.fill: parent
        spacing: 6

         GridLayout {
             Layout.fillWidth: true
             columns: 2
             Text { text: "this gate id:" }     TextField { id: gateId;         text: cppTalk.gateId;           color: cppTalk.gateId         == text ? palette.text : "darkred";    Layout.fillWidth: true;}
             Text { text: "renzvous IP:" }      TextField { id: rendezvousHost; text: cppTalk.rendezvousHost;   color: cppTalk.rendezvousHost == text ? palette.text : "darkred";    Layout.fillWidth: true;}
             Text { text: "port:" }             TextField { id: rendezvousPort; text: cppTalk.rendezvousPort;   color: cppTalk.rendezvousPort == text ? palette.text : "darkred";    Layout.fillWidth: true;}
         }

         Button {
             Layout.fillWidth: true
             text: "apply"
             onClicked: {
                 cppTalk.gateId         = gateId.text
                 cppTalk.rendezvousHost = rendezvousHost.text
                 cppTalk.rendezvousPort = rendezvousPort.text

                 cppTalk.applyStateRequested()
             }
         }

         GridLayout {
             Layout.fillWidth: true
             columns: 2
             Text { text: "rendezvous connectivity:" } Label { Layout.fillWidth: true; text: cppTalk.rendezvousConnectivity }
         }

         Label {
             Layout.fillHeight: true
         }
    }

    Connections {
        target: cppTalk
        function onGateIdChanged() {
            gateId.text = cppTalk.gateId
        }
        function onRendezvousHostChanged() {
            rendezvousHost.text = cppTalk.rendezvousHost
        }
        function onRendezvousPortChanged() {
            rendezvousPort.text = cppTalk.rendezvousPort
        }
    }

    onClosing: cppTalk.closing()
}
