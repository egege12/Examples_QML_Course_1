import QtQuick 2.15
import QtQuick.Window 2.15
//non-visual item class

// https://doc.qt.io/qt-6/qml-qtquick-item.html
Window {
    width: 640
    height: 480
    visible: true
    title: qsTr("Hello World")

//Burada kendi objemizi tanımladık ve içine başka objeler tanımlayabildik.
    Item {
        id: myItem
        anchors.centerIn: parent
        width: 100
        height: 100

        Rectangle{
            color : "blue"
            anchors.fill:parent
            opacity: 0.5
        }
        Rectangle{
            color : "red"
            anchors.fill: parent
            opacity: 0.5
        }
    }
}
