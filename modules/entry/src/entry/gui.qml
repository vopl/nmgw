import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Window {
    width: 411
    height: 731
    visible: true
    title: qsTr("nmgw entry")

    ColumnLayout {
        anchors.fill: parent
        spacing: 6

         GridLayout {
             Layout.fillWidth: true
             columns: 2
             Text { text: "this entry id:" }    TextField { id: entryId;        text: cppTalk.entryId;          color: cppTalk.entryId        == text ? palette.text : "darkred";    Layout.fillWidth: true;}
             Text { text: "renzvous IP:" }      TextField { id: rendezvousHost; text: cppTalk.rendezvousHost;   color: cppTalk.rendezvousHost == text ? palette.text : "darkred";    Layout.fillWidth: true;}
             Text { text: "port:" }             TextField { id: rendezvousPort; text: cppTalk.rendezvousPort;   color: cppTalk.rendezvousPort == text ? palette.text : "darkred";    Layout.fillWidth: true;}
             Text { text: "remote gate id:" }   TextField { id: gateId;         text: cppTalk.gateId;           color: cppTalk.gateId         == text ? palette.text : "darkred";    Layout.fillWidth: true;}
         }

         Button {
             Layout.fillWidth: true
             text: "apply"
             onClicked: {
                 cppTalk.entryId        = entryId.text
                 cppTalk.rendezvousHost = rendezvousHost.text
                 cppTalk.rendezvousPort = rendezvousPort.text
                 cppTalk.gateId         = gateId.text

                 cppTalk.applyStateRequested()
             }
         }

         GridLayout {
             Layout.fillWidth: true
             columns: 2
             Text { text: "rendezvous connectivity:" } Label { Layout.fillWidth: true; text: cppTalk.rendezvousConnectivity }
         }

         ListView {
             id: gatesList
             Layout.fillHeight: true

             flickableDirection: Flickable.AutoFlickDirection

             model: cppTalk.gateIds
             delegate: RowLayout {
                 Button {
                     text: "use";
                     implicitWidth: implicitContentWidth + leftPadding + rightPadding
                     onClicked: gateId.text = modelData
                 }
                 Text {
                     text: modelData;
                     color: gateId.text==modelData ? "darkgreen" : palette.text
                 }
             }
         }
    }

    Connections {
        target: cppTalk
        function onEntryIdChanged() {
            entryId.text = cppTalk.entryId
        }
        function onRendezvousHostChanged() {
            rendezvousHost.text = cppTalk.rendezvousHost
        }
        function onRendezvousPortChanged() {
            rendezvousPort.text = cppTalk.rendezvousPort
        }
        function onGateIdChanged() {
            gateId.text = cppTalk.gateId
        }
    }

    onClosing: cppTalk.closing()
}
