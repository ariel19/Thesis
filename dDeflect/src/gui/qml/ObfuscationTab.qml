import QtQuick 2.0
import QtQuick.Controls 1.1
import QtQuick.Controls.Styles 1.1
import QtQuick.Layouts 1.1
import QtQuick.Dialogs 1.1
import jsk.components 1.0

Component{
    id: obfuscationTab
    Tab {
        id: tab
        title: "Obfuscation"
        visible: true
        ColumnLayout{
            anchors.fill: parent

            Text{
                text:"Instruction Coverage"
                Layout.alignment: Qt.AlignCenter
            }

            Slider {
                id: coverage
                value: 0.01
                Layout.fillWidth: true
                tickmarksEnabled: true
                stepSize: 0.01

            }
            Text{
                text:"Minimum length of garbage data"
                Layout.alignment: Qt.AlignCenter
            }
            Slider {
                id: slider2
                value: 0.01
                Layout.fillWidth: true
                tickmarksEnabled: true
                stepSize: 0.01
                onValueChanged: if(value>=slider3.value)
                                    value=slider3.value-0.01

            }
            Text{
                text:"Maximum length of garbage data"
                Layout.alignment: Qt.AlignCenter
            }
            Slider {
                id: slider3
                value: 0.5
                Layout.fillWidth: true
                tickmarksEnabled: true
                stepSize: 0.01
                onValueChanged: if(value<=slider2.value)
                                    value=slider2.value+0.01

            }

            Button{
                Layout.alignment: Qt.AlignCenter
                width: 200
                height: 50
                text: "OBFUSCATE!"
                onClicked: {
                   applicationManager.obfuscateClicked(coverage.value*100, slider2.value*100, slider3.value*100);
                   tab.enabled = false
                }
            }
        }
    }
}
