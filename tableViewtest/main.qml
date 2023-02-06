import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Controls 2.0

Window {
    id: root
    width: 640
    height: 480
    visible: true
    title: qsTr("Interface Creator")


    StackView {
        id: stack
        anchors.fill:parent
        initialItem: "fileDialogPage.qml"

    }
    Connections {
        target : comObj
        onErrCode: {popup.open()}
    }

}
