import QtQuick 2.15
import QtQuick.Controls 2.15
Rectangle {

    Popup {
            id: popup
            x: 100
            y: 100
            width: 200
            height: 300
            modal: true
            focus: true
            closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutsideParent
            contentItem: Text {
                    text: "Content"
                }

        }


}
