import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Dialogs
Item {
    Rectangle {
        id: background

        anchors.fill : parent

        Rectangle {
            gradient:Gradient{
                GradientStop {position: 0.0; color : "#336BD6"}
                GradientStop {position: 1.0; color : "#194773"}
            }
            width : parent.width
            height: parent.height/2
            anchors.top:  parent.top
            Image {
                id: logo
                source: "qrc:/img/img/dbcGeneratorLogo.png"
                width: parent.width*.95
                height: parent.height*.95
                anchors.centerIn: parent
                fillMode:Image.PreserveAspectFit
            }
        }

        Rectangle {
            gradient:Gradient{
                GradientStop {position: 0.0; color : "#194773"}
                GradientStop {position: 1.0; color : "#1947AA"}
            }
            width : parent.width
            height: parent.height/2
            anchors.bottom: parent.bottom
            FileDialog {
                id: fileDialogSaveLoc
                title: "Please choose a DBC file"
                selectedNameFilter.index: 1
                onAccepted: {

                }
                onRejected:{
                    locNameTextField.text = "File is not selected"
                }
                Component.onCompleted: visible=false;
            }

            MenuButton {
                id : buttonSearchSaveLoc
                buttonImageSource : "qrc:/img/img/iconSearchFile.png"
                x : parent.width * 0.8
                width: parent.width * .15
                height: parent.height *.15
                onButtonClicked:{
                    fileDialogSaveLoc.open();
                }
                TextField {
                    id: locNameTextField
                    height: parent.height
                    width: parent.width *5
                    anchors{
                        right: parent.left
                        rightMargin: 5
                    }
                }
            }

            FileDialog {
                id: fileDialog
                title: "Please choose a DBC file"
                nameFilters: ["DBC files (*.dbc)"]
                onAccepted: {
                    var path = fileDialog.currentFile.toString();
                    // remove prefixed "file:///"
                    path= path.replace(/^(file:\/{3})|(qrc:\/{2})|(http:\/{2})/,"");
                    // unescape html codes like '%23' for '#'
                    var cleanPath = decodeURIComponent(path);
                    fileNameTextField.text = cleanPath
                }
                onRejected:{
                    fileNameTextField.text = "Dosya seçilmedi"

                }
                Component.onCompleted: visible=false;
            }

            MenuButton {
                id : buttonSearch
                buttonImageSource : "qrc:/img/img/iconSearchFile.png"
                x : parent.width * 0.8
                y : parent.height *.40
                width: parent.width * .15
                height: parent.height *.15

                onButtonClicked:{
                    fileDialog.open();
                }
                TextField {
                    id: fileNameTextField
                    height: parent.height
                    width: parent.width *5
                    anchors{
                        right: parent.left
                        rightMargin: 5

                    }

                }
            }

            MenuButton {
                id : buttonToPage1
                anchors{
                    bottom: parent.bottom
                    bottomMargin: parent.height * 0.1
                    left:parent.left
                    leftMargin: parent.width/2 - width/2
                }
                buttonImageSource : "qrc:/img/img/arrowRight.png"
                radius: width
                width: parent.width * .1
                height: width
                onButtonClicked: comObj.readFile(fileNameTextField.text)
                onButtonReleased: stack.push("MessageSelectPage.qml")
            }
        }
    }
}


