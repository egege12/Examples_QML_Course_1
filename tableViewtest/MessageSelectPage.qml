import QtQuick 2.15
import TableModel 0.1
import TableModel2 0.1
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.0

Rectangle {
    id: messageSelPage
    width : parent
    height: parent
    anchors.fill: parent
ColumnLayout{
    RowLayout{
        id:rowLayout
        Layout.alignment: {Qt.AlignTop}

        property string selectedMessage : "";

        spacing : 2
        Rectangle {
            id:messageRectangle
            Layout.preferredHeight: messageSelPage.width/3
            Layout.preferredWidth: messageSelPage.width/2
            Layout.alignment: Qt.AlignLeft
            onWidthChanged: {
                tableViewMessages.columnWidths[0] = messageRectangle.width*.4
                tableViewMessages.columnWidths[1] = messageRectangle.width*.2
                tableViewMessages.columnWidths[2] = messageRectangle.width*.2
                tableViewMessages.columnWidths[3] = messageRectangle.width*.2
                tableViewMessages.forceLayout();
            }

            TableView {
                id:tableViewMessages
                anchors.fill: parent
                columnSpacing: 1
                rowSpacing: 1
                clip: true

                model: TableModel{
                    id: tableMessages}
                Connections{
                    target: comObj
                    onInterfaceReady :tableMessages.setTable(comObj.messagesList())
                }
                Connections{
                    target: comObj
                    onSelectedStatChanged : tableMessages.setTable(comObj.messagesList())
                }
                property bool enableScrollbar: true
                ScrollBar.vertical: ScrollBar{
                    visible:  tableViewMessages.enableScrollbar  // not sure if you need to use enable or not
                }
                property var columnWidths: [220, 100, 80, 80]
                columnWidthProvider: function (column) { return columnWidths[column] }


                delegate: Rectangle {

                    implicitHeight: text.implicitHeight + 2
                    implicitWidth: text.implicitWidth +2
                    color: (heading==true)?"#2d2d2d": (rowLayout.selectedMessage == messageid )? "#EDEDE0":"#EDEDF0"

                    Text {
                        id:text
                        text: tabledata
                        width:tableMessages.width
                        padding: 1
                        font.pointSize: 10
                        Layout.alignment: Qt.AlignLeft
                        elide: Text.ElideRight
                        font.preferShaping: false
                        color: "#838383"
                    }
                    MouseArea{
                        anchors.fill: parent
                        onClicked: {
                            comObj.setDisplayReqSignal(messageid)
                            rowLayout.selectedMessage = messageid}
                        onDoubleClicked: {
                            comObj.setSelected(messageid)
                        }
                    }


                }


            }
        }
        /*Rectangle {
            id:midArea
            Layout.preferredHeight: 400
            Layout.preferredWidth: 70
            Layout.alignment: Qt.AlignCenter

        }*/
        Rectangle {
            id:signalRectangle
            Layout.preferredHeight: messageSelPage.width/3
            Layout.preferredWidth: messageSelPage.width/2
            Layout.alignment: Qt.AlignRight



            onWidthChanged: {
                tableViewSignals.columnWidths[0] = signalRectangle.width*.3
                tableViewSignals.columnWidths[1] = signalRectangle.width*.1
                tableViewSignals.columnWidths[2] = signalRectangle.width*.1
                tableViewSignals.columnWidths[3] = signalRectangle.width*.1
                tableViewSignals.columnWidths[4] = signalRectangle.width*.1
                tableViewSignals.columnWidths[5] = signalRectangle.width*.1
                tableViewSignals.columnWidths[6] = signalRectangle.width*.1
                tableViewSignals.columnWidths[7] = signalRectangle.width*.1
                tableViewSignals.forceLayout();
            }

            property string selectedSignalName : ""

            TableView {
                id:tableViewSignals
                anchors.fill: parent
                columnSpacing: 1
                rowSpacing: 1
                clip: true



                model: TableModelSignal{
                    id: tableSignals}
                Connections{
                    target: comObj
                    onSelectedViewChanged :tableSignals.setTable(comObj.signalsList())
                }


                property bool enableVScrollbar: true
                ScrollBar.vertical: ScrollBar{
                    visible:  tableViewSignals.enableVScrollbar  // not sure if you need to use enable or not
                }
                property bool enableHScrollbar: true
                ScrollBar.horizontal: ScrollBar{
                    visible:  tableViewSignals.enableHScrollbar  // not sure if you need to use enable or not
                }
                property var columnWidths: [220, 100, 80, 80,80,80,80,80]
                columnWidthProvider: function (column) { return columnWidths[column] }
                Timer {
                    running: true
                    interval: 1000
                    onTriggered: {
                        tableViewSignals.columnWidths[0] = signalRectangle.width*.3
                        tableViewSignals.columnWidths[1] = signalRectangle.width*.1
                        tableViewSignals.columnWidths[2] = signalRectangle.width*.1
                        tableViewSignals.columnWidths[3] = signalRectangle.width*.1
                        tableViewSignals.columnWidths[4] = signalRectangle.width*.1
                        tableViewSignals.columnWidths[5] = signalRectangle.width*.1
                        tableViewSignals.columnWidths[6] = signalRectangle.width*.1
                        tableViewSignals.columnWidths[7] = signalRectangle.width*.1
                        tableViewSignals.forceLayout();
                    }
                }

                delegate: Rectangle {

                    implicitHeight: textSignal.implicitHeight + 2
                    implicitWidth: textSignal.implicitWidth +2
                    color: (heading==true)?"#2d2d2d": (signalRectangle.selectedSignalName == messagename )? "#EDEDE0":"#EDEDF0"

                    Text {
                        id:textSignal
                        text: tabledata
                        width:tableSignals.width
                        padding: 1
                        font.pointSize: 10
                        elide: Text.ElideRight
                        font.preferShaping: false
                        color: "#838383"
                        Layout.alignment: Qt.AlignLeft
                    }
                    MouseArea {
                        anchors.fill: parent
                        onClicked: {signalRectangle.selectedSignalName = messagename}
                    }

                }


            }
        }
    }
    RowLayout{
        id:rowLayoutBottom
        Layout.alignment: {Qt.AlignBottom}
        Rectangle{
            id:areaConfig
            Layout.preferredHeight: messageSelPage.width/5
            Layout.preferredWidth: messageSelPage.width/2
            Layout.alignment: Qt.AlignLeft
            border.width: 1
        }
        Rectangle{
            id:areaGenerate
            Layout.preferredHeight: messageSelPage.width/5
            Layout.preferredWidth: messageSelPage.width/2
            Layout.alignment: Qt.AlignRight
            border.width: 1
        }

    }
}
}


