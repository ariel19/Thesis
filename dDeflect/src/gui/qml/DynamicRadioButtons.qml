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
    property bool checked: c1.checked||c2.checked||c3.checked||c4.checked||c5.checked||c6.checked

    function checkChanged(checked, rect){
        if(checked){
            ss = false
            buttonRow.visible = true
            console.log(applicationManager.sys)
            methodsModel = model
            handlersModel = model
            applicationManager.currCm = 0
            comboboxesmobel.clear()
            methodsModel = applicationManager.currMethods
            handlersModel = applicationManager.currHandlers
            applicationManager.saveClicked()
            ss = true
            rect.visible = true
        }
    }

    RowLayout{
        width: parent.width
        spacing: 2

        DRadioButton{
            text: "EntryPoint"
        }

        DRadioButton{
            text: "Thread"
        }

        DRadioButton{
            text: "Trampoline"
        }

        DRadioButton{
            text: "INIT"
        }
        DRadioButton{
            text: "INIT_ARRAY"
        }
        DRadioButton{
            text: "CTORS"
        }
    }

}
