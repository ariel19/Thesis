import QtQuick 2.3
import QtQuick.Controls 1.2
import QtQuick.Controls.Styles 1.2
import QtQuick.Layouts 1.1
import QtQuick.Dialogs 1.2
import jsk.components 1.0

Tab {
    title: "inv. method"
    RowLayout{
        anchors.fill: parent
        anchors.margins: 12

        TableView {
            id: methodsChoice
            anchors.fill: parent
            anchors.rightMargin: frame.width/2
            model: sourceCodeMethods.names


            TableViewColumn {
                role: "title"
                title: "Method's Name"
                //width: 120
            }


        }
        ColumnLayout{
            anchors.fill: parent
            anchors.leftMargin: frame.width/2
            TextArea{
                id: methodsDescription
                anchors.fill: parent
                anchors.bottomMargin: 40
                readOnly: true
                text:{ return methodsChoice===-1?"":sourceCodeMethods.methods[methodsChoice.currentRow].description }
            }
            Button{
                height:50
                text: "Apply Method"
                anchors.bottom: parent.bottom

            }
        }

    }
}

MethodList{
    id: sourceCodeMethods
    path: "/home/jsk/code/Thesis/build-dDeflect-Desktop_Qt_5_3_GCC_64bit-Debug/qtc_Desktop_Qt_5_3_GCC_64bit-debug/install-root/bin/methods.json"
}
