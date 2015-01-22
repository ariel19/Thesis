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
                    Layout.alignment: Qt.AlignCenter
                    width: 500
                    height: 50
                    spacing: 10
                    Button{
                        id: apply

                        text:"Apply"
                        onClicked: {
                            console.log("ATTEMPT TO SECURE A SOURCE!")
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
        }}
}
