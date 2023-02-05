import QtQuick 2.15
import QtQuick.Window 2.15

Window {
    width: 640
    height: 480
    visible: true
    title: qsTr("Hello World")


    Rectangle {
        id: myItem
        anchors.centerIn: parent
        color : "#aa0000"
        width: 200
        height: 200
        visible: true

        // Kenarlık ayarlaması için
        border.color: "#000000"
        border.width: 5

        // radius genişliğe eşitken tam bir daire olur
        radius: width

        // gradient adında bir özellik içinde  gradient adında bir child oluşturduk
        gradient: Gradient{
            GradientStop{ position:0.5;color: "#ff00AA"}
            GradientStop{ position:1.0;color: "#ffAA00"}
        }

    }
}
