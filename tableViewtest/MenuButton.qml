import QtQuick 2.15

Rectangle {
    signal buttonClicked()
    signal buttonReleased()
    property string buttonImageSource: ""
    property bool disableButtonClick
    height: width
    radius: width/8
    color :"#707070"
    Image{
        width: parent.width*.6
        height: parent.height*.6
        anchors.centerIn: parent
        antialiasing: true
        source: parent.buttonImageSource
        fillMode:Image.PreserveAspectFit
    }
    MouseArea{
        anchors.fill: parent
        onPressed: parent.color = parent.disableButtonClick ? "#707070" : "#575757"
        onReleased: {
            parent.color = "#707070"
            parent.disableButtonClick ? undefined : parent.buttonReleased()
        }
        onClicked: parent.disableButtonClick ? undefined: parent.buttonClicked()
    }
}
