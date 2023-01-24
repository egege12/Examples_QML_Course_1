import QtQuick 2.15
import QtQuick.Window 2.15

Window {
    width: 640
    height: 480
    visible: true
    title: qsTr("Hello World")

    Image {
        id: name
        source :"https://www.udemy.com/staticx/udemy/images/v7/logo-udemy-inverted.svg "
    }

    TextInput {
        id: myInput
        text: "Hello World"
        anchors.centerIn: parent

    }
    Text {
        id: myText
        text: myInput.text
    }
}
