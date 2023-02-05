import QtQuick 2.15
import QtQuick.Window 2.15


// X VE Y koordinat sistemine göre objeyi başlatır. x y kordinatları bulunduğu scope objesinin başladığı
//adresten itibaren başlar. Yeni aşağıdaki şekilleri resme koysaydık artık windowun değil resmin
// 50 50sinden 150 150sinden başlardı.

//Z ekseni önceliktir. Büyük z numarası objeyi öne getirir.
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

    }

    Rectangle {
        id : rectangle1
        color: "red"
        width: 200
        height: 200
        x: 50
        y: 50
        z: 2
        opacity :0.5

    }

    Rectangle {
        id : rectangle2
        color: "blue"
        width: 200
        height: 200
        x: 150
        y: 150
        z: 1
        opacity :0.5

    }



}
