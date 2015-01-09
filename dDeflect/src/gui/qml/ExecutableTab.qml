import QtQuick 2.3
import QtQuick.Controls 1.2
import QtQuick.Controls.Styles 1.2
import QtQuick.Layouts 1.1
import QtQuick.Dialogs 1.2
import jsk.components 1.0

Component{
    id: execTab

    Tab {
        title: "Executable
"
        visible:true
        ColumnLayout{
            id:cl
            anchors.fill: parent
            //anchors.margins: 12
            anchors.topMargin: addButton.height/2
            spacing: 10

            Button{
                id: addButton
                Layout.alignment: Qt.AlignLeft
                text:"+"
                onClicked: {
                    fruitModel.append({"name":"kuba"})
                }
            }
            ListView {
                id: lv
                visible: true
                Layout.alignment: Qt.AlignLeft
                //                width: execTab.width
                //                height: 230

                anchors.fill: parent
                anchors.topMargin: addButton.height*3/2+lv.spacing
                model: fruitModel
                delegate: contactDelegate

            }
            ListModel {
                id: fruitModel
            }
            Component {
                id: contactDelegate
                Item{

                    //RowLayout{
                    height: 50
                    width: parent.width
                    ComboBox {
                        currentIndex: 2
                        model:applicationManager.x86MethodsNames
                        width: 200
                        onCurrentIndexChanged: console.debug(cbItems.get(currentIndex).text + ", " + cbItems.get(currentIndex).color)
                    }
                    //}
                    ListModel {
                        id: cbItems
                        ListElement { text: "Banana"; color: "Yellow" }
                        ListElement { text: "Apple"; color: "Green" }
                        ListElement { text: "Coconut"; color: "Brown" }
                    }
                }

            }
        }
    }
}
