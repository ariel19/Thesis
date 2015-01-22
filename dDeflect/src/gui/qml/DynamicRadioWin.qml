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

        DRadioButton{
            text:"EntryPoint"
            callingMethod: 0
        }
        DRadioButton{
            text:"Thread"
            callingMethod: 1
        }
        DRadioButton{
            text:"Trampoline"
            callingMethod: 2
        }
        DRadioButton{
            text:"TLS"
            callingMethod: 6
        }
    }
}
