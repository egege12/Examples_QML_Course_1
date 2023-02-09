import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Controls 2.0

Window {
    id: root
    width: 1080
    height: 580
    visible: true
    title: qsTr("Interface Creator")



    StackView {
        id: stack
        anchors.fill:parent
        initialItem: "fileDialogPage.qml"

    }


}
