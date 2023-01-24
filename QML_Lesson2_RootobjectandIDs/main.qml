import QtQuick 2.15
import QtQuick.Window 2.15

//ID - benzersiz olmalı
// Root Object - Sadece bir tane kök obje olabilir.

// Bu obje kök objedir. En tepede ve bir tane olabilir.
Window {
    width: 640
    height: 480
    visible: true
    title: qsTr("Hello World")
    id:root  // yalnızca bir tane

    Image {
        id : myimage
        source: "https://www.udemy.com/staticx/udemy/images/v7/logo-udemy.svg"
        width: 150
        height: 100
        anchors.centerIn: parent
        Rectangle {
            color : "red"
            width: parent.width   // bunun parent'i image olur. myimage'a bağlanır.
            height: parent.height
            opacity: 0.5
        }
    }
}
