
import QtQuick 2.15
import QtQuick.Window 2.15

Window {
    width: 640
    height: 480
    visible: true
    title: qsTr("Hello World")

    Text {
        id: myText
        text: qsTr("<html><b>Hello</b> <i><font color='green'> World</font><i/></html>
                    <br><font color='#ff0023'> I am Egemen</font>
                    <br><a href='http//www.google.com'> google </a>")
        anchors.centerIn: parent

        font.pointSize:  35  // font büyüklüğünü ayarlar
       // font.bold: false  // çalışmaz çünkü htmlde bu özelliği aktif ettik.
        //font.italic: false
        color : "red"
        linkColor: "green"

        onLinkHovered:{
            console.log("Hover: " + link)
            if(link){
                myText.font.bold=true;
            }else{
                myText.font.bold=false;
            }

        }
        onLinkActivated: Qt.openUrlExternally(link)
    }
}
