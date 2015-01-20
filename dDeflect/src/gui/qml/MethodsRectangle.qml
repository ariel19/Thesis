import QtQuick 2.3
import QtQuick.Controls 1.2
import QtQuick.Controls.Styles 1.2
import QtQuick.Layouts 1.1
import QtQuick.Dialogs 1.2
import QtQml.Models 2.1
import jsk.components 1.0

ListView {
    property var methodsModel: applicationManager.currMethods
    property var handlersModel: applicationManager.archType === applicationManager.X86 ? applicationManager.x86handlers : applicationManager.x86handlers

    id: lv
    visible: true

    anchors.fill: parent
    anchors.topMargin: addButton.height*3/2+lv.spacing
    model: comboboxesmobel
    delegate: cbDelegate
    Component {
        id: cbDelegate

        Item{

            height: 50
            width: parent.width
            RowLayout{
                height: 50
                width: parent.width
                ComboBox {
                    id: methodCombo
                    currentIndex: 2
                    model: methodsModel
                    //model: applicationManager.x86methods
                    width: 200
                    textRole: "name"

                }
                Loader {

                    Component{
                        id: handlerCombo
                        ComboBox {
                            id: cb
                            model: handlersModel
                            //model: applicationManager.x86methods
                            width: 200
                            textRole: "name"

                        }
                    }
                    Component{
                        id: c2
                        RowLayout{
                            width: 400
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
                            Text{
                                text: "Add methods to thread."
                            }

                        }
                    }
                    Component{
                        id: emptyId
                        Text{
                            width: 200
                            text:"No handler possible."
                        }
                    }
                    sourceComponent: methodsModel[methodCombo.currentIndex].returns ? (methodsModel[methodCombo.currentIndex].isThread === true ? c2 : handlerCombo) :emptyId

                }
            }

        }

    }
}
