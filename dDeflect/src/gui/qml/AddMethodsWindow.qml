import QtQuick.Window 2.0
import QtQuick 2.0
import QtQuick.Controls 1.1
import QtQuick.Controls.Styles 1.1
import QtQuick.Layouts 1.1
import QtQuick.Dialogs 1.1
import jsk.components 1.0

Window{
    width: 300
    height: 300

    visible:true
    ColumnLayout{
        id:cl
        anchors.fill: parent
        //anchors.margins: 12
        anchors.topMargin: addButton.height/2
        spacing: 10

        Button{
            id: addButton
            anchors.top: parent.top
            anchors.left: parent.left

            Layout.alignment: Qt.AlignLeft
            text:"+"
            onClicked: {
                fruitModel.append({"name":"kuba"})
                console.log(fruitModel)
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
                height: 50
                width: parent.width
                ListModel {
                    id: cbItems
                    ListElement { text: "Banana"; color: "Yellow" }
                    ListElement { text: "Apple"; color: "Green" }
                    ListElement { text: "Coconut"; color: "Brown" }
                }
                RowLayout{
                    height: 50
                    width: parent.width
                    ComboBox {
                        currentIndex: 2
                        model:applicationManager.x86MethodsNames
                        width: 200
                        onCurrentIndexChanged: console.debug(cbItems.get(currentIndex).text + ", " + cbItems.get(currentIndex).color)
                    }
                    Loader {

                        Component{
                            id: c1
                            ComboBox{
                                width: 200
                            }
                        }
                        Component{
                            id: c2

                            Button{
                                id: threadButton
                                width: 200
                                text:"+"

                                onClicked: {
                                    var component = Qt.createComponent("AddMethodsWindow.qml");
                                    var win = component.createObject(root);
                                    win.show();
                                }

                            }
                        }
                        property var tab: [c1, c2]
                        sourceComponent: tab[1]

                    }
                }

            }

        }
    }
}
