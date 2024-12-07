import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Window {
    width: 411
    height: 731
    visible: true
    title: qsTr("nmgw endpoint")

    ColumnLayout {
        anchors.fill: parent
        spacing: 6

         GridLayout {
             Layout.fillWidth: true
             columns: 2
             Text { text: "renzvous IP:" } TextField { id: rendezvousHost; Layout.fillWidth: true; text: cppGate.rendezvousHost }
             Text { text: "port:" }        TextField { id: rendezvousPort; Layout.fillWidth: true; text: cppGate.rendezvousPort }
         }

         Button {
             Layout.fillWidth: true
             text: "apply"
             onClicked: {
                 cppGate.rendezvousHost = rendezvousHost.text
                 cppGate.rendezvousPort = rendezvousPort.text
             }
         }

         GridLayout {
             Layout.fillWidth: true
             columns: 2
             Text { text: "rendezvous connectivity:" } Label { Layout.fillWidth: true; text: cppGate.rendezvousConnectivity }
             Text { text: "payload connections:" }   Label { Layout.fillWidth: true; text: "unknown" }
         }

         Label {
             Layout.fillHeight: true
         }
    }

    Connections {
        target: cppGate
        function onRendezvousHostChanged() {
            rendezvousHost.text = cppGate.rendezvousHost
        }
        function onRendezvousPortChanged() {
            rendezvousPort.text = cppGate.rendezvousHost
        }
    }

    onClosing: cppGate.closing()
}
