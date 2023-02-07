import QtQuick 2.15
import TableModel 0.1
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.0
Rectangle {
    width : parent.width*.45
    height: parent.height
    anchors.fill: parent
    RowLayout{
        spacing : 2
        Rectangle {

            Layout.preferredHeight: 400
            Layout.preferredWidth: 500
            border.width: 1
            Layout.alignment: Qt.AlignLeft
            TableView {
                id:tableView
                anchors.fill: parent
                columnSpacing: 1
                rowSpacing: 1
                clip: true

                model: TableModel {
                    id:tableMessages

                }
                Connections{
                    target: comObj
                    onInterfaceReady :tableMessages.setTable(comObj.messagesList())
                }

                property bool enableScrollbar: true
                ScrollBar.vertical: ScrollBar{
                        visible:  tableView.enableScrollbar  // not sure if you need to use enable or not
                    }
                property var columnWidths: [220, 100, 80, 80]
                    columnWidthProvider: function (column) { return columnWidths[column] }
                    Timer {
                        running: true
                        interval: 2000
                        onTriggered: {
                            tableView.columnWidths[0] = 220
                            tableView.forceLayout();
                        }
                    }

                delegate: Rectangle {

                    implicitHeight: text.implicitHeight + 2
                    implicitWidth: text.implicitWidth +2
                    border.width: 1
                    border.color: "white"
                    color: (heading==true)?"#2d2d2d": "#EDEDF0"
                    Text {
                        id:text
                        text: tabledata
                        width:tableMessages.width
                        padding: 1
                        font.pointSize: 10
                        anchors.centerIn:parent
                        elide: Text.ElideRight
                        font.preferShaping: false
                        color: "#838383"
                    }

                }

            }
        }
        Rectangle {
            id:midArea
            Layout.preferredHeight: 400
            Layout.preferredWidth: 80
            border.width: 1
            Layout.alignment: Qt.AlignCenter

        }
        Rectangle {
            Layout.preferredHeight: 400
            Layout.preferredWidth: 500
            border.width: 1
            Layout.alignment: Qt.AlignRight
        }
    }

}

