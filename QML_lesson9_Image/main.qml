import QtQuick 2.15
import QtQuick.Window 2.15

Window {
    width: 640
    height: 480
    visible: true
    title: qsTr("Hello World")

    property var middle: height / 2

    Image {
        id:localImage
        source: "qrc:/images/smile.png"
        fillMode: Image.PreserveAspectFit //  boyutları değiştirdikçe oranı bozulduğu için bunu kullandık yalnızca yükseklik veya genişlikle boyut ayarlayabilirsin.constructor
        width: 150
        x: 300
        y: middle - 100

    }
    Image {
        id:remoteImage
        source: "https://upload.wikimedia.org/wikipedia/commons/thumb/0/0b/Qt_logo_2016.svg/1280px-Qt_logo_2016.svg.png"
        fillMode: Image.PreserveAspectFit //  boyutları değiştirdikçe oranı bozulduğu için bunu kullandık yalnızca yükseklik veya genişlikle boyut ayarlayabilirsin.constructor
        width: 150
        x: 150
        y: middle - 100
        onProgressChanged: console.log(localImage.progress,"text is test ")
        onStatusChanged: if(remoteImage.status == Image.Ready) console.log("Remote image loaded")
    }
}
