import QtQuick 2.3
import QtQuick.Controls 1.2
import QtQuick.Controls.Styles 1.2
import QtQuick.Layouts 1.1
import QtQuick.Dialogs 1.2
import jsk.components 1.0

Component {
    id:sourceTab

    Tab{
        title: "Source"
        visible:true
        RowLayout{
            anchors.fill: parent
            anchors.margins: 12

            Component {
                id: contactDelegate
                Item {
                    width: 180; height: 20
                    Row {
                        anchors.centerIn: parent.Center
                        CheckBox{
                            onCheckedChanged: {
                                if(checked===true){
                                    if(t.indexOf(index)===-1){
                                        t.push(index);
                                        console.log(t);
                                    }
                                }else{
                                    if(t.indexOf(index)!==-1){
                                        var i = t.indexOf(index);
                                        t.splice(i,1);
                                        console.log(t);
                                    }
                                }
                            }
                        }
                        Text{text: '<b>Name:</b> ' + modelData }
                    }
                }
            }
            ListView {
                id: lv
                anchors.fill: parent
                anchors.rightMargin: frame.width/2
                model: applicationManager.x86MethodsNames
                delegate: contactDelegate

            }
            ColumnLayout{
                anchors.fill: parent
                anchors.leftMargin: frame.width/2
                TextArea{

                    anchors.fill: parent
                    anchors.bottomMargin: 40
                    text: "The only method:"
                }
                Button{
                    height:50
                    text: "Apply Method"
                    anchors.bottom: parent.bottom
                    onClicked: {
//                        applicationManager.applyClicked(t)
//                        applicationManager.state = Manager.IDLE
//                        appState = Manager.IDLE
//                        fileUrlText.text = "Choose a C++ source file or an executive file."
                        applicationManager.getDeclarations();
                    }
                }
            }
        }
    }
}
