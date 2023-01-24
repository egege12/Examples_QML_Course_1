
import QtQuick 2.15
import QtQuick.Window 2.15


// Parent and child relationships

Window {
    width: 640
    height: 480
    visible: true
    title: qsTr("Hello World")


    Image{
        id: image
        source:"https://upload.wikimedia.org/wikipedia/commons/thumb/0/0b/Qt_logo_2016.svg/1280px-Qt_logo_2016.svg.png"
        width: 150
        height: 100
        x:150
        y:150
        z:3

        Rectangle {
            id: rectangle1
            color: "blue"
            x: 0
            y: 0
            width: 50
            height: 50
            opacity:0.5
        }
        Rectangle {
            id: rectangle2
            color: "red"
            x: parent.width- width
            y: parent.height-height
            width: 50
            height: 50
            opacity:0.5
        }
    }

}
