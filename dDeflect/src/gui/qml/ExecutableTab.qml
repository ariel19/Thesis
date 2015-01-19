import QtQuick 2.3
import QtQuick.Controls 1.2
import QtQuick.Controls.Styles 1.2
import QtQuick.Layouts 1.1
import QtQuick.Dialogs 1.2
import QtQml.Models 2.1
import jsk.components 1.0


Component{
    id: execTab
    Tab {
        title: "Executable"
        visible:true
        property var methodsModel: applicationManager.archType === applicationManager.X86 ? applicationManager.x86methods : applicationManager.x64methods
        property var handlersModel: applicationManager.archType === applicationManager.X86 ? applicationManager.x86handlers : applicationManager.x86handlers

        Component{
            id: displayDelegate

                Text {
                    text: name
                    anchors.centerIn: parent
                }
        }
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
                            id: cb2
                            currentIndex: 2
                            model: methodsModel
                            //model: applicationManager.x86methods
                            width: 200
                            textRole: "name"
                            onCurrentIndexChanged: {
                                console.log(cb2.model[currentIndex].returns);
                                console.log("chuj");
                            }
                        }
                        Loader {

                            Component{
                                id: c1
                                ComboBox {
                                    id: cb
                                    currentIndex: 2
                                    model: handlersModel
                                    //model: applicationManager.x86methods
                                    width: 200
                                    textRole: "name"
                                    onCurrentIndexChanged: {
                                        console.log(cb.model(currentIndex).returns);
                                        console.log("chuj");
                                    }
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
                            Component{
                                id: emptyId
                                Text{
                                    text:"No handler"
                                }
                            }

                            property var tab: [c1, c2]
                            sourceComponent: methodsModel[cb2.currentIndex].returns ? (methodsModel[cb2.currentIndex].isThread === true ? c2 : c1) :emptyId

                        }
                    }

                }

            }
        }
    }
}
