import QtQuick 2.15

Rectangle {
    signal buttonClicked()
    signal buttonReleased()
    property string buttonImageSource: ""
    height: width
    radius: width/8
    color :"#707070"
    Image{
        width: parent.width*.6
        height: parent.height*.6
        anchors.centerIn: parent

        source: parent.buttonImageSource
        fillMode:Image.PreserveAspectFit
    }
    MouseArea{
        anchors.fill:parent
        onPressed: parent.color = "#575757"
        onReleased: {
            parent.color = "#707070"
            parent.buttonReleased()
        }
        onClicked: parent.buttonClicked()
    }
}
