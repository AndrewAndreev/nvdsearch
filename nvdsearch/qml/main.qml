import QtQuick 2.10
import QtQuick.Controls 2.3
import QtQuick.Controls.Material 2.3

ApplicationWindow {
    visible: true

    Material.theme: Material.Light
    Material.accent: Material.Cyan

    Column {
        anchors.right: parent.right
        anchors.rightMargin: 16
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 16

        Button {
            Material.background: Material.Orange
            text: qsTr("Close")
            onClicked: Qt.quit();
        }
    }
}
