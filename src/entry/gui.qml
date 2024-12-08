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
             Text { text: "this entry id:" }    TextField { id: entryId;        Layout.fillWidth: true; text: cppTalk.entryId}
             Text { text: "renzvous IP:" }      TextField { id: rendezvousHost; Layout.fillWidth: true; text: cppTalk.rendezvousHost }
             Text { text: "port:" }             TextField { id: rendezvousPort; Layout.fillWidth: true; text: cppTalk.rendezvousPort }
             Text { text: "remote gate id:" }   TextField { id: gateId;         Layout.fillWidth: true; text: cppTalk.gateId}
         }

         Button {
             Layout.fillWidth: true
             text: "apply"
             onClicked: {
                 cppTalk.entryId = entryId.text
                 cppTalk.rendezvousHost = rendezvousHost.text
                 cppTalk.rendezvousPort = rendezvousPort.text
                 cppTalk.gateId = gateId.text
                 cppTalk.applyStateRequested()
             }
         }

         GridLayout {
             Layout.fillWidth: true
             columns: 2
             Text { text: "rendezvous connectivity:" } Label { Layout.fillWidth: true; text: cppTalk.rendezvousConnectivity }
             Text { text: "payload connections:" }   Label { Layout.fillWidth: true; text: "unknown" }
         }

         Label {
             Layout.fillHeight: true
         }
    }

    Connections {
        target: cppTalk
        function onRendezvousHostChanged() {
            rendezvousHost.text = cppTalk.rendezvousHost
        }
        function onRendezvousPortChanged() {
            rendezvousPort.text = cppTalk.rendezvousHost
        }
    }

    onClosing: cppTalk.closing()
}
