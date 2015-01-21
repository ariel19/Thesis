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

    RowLayout{
        width: parent.width
        spacing: 2

    RadioButton {
        id: c1
        text: "EntryPoint"
        exclusiveGroup: tabPositionGroup
        onCheckedChanged: {
            console.log("OEP"+checked)
            console.log("System : "+applicationManager.sys  )
            if(checked){
                ss = false
                console.log(applicationManager.sys  )
                methodsModel = model
                handlersModel = model
                applicationManager.currCm = 0
                comboboxesmobel.clear()
                methodsModel = applicationManager.currMethods
                handlersModel = applicationManager.currHandlers
                applicationManager.saveClicked()
                ss = true
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
                ss = false
                methodsModel = model
                handlersModel = model
                applicationManager.currCm = 1
                comboboxesmobel.clear()
                methodsModel = applicationManager.currMethods
                handlersModel = applicationManager.currHandlers
                applicationManager.saveClicked()
                ss = true
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
                ss = false
                methodsModel = model
                handlersModel = model
                applicationManager.currCm = 2
                comboboxesmobel.clear()
                methodsModel = applicationManager.currMethods
                handlersModel = applicationManager.currHandlers
                applicationManager.saveClicked()
                ss = true
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
                ss = false
                methodsModel = model
                handlersModel = model
                applicationManager.currCm = 3
                comboboxesmobel.clear()
                methodsModel = applicationManager.currMethods
                handlersModel = applicationManager.currHandlers
                applicationManager.saveClicked()
                ss = true
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
                ss = false
                methodsModel = model
                handlersModel = model
                applicationManager.currCm = 4
                comboboxesmobel.clear()
                methodsModel = applicationManager.currMethods
                handlersModel = applicationManager.currHandlers
                applicationManager.saveClicked()
                ss = true
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
                ss = false
                methodsModel = model
                handlersModel = model
                applicationManager.currCm = 5
                comboboxesmobel.clear()
                methodsModel = applicationManager.currMethods
                handlersModel = applicationManager.currHandlers
                applicationManager.saveClicked()
                ss = true
            }
        }
    }
    }

}
