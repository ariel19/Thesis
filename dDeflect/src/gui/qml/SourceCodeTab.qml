import QtQuick 2.3
import QtQuick.Controls 1.2
import QtQuick.Controls.Styles 1.2
import QtQuick.Layouts 1.1
import QtQuick.Dialogs 1.2
import QtQml.Models 2.1
import jsk.components 1.0


Component{
    id: sourceTab
    Tab {
        title: "Source"
        visible:true
        Rectangle{
        color: "gray"
        anchors.fill: parent



        property bool ss: true

        ColumnLayout{
            id:cl
            anchors.fill: parent
            //anchors.margins: 12
            anchors.topMargin: addButton.height/2
            spacing: 10
            RowLayout{
                Layout.alignment: Qt.AlignLeft
                width: 500
                height: 50
                spacing: 10
                Button{
                    id: addButton

                    text:"Click to add a method."
                    onClicked: {
                        comboboxesmobel.append({"name":"kuba"})
                    }
                }
                Button{
                    id: save

                    text:"Save"
                    onClicked: {
                        console.log("Save?")
                        applicationManager.getDeclarations();
                    }
                }
                Button{
                    id: apply

                    text:"Apply"
                    onClicked: {
                        console.log("ATTEMPT TO SECURE A SOURCE!")
                    }
                }
            }
            RowLayout{
                focus: true
                z: 1000
                Rectangle{
                    width: 500
                    height: 50
                    Loader{
                        anchors.fill: parent
                        // sourceComponent: applicationManager.state===1 ? winCombo : linCombo

                    }
                }


            }

            ListView {
                id: lv
                visible: true
                Layout.alignment: Qt.AlignLeft

                anchors.fill: parent
                anchors.topMargin: (addButton.height*3/2+lv.spacing)*2
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
                                id: functionCombo

                                model: applicationManager.x86MethodsNames
                                width: 200

                                onCurrentIndexChanged: {
                                    console.log(index);
                               }

                            }
                            ComboBox {
                                id: methodCombo
                                visible: true
                                model: applicationManager.currSourceMethods

                                width: 200
                                textRole: "name"
                                onCurrentIndexChanged: {
                                    console.log(index);
                              }

                            }

                        }

                    }

                }
            }
            ListModel {
                id: comboboxesmobel
            }
            ListModel {
                     id: functionsModel
                     ListElement { name: "main"; text: "main" }
                 }
        }
    }}
}
