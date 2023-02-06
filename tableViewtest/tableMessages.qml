import QtQuick 2.15
import TableModel 0.1
Item {
    Rectangle{

        id:background
        anchors.fill: parent
        TableView {
            anchors.fill: parent
            columnSpacing: 1
            rowSpacing: 1
            clip: true

            model: TableModel {

            }

            Connections{
                target: interfaceObj.interfaceReady()
                onClicked: TableModel.generateTable()
            }

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
}
