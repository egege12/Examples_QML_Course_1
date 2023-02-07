import QtQuick 2.15
import TableModel 0.1
import QtQuick.Controls 2.2
Rectangle {
    width : parent.width
    height: parent.height
    anchors.fill: parent
    TableView {
        anchors.fill: parent
        columnSpacing: 1
        rowSpacing: 1
        clip: true

        model: TableModel {
            id:tableMessages

        }

        delegate: Rectangle {
            implicitWidth: 100
            implicitHeight: 50

            Text {
                text: tabledata
                font.pointSize: 12
                anchors.centerIn:parent

            }
        }

    }
    Button {
        onClicked: tableMessages.setTable(comObj.messagesList())
    }



}
