import QtQuick 2.3
import QtQuick.Controls 1.2
import QtQuick.Controls.Styles 1.2
import QtQuick.Layouts 1.1
import QtQuick.Dialogs 1.2
import jsk.components 1.0

Component{
    id:packingTab
    Tab {
        title: "Packing"
        visible:true
        RowLayout{
            anchors.fill: parent
            anchors.margins: 12

            TableView {
                anchors.fill: parent
                anchors.rightMargin: frame.width/2
                model: ["method1", "method2"]
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

                    anchors.fill: parent
                    anchors.bottomMargin: 40
                    text: "The only method:"
                }
                Button{
                    height:50
                    text: "Apply Method"
                    anchors.bottom: parent.bottom

                }
            }

        }
    }
}
