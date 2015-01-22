import QtQuick 2.3
import QtQuick.Controls 1.2
import QtQuick.Controls.Styles 1.2
import QtQuick.Layouts 1.1
import QtQuick.Dialogs 1.2
import QtQml.Models 2.1
import jsk.components 1.0

RadioButton {
    id: c6

    function checkChanged(checked, rect){
        if(checked){
            ss = false

            addButton.enabled = true;
            saveButton.enabled = true;
            applyButton.enabled = true;

            //buttonRow.visible = true
            console.log(applicationManager.sys)
            methodsModel = model
            handlersModel = model
            applicationManager.currCm = 0
            comboboxesmobel.clear()
            methodsModel = applicationManager.currMethods
            handlersModel = applicationManager.currHandlers
            applicationManager.saveClicked()
            ss = true
            c6.enabled = false
        }
    }

    Rectangle{
        id: rect6
        opacity: 0.5
        anchors.fill: parent
        color: "black"
        z: parent.z+1
        visible:false
        MouseArea {
            anchors.fill: parent
            onClicked: { parent.color = 'black' }
        }
    }
    exclusiveGroup: tabPositionGroup
    onCheckedChanged: {
        console.log("CTORS"+checked)
        checkChanged(checked,rect6)
    }
}
