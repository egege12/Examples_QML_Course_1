import QtQuick 2.15
import QtQuick.Window 2.15
import TableModel 0.1
Window {
    width: 840
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
            implicitWidth: 200
            implicitHeight: 20
            border.color: "black"
            border.width: 0.5
            color : heading ? "gray" : "white";
            TableView.onPooled:console.log(tabledata + "pool");
            TableView.onReused: console.log(tabledata + "reused");
            Text {
                text: tabledata
                font.pointSize: 12
                anchors.centerIn: parent
            }
        }
    }

}
