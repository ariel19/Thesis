import QtQuick 2.0
import QtQuick.Controls 1.1
import QtQuick.Controls.Styles 1.1
import QtQuick.Layouts 1.1
import QtQuick.Dialogs 1.1
import QtQml.Models 2.1
import jsk.components 1.0

Rectangle {
    width: parent.width
    height: 50
    RowLayout{
        width:parent.width
        height: parent.height
        Rectangle{
            width: parent.width/3
            height: 50
            color: "red"
            Text{
                text: "Entry Point >>"
                color: "black"
            }
        }
        Rectangle{
            width: parent.width/3
            height: 50
            color: "green"
            Text{
                text: "TLS >>"
                color: "black"
            }
        }
        Rectangle{
            width: parent.width/3
            height: 50
            color: "yellow"
            Text{
                text: "Trampoline >>"
                color: "black"
            }
        }
    }
}
