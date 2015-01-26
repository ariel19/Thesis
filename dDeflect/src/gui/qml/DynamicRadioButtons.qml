import QtQuick 2.0
import QtQuick.Controls 1.1
import QtQuick.Controls.Styles 1.1
import QtQuick.Layouts 1.1
import QtQuick.Dialogs 1.1
import QtQml.Models 2.1
import jsk.components 1.0



GroupBox {
    title: "Injection type"
    width: 500
    property bool checked: c1.checked||c2.checked||c3.checked||c4.checked||c5.checked||c6.checked


    RowLayout{
        width: parent.width
        spacing: 2

        DRadioButton{
            text: "EntryPoint"
            callingMethod: 0
        }

        DRadioButton{
            text: "Thread"
            callingMethod: 1
        }

        DRadioButton{
            text: "Trampoline"
            callingMethod: 2
        }

        DRadioButton{
            text: "INIT"
            callingMethod: 3
        }
        DRadioButton{
            text: "INIT_ARRAY"
            callingMethod: 4
        }
        DRadioButton{
            text: "CTORS"
            callingMethod: 5
        }
    }

}
