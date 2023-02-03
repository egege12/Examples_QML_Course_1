import QtQuick 2.15
import QtQuick.Window 2.15
import TableModel 0.1
Window {
    width: 640
    height: 480
    visible: true
    title: qsTr("Hello World")

    TableView {
        anchors.fill: parent
        columnSpacing: 1
        rowSpacing: 1
        clip: true

        model: TableModel {}

        delegate: Rectangle {
            implicitWidth: 100
            implicitHeight: 50
            border.color: "black"
            border.width: 0.5

            Text {
                text: display
                font.pointSize: 12
                anchors.centerIn: parent
            }
        }
    }

}
