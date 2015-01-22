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
            color: "white"
            anchors.fill: parent



            property bool ss: true

            ColumnLayout{
                id:cl
                anchors.fill: parent
                spacing: 10
                RowLayout{
                    Layout.alignment: Qt.AlignCenter
                    width: 500
                    height: 50
                    spacing: 10
                    Button{
                        id: apply

                        text:"Apply"
                        onClicked: {
                            console.log("ATTEMPT TO SECURE A SOURCE!")
                            if(functionCombo.currentText!=="" && methodCombo.currentText!==""){
                                applicationManager.insertMethods(functionCombo.currentText, methodCombo.currentText)
                            }
                        }
                    }
                    GroupBox {
                        title: "Operating System"

                        RowLayout {
                            ExclusiveGroup { id: sysGroup }
                            RadioButton {
                                id:linButton
                                text: "Linux"
                                exclusiveGroup: sysGroup
                                onCheckedChanged: {
                                    if(checked){
                                        applicationManager.sys = 0
                                        linButton.enabled = false;
                                        winButton.enabled = false;
                                    }
                                }
                            }
                            RadioButton {
                                id: winButton
                                text: "Windows"
                                exclusiveGroup: sysGroup
                                onCheckedChanged: {
                                    if(checked)
                                        applicationManager.sys = 1
                                        linButton.enabled = false;
                                        winButton.enabled = false;
                                }
                            }
                        }
                    }
                }
                RowLayout{
                    height: 50
                    width: parent.width
                    Layout.alignment: Qt.AlignCenter
                    ComboBox {
                        id: functionCombo

                        model: applicationManager.x86MethodsNames
                        width: 200

                        onCurrentIndexChanged: {
                        }

                    }
                    ComboBox {
                        id: methodCombo
                        visible: true
                        model: applicationManager.currSourceMethods

                        width: 200
                        textRole: "name"
                        onCurrentIndexChanged: {
                        }

                    }

                }

            }
        }}
}
