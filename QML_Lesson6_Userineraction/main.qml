
import QtQuick 2.15
import QtQuick.Window 2.15

Window {
    width: 640
    height: 480
    visible: true
    title: qsTr("Hello World")


    Rectangle {
        TapHandler{
            id:inputHandler
        }
        anchors.centerIn: parent
        width: 100
        height: 100
        color: inputHandler.pressed ? "red": "blue"
        visible: inputHandler.pressed ? false: true
        // Handler'ı kurduk sonrasında yukardaki şekilde bir koşul oluşturduk.

    }
}
