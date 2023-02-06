import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Controls 2.0
import Monty 1.0
Window {
    width: 640
    height: 480
    visible: true
    title: qsTr("Hello World")

    SomeClass {
        id : myClass
    }

    Button {
        id : myButton
        anchors.centerIn : parent
        text: "Call Me"
        onClicked: myClass.callMe();
    }

}
