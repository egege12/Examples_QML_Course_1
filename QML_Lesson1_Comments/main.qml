import QtQuick 2.15
import QtQuick.Window 2.15

Window {
    width: 1080
    height: 1080
    visible: true
    title: qsTr("Hello World")

    /*
     We are going to make this
*/

    Image {
        //Her obje bir id'ye sahiptir.
        id     : myimage
        //Dosyayı kaynaktan kendisi indirebilir.
        source : "https://www.udemy.com/staticx/udemy/images/v7/logo-udemy.svg"
        //Çapa manasına gelen anchor sabitlemek için kullanılır.
            /*Methodları : left right centerIn bottom top
                    */
        anchors.centerIn: parent
    }
}
