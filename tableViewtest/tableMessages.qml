import QtQuick 2.15
import TableModel 0.1
Rectangle {
    TableView {
         anchors.fill: parent
         columnSpacing: 1
         rowSpacing: 1
         clip: true

         model: TableModel {}

         delegate: Rectangle {
             implicitWidth: 100
             implicitHeight: 50
             Text {
                 text: display
             }
         }
     }
}


