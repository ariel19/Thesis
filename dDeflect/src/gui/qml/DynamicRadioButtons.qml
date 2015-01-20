import QtQuick 2.3
import QtQuick.Controls 1.2
import QtQuick.Controls.Styles 1.2
import QtQuick.Layouts 1.1
import QtQuick.Dialogs 1.2
import QtQml.Models 2.1
import jsk.components 1.0
GroupBox {
    title: "Injection type"
    width: 500
    property var tab:[c1,c2,c3,c4,c5,c6]
    RadioButton {
        id: c1
        text: "OEP"
        checked: true
        exclusiveGroup: tabPositionGroup
        onCheckedChanged: {
            console.log("OEP"+checked)
            if(checked){
                applicationManager.currCm = 0
            }
        }
    }
    RadioButton {
        id: c2
        text: "Thread"
        exclusiveGroup: tabPositionGroup
        onCheckedChanged: {
            console.log("Thread"+checked)
            if(checked){
                applicationManager.currCm = 1
            }
        }
    }
    RadioButton {
        id: c3
        text: "Trampoline"
        exclusiveGroup: tabPositionGroup
        onCheckedChanged: {
            console.log("Trampoline"+checked)
            if(checked){
                applicationManager.currCm = 2

            }
        }
    }
    RadioButton {
        id: c4
        text: "INIT"
        exclusiveGroup: tabPositionGroup
        onCheckedChanged: {
            console.log("INIT"+checked)
            if(checked){
                applicationManager.currCm = 3

            }
        }
    }
    RadioButton {
        id: c5
        text: "INIT_ARRAY"
        exclusiveGroup: tabPositionGroup
        onCheckedChanged: {
            console.log("INIT_ARRAY"+checked)
            if(checked){
                applicationManager.currCm = 4

            }
        }
    }
    RadioButton {
        id: c6
        text: "CTORS"
        exclusiveGroup: tabPositionGroup
        onCheckedChanged: {
            console.log("CTORS"+checked)
            if(checked){
                applicationManager.currCm = 5

            }
        }
    }
    RowLayout {
        ExclusiveGroup { id: tabPositionGroup }
        Grid{
            spacing: 3
            Repeater{
                model: [0,1,2]
                Component {
                    id: delegate1
                    Loader{
                        width: 50
                        sourceComponent: tab[modelData]
                    }
                }
                delegate: delegate1
            }
        }
    }
}
