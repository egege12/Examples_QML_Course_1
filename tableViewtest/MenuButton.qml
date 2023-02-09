import QtQuick 2.15

Rectangle {
    signal buttonClicked()
    signal buttonReleased()
    property string buttonImageSource: ""
    property bool disableButtonClick
    property color disabledColor : "#9e9b9b"
    property color pressedColor : "#575757"
    property color releasedColor : "#707070"
    height: width
    radius: width/8
    onDisableButtonClickChanged: disableButtonClick ? disabledColor : releasedColor
    color :disableButtonClick ? disabledColor : releasedColor
    Image{
        width: parent.width*.6
        height: parent.height*.6
        anchors.centerIn: parent
        antialiasing: true
        source: parent.buttonImageSource
        opacity: parent.disableButtonClick ? 0.5 : 1.0
        fillMode:Image.PreserveAspectFit
    }
    MouseArea{
        anchors.fill: parent
        onPressed: parent.color = parent.disableButtonClick ? parent.disabledColor : parent.pressedColor
        onReleased: {
            parent.color = parent.disableButtonClick ? parent.disabledColor : parent.releasedColor
            parent.disableButtonClick ? undefined : parent.buttonReleased()
        }
        onClicked: parent.disableButtonClick ? undefined: parent.buttonClicked()
    }
}
